/* ================================================================================
 * Copyright: (C) 2022, SIRRL Social and Intelligent Robotics Research Laboratory, 
 *     University of Waterloo, All rights reserved.
 * 
 * Authors: 
 *     Austin Kothig <austin.kothig@uwaterloo.ca>
 * 
 * CopyPolicy: Released under the terms of the MIT License. 
 *     See the accompanying LICENSE file for details.
 * ================================================================================
 */

#include <embodiedSocialInterface.hpp>


bool EmbodiedSocialInterface::configure(yarp::os::ResourceFinder &rf) {

    //-- Get some variables from the configuration file that the resource finder loaded.
    module_name = rf.check("name", yarp::os::Value("/embodiedSocialInterface"), "module name (string)").asString();
    this->setName(module_name.c_str());

    max_tower_height = 10;
    window_height    = 13; // set via rf.
    window_width     = 36;
    

    //-- Attach a port of the same name as the module so that messages 
    //-- received from the port are redirected to the respond method.
    std::string handle_name = this->getName();
    if (!handler.open(handle_name.c_str())) {
        yInfo("%s: Unable to open port %s", this->getName().c_str(), handle_name.c_str());
        return false;
    }
    this->attach(handler);


    //-- Initialize the rpc client for sending/receiving messages with the game server.
    std::string rpc_name = this->getName() + "/rpc";
    if (!rpc.open(rpc_name.c_str())) {
        yInfo("%s: Unable to open port %s", this->getName().c_str(), rpc_name.c_str());
        return false;
    }


    // TODO: Set everything else.
    /* // Don't need these right now...
    bool ok = true;
    ok &= audio_port.open( this->getName() + "/audio:o" );
    ok &= video_port.open( this->getName() + "/video:o" );
    ok &= web_port.open(   this->getName() + "/web:o"   );
    if (!ok) {
        yInfo("%s: Something went wrong opening the auxiliary ports!", this->getName.c_str());
        return false;
    }
    */   

    selected_from = -1;
    selected_to   = -1;

    move_count    =  0;
    waiting_count =  0;
    game_complete = false;
    

    //-- Init the ncurses window.
    setlocale(LC_ALL, "");
    
    initscr();
    noecho();
    
    keypad(stdscr, TRUE);

    refresh();

    return true;
}


bool EmbodiedSocialInterface::interruptModule() {
    handler.interrupt();
    rpc.interrupt();
    return true;
}


bool EmbodiedSocialInterface::close() {

    //-- Close the yarp ports.
    handler.close();
    rpc.close();

    //-- End the ncurses window.
    endwin();

    //-- Give a little end of game message.
    yInfo() << "You made it to the end in" << move_count << "moves!!";

    return true;
}


bool EmbodiedSocialInterface::respond(const yarp::os::Bottle &cmd, yarp::os::Bottle &reply) {
    
    std::string helpMessage = std::string(getName().c_str()) + " commands are: \n" + "help \n" + "quit \n";
    reply.clear();

    if (cmd.get(0).asString() == "quit") {
        reply.addString("quitting");
        return false;
    } else if (cmd.get(0).asString() == "help") {
        yInfo() << helpMessage;
        reply.addString(helpMessage);
    }

    return true;
}


double EmbodiedSocialInterface::getPeriod() {
    // control rate set here in seconds.
    return 0.1;
}


bool EmbodiedSocialInterface::updateModule() {

    //-- Check if we have a connection to the game server... 
    if (rpc.getOutputCount() == 0) {
        drawWaiting();
        yarp::os::Time::delay(1.0);
        return true;
    }
    
    //-- Init some bottles for communication.
    yarp::os::Bottle cmd, rsp;

    //-- Query the game server for the current board state.
    std::string board_status = communicate("show", cmd, rsp);

    //-- Parse it up into neat rows.
    parseShowable(board_status);

    //-- Draw the interface.
    drawInterface();

    //-- Query the game server for the current boards next best move.
    std::string game_hint = communicate("hint", cmd, rsp);


    //
    // Do something with the hint and state_machine here.
    //


    //-- Wait for key input.
    int key_press = getch();
    bool execute_move = false;

    //-- See if one of `our` keys were pressed.
    switch (key_press) {

        case '1':
            keyPressed(1);
            break;

        case '2':
            keyPressed(2);
            break;

        case '3':
            keyPressed(3);
            break;

        case 10: // [ENTER]
            if (selected_from != -1 && selected_to != -1) {
                execute_move = true;
            }
            break;
        
        // Nope.
        default:
            break;
    }

    
    //-- Time to try making a move!
    if (execute_move) {

        //-- Format the move.
        std::string move = "move " //+ game_hint;    // uncomment this for auto-solve.
            + std::to_string(selected_from-1) + " " 
            + std::to_string(selected_to-1);

        //-- Write the move to the game server.
        std::string move_status = communicate(move, cmd, rsp);

        //-- If the move was a good move, reset the selected
        if (move_status != "0") { // "1" and "2" are accepted moves.
            selected_from = -1;
            selected_to   = -1;
            move_count++;
        }

        //-- Game complete, close module.
        if (move_status == "2") { 

            //-- Mark the game as completed.
            game_complete = true;

            //-- Get the final board state.
            board_status = communicate("show", cmd, rsp);

            //-- Parse the final board status and show it.
            parseShowable(board_status);
            drawInterface();
            
            //-- Wait for a few seconds before beginning to wrap up.
            yarp::os::Time::delay(5.0);

            //-- Tell the game to close.
            communicate("exit", cmd, rsp);

            //
            // Should write to the auxiliary ports here.
            //
            
            this->interruptModule(); 
            this->close(); 
            return false; 
        }
    }


    return true;
}


std::string EmbodiedSocialInterface::communicate(const std::string msg, yarp::os::Bottle& command, yarp::os::Bottle& response) {

    //-- Clear out the bottles
    command.clear(); response.clear();

    //-- Add the intended message.
    command.addString(msg);

    //-- Write the command and wait for a response.
    rpc.write(command, response);

    //-- Return the first item in the bottle
    //-- Note:
    //--   If there is other data needed in the response bottle
    //--   it can be accessed outside this function since we're
    //--   passed by reference.
    return response.get(0).asString();
}


void EmbodiedSocialInterface::keyPressed(int key_num) {

    //-- Deselect the from.
    if (selected_from == key_num) {
        selected_from = -1;
        selected_to   = -1;
        return;
    }

    //-- Deselect the to.
    if (selected_to == key_num) {
        selected_to = -1;
        return;
    }

    //-- Not a deselect... Try selecting!

    if (selected_from == -1) {
        selected_from = key_num;
    } else {
        selected_to = key_num; // overwrite.
    }

    return;
}


void EmbodiedSocialInterface::parseShowable(std::string str) {

    std::stringstream ss(str);
    showable_rows.clear();

    std::string seg;
    while (getline(ss, seg, '\n')) {
        showable_rows.push_back(seg + "\n");
    }
    
    return;
}


void EmbodiedSocialInterface::drawInterface() {

    //-- Reset and move the screen position.
    clear(); move(0,0);

    //-- Init a buffer string to write to.
    std::string line_buffer;

    //-- ROW SECTION 1: move count.
    std::string move_number = std::to_string(move_count);

    int leadingzeros = 3;
    move_number = std::string(leadingzeros-move_number.length(), '0') + move_number;
    //move_number.insert(0, 3-move_number.length(), '0'); //two options to try. // replace 3 with "leadingzeros"
    line_buffer = "move #" + move_number;

    if (game_complete) {
        std::string win_buffer = "[YOU WIN!!]";
        line_buffer = win_buffer + std::string(3, ' ') + line_buffer;
        //std::string( (window_width - win_buffer.length() + 1)/2, ' ')  + win_buffer + "\n";
    }

    //-- Right Justified
    line_buffer = std::string(window_width - line_buffer.length(), ' ')  + line_buffer + "\n\n";
    addstr(line_buffer.c_str());

    //-- ROW SECTION 2: peg tops until disks.
    for (int count = 0; count < max_tower_height - (showable_rows.size()-2); ++count) {
        addstr(showable_rows[1].c_str());
    }

    //-- ROW SECTION 3: disks until base.
    for (int idx = 2; idx < showable_rows.size()-1; ++idx) {
        addstr(showable_rows[idx].c_str());
    }

    //-- ROW SECTION 4: disk selections.
    line_buffer = std::string(6, ' ') 
        + "[1]" + std::string(8, ' ') 
        + "[2]" + std::string(8, ' ') 
        + "[3]" + "\n";
    addstr(line_buffer.c_str());

    // This is embarrassing and sucks but formatting...
    std::vector<int> MAGIC_FORMAT {0, 5, 16, 27}; 

    //-- Clear the buffer.
    line_buffer = std::string(MAGIC_FORMAT[0], ' ');
    if (selected_from != -1 && selected_to == -1) {
        line_buffer += std::string(MAGIC_FORMAT[selected_from], ' ') + "^^^^^" + "\n" 
                     + std::string(MAGIC_FORMAT[selected_from], ' ') + "FROM " + "\n\n";
    } else if (selected_from != -1 && selected_to != -1) {

        // Do the selected row first.
        if (selected_from == 1 || selected_to == 1) {
            line_buffer += std::string(5, ' ') + "^^^^^";
        } else {
            line_buffer += std::string(10, ' ');
        }

        if (selected_from == 2 || selected_to == 2) {
            line_buffer += std::string(6, ' ') + "^^^^^";
        } else {
            line_buffer += std::string(11, ' ');
        }

        if (selected_from == 3 || selected_to == 3) {
            line_buffer += std::string(6, ' ') + "^^^^^";
        } else {
            line_buffer += std::string(11, ' ');
        }
        line_buffer += "\n";

        //-- Fill peg 1 if selected as FROM or TO
        if (selected_from == 1) {
            line_buffer += std::string(5, ' ') + "FROM ";
        } else if (selected_to == 1) {
            line_buffer += std::string(5, ' ') + " TO  ";
        } else {
            line_buffer += std::string(10, ' ');
        }

        //-- Fill peg 2 if selected as FROM or TO
        if (selected_from == 2) {
            line_buffer += std::string(6, ' ') + "FROM ";
        } else if (selected_to == 2) {
            line_buffer += std::string(6, ' ') + " TO  ";
        } else {
            line_buffer += std::string(11, ' ');
        }

        //-- Fill peg 3 if selected as FROM or TO
        if (selected_from == 3) {
            line_buffer += std::string(6, ' ') + "FROM ";
        } else if (selected_to == 3) {
            line_buffer += std::string(6, ' ') + " TO  ";
        } else {
            line_buffer += std::string(11, ' ');
        }
        line_buffer += "\n";

        std::string enter_buffer = "[ENTER]";
        line_buffer += std::string( (window_width - enter_buffer.length() + 1)/2, ' ')  + enter_buffer + "\n";

    } else {
        // Nothing selected...
        line_buffer = "\n\n\n"; 
    }


    addstr(line_buffer.c_str());

    refresh();

    return;
}


void EmbodiedSocialInterface::drawWaiting() {

    //-- Reset and move the screen position.
    clear(); move(0,0);

    //-- Little waiting animation.
    std::vector<std::string> waiting = { " \\", " ─ ", " / ", " | " }; 
    int waiting_len = waiting.size();
    std::string line_flush(30, ' ');
    line_flush += "\n";

    //-- Put the waiting dialogue in the middle of the screen.
    for (int ldx = 0; ldx < window_height/2; ++ldx) {
        addstr( line_flush.c_str() );
    }
    
    //-- Write an output message on the status.
    std::string write_string = std::string(8, ' ') + "Waiting for connection to game server..." + waiting[waiting_count % waiting_len] + "\n";
    addstr(write_string.c_str());
    waiting_count++;

    //-- Flush the remaining lines.
    for (int ldx = 0; ldx < window_height/2; ++ldx) {
        addstr( line_flush.c_str() );
    }

    refresh();

    return;
}
