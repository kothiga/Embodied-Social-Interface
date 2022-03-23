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

#include <clipMaker.hpp>


bool ClipMaker::configure(yarp::os::ResourceFinder &rf) {

    //-- Get some variables from the configuration file that the resource finder loaded.
    _module_name = rf.check("name", yarp::os::Value("/clipMaker"), "module name (string)").asString();
    this->setName(_module_name.c_str());

    //-- Initialize and attach the RPC server to handle receiving and sending commands.
    std::string rpc_name = this->getName() + "/rpc";
    if (!_rpc.open(rpc_name.c_str())) {
        yInfo("%s: Unable to open port %s", this->getName().c_str(), rpc_name.c_str());
        return false;
    }
    this->attach(_rpc);

    bool ok = true;
    ok &= _body_port.open(this->getName() + "/body:o");
    ok &= _expr_port.open(this->getName() + "/expr:o");
    ok &= _gaze_port.open(this->getName() + "/gaze:o");
    if (!ok) {
        yInfo("%s: Unable to open control ports!!", this->getName().c_str());
        return false;
    }


    //-- Init the body vars.


    //-- Init the expression vars.
    _expr_timer = rf.check("expr_timer", yarp::os::Value(0.2), "expressions (double)").asFloat64();


    //-- Init the gaze vars.


    //-- Init the speech vars.
    _spch_timer = rf.check("spch_timer", yarp::os::Value(3.0), "speech duration (double)").asFloat64();

    return true;
}


bool ClipMaker::interruptModule() {
    
    //-- Interrupt the ports.
    _rpc.interrupt();
    _body_port.interrupt();
    _expr_port.interrupt();
    _gaze_port.interrupt();

    return true;
}


bool ClipMaker::close() {

    //-- Close the yarp ports.
    _rpc.close();
    _body_port.close();
    _expr_port.close();
    _gaze_port.close();

    return true;
}


bool ClipMaker::respond(const yarp::os::Bottle &cmd, yarp::os::Bottle &reply) {
    
    std::string helpMessage = std::string(getName().c_str()) + " commands are: home | beh {blob|body|spch|gaze|expr} <int> <int> | help | quit";
    reply.clear();

    std::string command = cmd.get(0).asString();
    if (command == "quit" || command == "exit") {
        reply.addString("quitting");
        return false;
    } else if (command == "help") {
        reply.addString(helpMessage);
    } else if (command == "home") {

        bool result = runHome();
        reply.addString((result ? "ack" : "err"));

    } else if (command == "beh") {

        if (cmd.size() != 4) {
            reply.addString("[error] Incorrect number of inputs for behavior command. Example: ``beh body 1 2``");
            reply.addString(helpMessage);
            return true;
        }

        //-- Get the target behavior.
        std::string behavior = cmd.get(1).asString();

        //-- Get the from and to for this behavior.
        int from = cmd.get(2).asInt32();
        int to   = cmd.get(3).asInt32(); // if not an int, will return 0;

        bool result = runBehavior(behavior, from, to);
        reply.addString((result ? "ack" : "err"));

    } else {
        reply.addString("nack");
    }

    return true;
}


double ClipMaker::getPeriod() {
    // control rate set here in seconds.
    return 0.1;
}


bool ClipMaker::updateModule() {
    return true;
}


bool ClipMaker::runHome() {

    //-- Ensure atomicity of communications.
    std::lock_guard<std::mutex> lg(lock);

    //-- Use the same bottle for everyone.
    //yarp::os::Bottle bot;
    


    //-- Set to default expression.
    sendMessage(_expr_port, "set all neu");

    return true;
}


bool ClipMaker::runBehavior(const std::string behavior, const int from, const int to) {

    //-- Don't even process if the same.
    if (from == to) return false;

    // blob|body|spch|gaze|expr
    if (behavior == "blob") {
        return true;
    } else if (behavior == "body") {
        return body(from, to);
    } else if (behavior == "spch") {
        return speech();
    } else if (behavior == "gaze") {
        return gaze(from, to);
    } else if (behavior == "expr") {
        return expression(from, to);
    }

    return false;
}


bool ClipMaker::body(const int from, const int to) {

    //-- Ensure atomicity of communications.
    std::lock_guard<std::mutex> lg(lock);

    bool left_used  = (from == 0 || to == 0);
    bool mid_used   = (from == 1 || to == 1);
    bool right_used = (from == 2 || to == 2);

    int right_arm_peg, left_arm_peg;

    if (left_used) {

        //-- Right arm will point to left peg (0)
        right_arm_peg = 0;

        //-- If the right peg (2) was used, left arm points to right peg.
        left_arm_peg = (right_used ? 2 : 1);

    } else {

        //-- If the left (0) is not used we have...
        right_arm_peg = 1;
        left_arm_peg  = 2;

    }

    //-- Right arm goes first when f:0 t:1, f:0 t:2, and f:1 t:2
    bool right_arm_first = (from < to);







    return true;
}


bool ClipMaker::expression(const int from, const int to) {

    //-- Ensure atomicity of communications.
    std::lock_guard<std::mutex> lg(lock);


    //-- Ensure we're starting at neutral.
    sendMessage(_expr_port, "set all neu");

    //-- Wait a short while before beginning.
    yarp::os::Time::delay(1.0);

    //-- Move the right eyebrow up and down equal to idx for from.
    for (int i = 0; i < (from+1); ++i) {

        //-- Eyebrow up.
        sendMessage(_expr_port, "set reb sur");
        sendMessage(_expr_port, "set mou neu"); // mouth can do strange things...

        //-- Wait.
        yarp::os::Time::delay(_expr_timer);

        //-- Eyebrow down.
        sendMessage(_expr_port, "set reb neu");
        sendMessage(_expr_port, "set mou neu");

        //-- Wait.
        yarp::os::Time::delay(_expr_timer);

    } // repeat.
    
    //-- Wait a little bit between hints.
    yarp::os::Time::delay((1.0 - _expr_timer));

    //-- Move the left eyebrow up and down equal to idx for to.
    for (int i = 0; i < (to+1); ++i) {

        //-- Eyebrow up.
        sendMessage(_expr_port, "set leb sur");
        sendMessage(_expr_port, "set mou neu"); 

        //-- Wait.
        yarp::os::Time::delay(_expr_timer);

        //-- Eyebrow down.
        sendMessage(_expr_port, "set leb neu");
        sendMessage(_expr_port, "set mou neu");

        //-- Wait.
        yarp::os::Time::delay(_expr_timer);

    } // repeat.

    //-- Wait a bit of time then show "correct" and "incorrect" guess gestures.
    yarp::os::Time::delay(3.0);

    sendMessage(_expr_port, "set all neu");

    yarp::os::Time::delay(3.0);

    sendMessage(_expr_port, "set all hap");

    yarp::os::Time::delay(3.0);

    sendMessage(_expr_port, "set all neu");

    yarp::os::Time::delay(3.0);

    sendMessage(_expr_port, "set all shy");

    yarp::os::Time::delay(3.0);

    sendMessage(_expr_port, "set all neu");

    return true;
}


bool ClipMaker::gaze(const int from, const int to) {

    //-- Ensure atomicity of communications.
    std::lock_guard<std::mutex> lg(lock);

    return true;
}


bool ClipMaker::speech() {

    //-- Ensure atomicity of communications.
    std::lock_guard<std::mutex> lg(lock);


    //-- Ensure we're starting at neutral.
    sendMessage(_expr_port, "set all neu"); // reuse expr ports

    //-- Wait a short while before beginning.
    yarp::os::Time::delay(1.0);

    //-- Move the mouth for the specified amount of time.
    double start_time = yarp::os::Time::now();
    while ((yarp::os::Time::now() - start_time) < _spch_timer) {

        sendMessage(_expr_port, "set mou surp");
        yarp::os::Time::delay(0.2);
        sendMessage(_expr_port, "set mou neu");
        yarp::os::Time::delay(0.2);

    }


    return true;
}


//std::string ClipMaker::communicate(const std::string msg, yarp::os::Bottle& command, yarp::os::Bottle& response) {
//
//    //-- Clear out the bottles
//    command.clear(); response.clear();
//
//    //-- Add the intended message.
//    command.addString(msg);
//
//    //-- Write the command and wait for a response.
//    _rpc.write(command, response);
//
//    //-- Return the first item in the bottle
//    //-- Note:
//    //--   If there is other data needed in the response bottle
//    //--   it can be accessed outside this function since we're
//    //--   passed by reference.
//    return response.get(0).asString();
//}


void ClipMaker::sendMessage(yarp::os::Port& port, const std::string msg) {
    
    //-- Make a bottle.
    yarp::os::Bottle bot; bot.clear();

    //-- Parse this string up into individual words.
    std::istringstream ss(msg);

    //-- Add each word to the bottle.
    std::string word;
    while (ss >> word)
        bot.addString(word);

    //-- Write it out on the port.
    port.write(bot);

    return;
}
