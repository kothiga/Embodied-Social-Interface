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
    _module_name = rf.check("name", yarp::os::Value("/embodiedSocialInterface"), "module name (string)").asString();
    this->setName(_module_name.c_str());

    //-- Attach a port of the same name as the module so that messages 
    //-- received from the port are redirected to the respond method.
    std::string handle_name = this->getName();
    if (!_handler.open(handle_name.c_str())) {
        yInfo("%s: Unable to open port %s", this->getName().c_str(), handle_name.c_str());
        return false;
    }
    this->attach(_handler);


    //-- Initialize the rpc client for sending/receiving messages with the game server.
    std::string rpc_name = this->getName() + "/rpc";
    if (!_rpc.open(rpc_name.c_str())) {
        yInfo("%s: Unable to open port %s", this->getName().c_str(), rpc_name.c_str());
        return false;
    }


    //-- Initialize the auxiliary ports.
    bool ok = true;
    ok &= _media_port.open( this->getName() + "/media:o" );
    ok &= _web_port.open(   this->getName() + "/web:o"   );
    if (!ok) {
        yInfo("%s: Something went wrong opening the auxiliary ports!", this->getName().c_str());
        return false;
    }


    //-- Initialize the file stream.
    _user_name = rf.check("user",  yarp::os::Value("user01"), "user name (string)").asString();
    _file_path = rf.check("fpath", yarp::os::Value("./"),     "file path (string)").asString();

    std::string file_name = _file_path + "/" + _user_name + ".csv";
    if (!_logger.openLogger(file_name)) {
        yInfo("%s: Unable to file stream for logging %s", this->getName().c_str(), file_name.c_str());
        return false;
    }


    //-- Init the state machine states.
    _media_path = rf.check("mpath", yarp::os::Value("./data"), "file path (string)").asString();

    yarp::os::Bottle* bot = rf.find("states").asList();
    if (bot) {
        for (int idx = 0; idx < bot->size(); ++idx) {
            _machine.addState(bot->get(idx).toString());
        }
    } else {
        _machine.addState("none");
    }

    //-- Set the min/max number of steps.
    _min_steps_per = rf.check("minSteps", yarp::os::Value(5), "min steps (int)").asInt32();
    _max_steps_per = rf.check("maxSteps", yarp::os::Value(5), "max steps (int)").asInt32();

    _machine.setMinSteps(_min_steps_per);
    _machine.setMaxSteps(_max_steps_per);


    //-- Set the time to wait between allowing moves to pass
    _time_between = rf.check("seconds", yarp::os::Value(5.0), " (double)").asFloat64();


    //-- Set some interface appearance vars.
    _max_tower_height = rf.check("maxTower", yarp::os::Value(10), " (int)").asInt32();
    _window_height    = rf.check("height",   yarp::os::Value(13), " (int)").asInt32();
    _window_width     = rf.check("width",    yarp::os::Value(36), " (int)").asInt32();
    _right_shift      = rf.check("rshift",   yarp::os::Value(0),  " (int)").asInt32();


    //-- Set the URL to the end of game survey.
    _end_survey = rf.check("survey", yarp::os::Value("https://kothiga.github.io/"), "survey url (string)").asString();


    //-- Init the from and to as unselected.
    selected_from = -1;
    selected_to   = -1;


    //-- Init some interface logic vars.
    _move_count        =  0;
    _waiting_count     =  0;
    _game_complete     = false;
    _current_hint_sent = false;
    _last_execution    = yarp::os::Time::now();
    

    //-- Init the ncurses window.
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    keypad(stdscr, TRUE);
    refresh();

    return true;
}


bool EmbodiedSocialInterface::interruptModule() {
    
    //-- Interrupt the ports.
    _handler.interrupt();
    _rpc.interrupt();

    _media_port.interrupt();
    _web_port.interrupt();

    return true;
}


bool EmbodiedSocialInterface::close() {

    //-- Close the yarp ports.
    _handler.close();
    _rpc.close();

    _media_port.close();
    _web_port.close();

    //-- Close the file stream.
    _logger.closeLogger();

    //-- End the ncurses window.
    endwin();

    //-- Give a little end of game message.
    yInfo() << "You made it to the end in" << _move_count << "moves!!";

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
    if (_rpc.getOutputCount() == 0) {

        drawWaiting();
        yarp::os::Time::delay(1.0);
        
        sendMessage(_media_port, "none");
        _current_hint_sent = false;

        _last_execution = yarp::os::Time::now();
        _start_time     = yarp::os::Time::now();

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
    _machine.setCurrentHint(game_hint);


    //-- Allow some time for the out to finish executing before sending next in.
    if (!_current_hint_sent && _media_port.getOutputCount() != 0) {
        
        std::string media_msg = _media_path + "/" + _machine.getStateHint("in") + ".mp4";
        
        sendMessage(_media_port, media_msg);
        
        _current_hint_sent = true;
    }


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

        //-- If this move was too fast, don't let it go through.
        if ((yarp::os::Time::now() - _last_execution) < _time_between) {
            //std::cout << yarp::os::Time::now() - _last_execution << " no" << std::endl;
            return true;
        }


        //-- Format the move.
        std::string game_move = std::to_string(selected_from-1) + " " + std::to_string(selected_to-1);
        std::string move = "move " + game_move;
            //+ game_hint;    // uncomment this for auto-solve.


        //-- Get the current hash and dist incase this move 
        //-- is successful (pass to csv logger).
        std::string game_hash = communicate("hash", cmd, rsp);
        std::string game_dist = communicate("dist", cmd, rsp);


        //-- Write the move to the game server.
        std::string move_status = communicate(move, cmd, rsp);

        //-- If the move was not good, go to next update step.
        if (move_status == "0") { // "1" and "2" are accepted moves.
            return true;
        }

        
        // Log the data for this move.
        _logger.log(
            /*int_time   =*/ yarp::os::Time::now() - _start_time,
            /*user_id    =*/ _user_name,
            /*channel    =*/ _machine.getCurrentState(),
            /*hint_id    =*/ game_hint,
            /*hash       =*/ game_hash,
            /*distance   =*/ game_dist,
            /*move_number=*/ _move_count,
            /*from       =*/ selected_from-1,
            /*to         =*/ selected_to-1
        );


        //-- Reset the selected moves and increment counter.
        selected_from = -1;
        selected_to   = -1;
        _move_count++;


        //-- Retract the hint back to the home state.
        std::string media_msg = _media_path + "/" + _machine.getStateHint("out") + ".mp4";
        
        //-- Do something slightly different for expressions.
        if (_machine.getCurrentState() == "icub-expression") {
            media_msg = _media_path + "/" + _machine.getCurrentState() + "_";
            media_msg += (game_move == game_hint ? "correct" : "wrong");
            media_msg += "_out.mp4";
        }
        
        sendMessage(_media_port, media_msg);
        
        _current_hint_sent = false;


        //-- Set the previous execution time.
        _last_execution = yarp::os::Time::now();
        
        //-- Step the state machine.
        _machine.step();

        //-- Allow a bit of time for the media port to read in
        //-- the previous message before looping back around.
        yarp::os::Time::delay(0.2);

        
        //-- Game complete, close module.
        if (move_status == "2") { 

            //-- Mark the game as completed.
            _game_complete = true;

            //-- Get the final board state.
            board_status = communicate("show", cmd, rsp);

            //-- Parse the final board status and show it.
            parseShowable(board_status);
            drawInterface();

            //-- Get the final board state information.
            game_hash = communicate("hash", cmd, rsp);
            game_dist = communicate("dist", cmd, rsp);

            //-- Finally log it.
            _logger.log(
                /*int_time   =*/ yarp::os::Time::now() - _start_time,
                /*user_id    =*/ _user_name,
                /*channel    =*/ "",
                /*hint_id    =*/ "",
                /*hash       =*/ game_hash,
                /*distance   =*/ game_dist,
                /*move_number=*/ _move_count,
                /*from       =*/ selected_from, // -1
                /*to         =*/ selected_to    // -1
            );

            //-- Send a celebration video for completing the game.
            sendMessage(_media_port, _media_path + "/celebrate.mp4");

            //-- Wait for a few seconds before beginning to wrap up.
            yarp::os::Time::delay(8.0);
            sendMessage(_web_port, _end_survey);
            yarp::os::Time::delay(5.0);


            //-- Tell the game to close.
            communicate("exit", cmd, rsp);

            //-- Write to the auxiliary ports to cleanup.
            sendMessage(_media_port, "exit");
            sendMessage(_web_port,   "exit");
            
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
    _rpc.write(command, response);

    //-- Return the first item in the bottle
    //-- Note:
    //--   If there is other data needed in the response bottle
    //--   it can be accessed outside this function since we're
    //--   passed by reference.
    return response.get(0).asString();
}


void EmbodiedSocialInterface::sendMessage(yarp::os::Port& port, const std::string msg) {
    
    //-- Put my message in a modem.
    yarp::os::Bottle bot;
    bot.addString(msg);

    //-- And throw it in the Cyber Sea.
    port.write(bot);

    return;
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

    //-- Right shift buffer.
    std::string shift(_right_shift, ' ');

    //-- Init a buffer string to write to.
    std::string line_buffer;

    //-- ROW SECTION 1: move count.
    std::string move_number = std::to_string(_move_count);

    int leadingzeros = 3;
    move_number = std::string(leadingzeros-move_number.length(), '0') + move_number;
    line_buffer = "move #" + move_number;

    if (_game_complete) {
        std::string win_buffer = "[YOU WIN!!]";
        line_buffer = win_buffer + std::string(3, ' ') + line_buffer;
        //std::string( (window_width - win_buffer.length() + 1)/2, ' ')  + win_buffer + "\n";
    }

    //-- Right Justified
    line_buffer = std::string(_window_width - line_buffer.length(), ' ')  + line_buffer + "\n\n";
    addstr((shift+line_buffer).c_str());

    //-- ROW SECTION 2: peg tops until disks.
    for (int count = 0; count < _max_tower_height - (showable_rows.size()-2); ++count) {
        addstr((shift+showable_rows[1]).c_str());
    }

    //-- ROW SECTION 3: disks until base.
    for (int idx = 2; idx < showable_rows.size()-1; ++idx) {
        addstr((shift+showable_rows[idx]).c_str());
    }

    //-- ROW SECTION 4: disk selections.
    line_buffer = std::string(6, ' ') 
        + "[1]" + std::string(8, ' ') 
        + "[2]" + std::string(8, ' ') 
        + "[3]" + "\n";
    addstr((shift+line_buffer).c_str());

    // This is embarrassing and sucks but formatting...
    std::vector<int> MAGIC_FORMAT {0, 5, 16, 27}; 

    //-- Clear the buffer.
    line_buffer = shift+std::string(MAGIC_FORMAT[0], ' ');
    if (selected_from != -1 && selected_to == -1) {
        line_buffer += std::string(MAGIC_FORMAT[selected_from], ' ') + "^^^^^" + "\n" + shift
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
        line_buffer += "\n" + shift;

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
        line_buffer += "\n" + shift;

        std::string enter_buffer = "[ENTER]";
        line_buffer += std::string( (_window_width - enter_buffer.length() + 1)/2, ' ')  + enter_buffer + "\n";

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
    for (int ldx = 0; ldx < _window_height/2; ++ldx) {
        addstr( line_flush.c_str() );
    }
    
    //-- Write an output message on the status.
    std::string write_string = std::string(4, ' ') + "Waiting for connection to game server..." + waiting[_waiting_count % waiting_len] + "\n";
    addstr(write_string.c_str());
    _waiting_count++;

    //-- Flush the remaining lines.
    for (int ldx = 0; ldx < _window_height/2; ++ldx) {
        addstr( line_flush.c_str() );
    }

    refresh();

    return;
}
