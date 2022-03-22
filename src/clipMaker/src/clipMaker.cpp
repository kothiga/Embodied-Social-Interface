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
        int to   = cmd.get(3).asInt32(); //TODO: what do if not int?

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
    yarp::os::Bottle bot;
    


    //-- Set to default expression.
    bot.clear();
    bot.addString("set all neu");
    _expr_port.write(bot);

    return true;
}


bool ClipMaker::runBehavior(const std::string behavior, const int from, const int to) {

    // blob|body|spch|gaze|expr
    if (behavior == "blob") {
        return true;
    } else if (behavior == "body") {
        return body(from, to);
    } else if (behavior == "spch") {
        return true;
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

    return true;
}


bool ClipMaker::expression(const int from, const int to) {

    //-- Ensure atomicity of communications.
    std::lock_guard<std::mutex> lg(lock);



    return true;
}


bool ClipMaker::gaze(const int from, const int to) {

    //-- Ensure atomicity of communications.
    std::lock_guard<std::mutex> lg(lock);

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


//void ClipMaker::sendMessage(yarp::os::Port& port, const std::string msg) {
//    
//    //-- Put my message in a modem.
//    yarp::os::Bottle bot;
//    bot.addString(msg);
//
//    //-- And throw it in the Cyber Sea.
//    port.write(bot);
//
//    return;
//}
