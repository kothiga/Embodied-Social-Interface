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

    module_name = rf.check("name", yarp::os::Value("/embodiedSocialInterface"), "module name (string)").asString();
    this->setName(module_name.c_str());

    // Attach a port of the same name as the module so that messages 
    // received from the port are redirected to the respond method.
    std::string handle_name = this->getName();
    if (!handler.open(handle_name.c_str())) {
        yInfo("%s: Unable to open port %s", this->getName().c_str(), handle_name.c_str());
        return false;
    }

    this->attach(handler);

    
    std::string rpc_name = this->getName() + "/rpc";
    if (!rpc.open(rpc_name.c_str())) {
        yInfo("%s: Unable to open port %s", this->getName().c_str(), rpc_name.c_str());
        return false;
    }
    
    // TODO: Set everything else.

    initscr();
    //max_height = 0;

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

    //-- End the window.
    endwin();

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

    int key_press = getch();
    std::cout << key_press << std::endl;

    

    yInfo() << "Module running happily!";

    
    //-- Init some bottles for communication.
    yarp::os::Bottle cmd, response;


    cmd.clear(); response.clear();
    cmd.addString("show");
    rpc.write(cmd,response);
    std::string board_status = response.get(0).asString();

    parseShowable(board_status);


    

/*    
    cmd.clear(); response.clear();
    cmd.addString("hint");
    rpc.write(cmd,response);
    std::cout << response.get(0).asString() << std::endl;

    std::string move = "move " + response.get(0).asString();

    cmd.clear(); response.clear();
    cmd.addString(move);
    rpc.write(cmd,response);


    std::string move_status = response.get(0).asString();
    std::cout << "move status: " << move_status << std::endl;

    // Game complete, close module.
    if (move_status == "2") { 
        this->interruptModule(); 
        this->close(); 
        return false; 
    }
*/

    return true;
}


void EmbodiedSocialInterface::parseShowable(std::string str) {

    std::stringstream ss(str);
    showable_rows.clear();

    std::string seg;
    while (getline(ss, seg, '\n')) {
        showable_rows.push_back(seg);
    }

    
    // TODO: REMOVE.
    //std::cout << "Rows: " << showable_rows.size() << " " << max_height << std::endl;
    for (int idx = 0; idx < showable_rows.size(); ++idx) {
        std::cout << idx << ": " << showable_rows[idx] << std::endl;
    }
    std::cout << "--------------done--------------" << std::endl;

    //if (showable_rows.size() > max_height) { max_height = showable_rows.size(); }
    
    return;
}