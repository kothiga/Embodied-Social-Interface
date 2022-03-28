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
    ok &= _expr_port.open(this->getName() + "/expr:o");
    if (!ok) {
        yInfo("%s: Unable to open control ports!!", this->getName().c_str());
        return false;
    }

    _robot_name = rf.check("robot", yarp::os::Value("icubSim"), "robot name (string)").asString();


    //-- Init the body vars.
    yarp::os::Bottle* la_home = rf.find("left_arm_home").asList();
    yarp::os::Bottle* ra_home = rf.find("right_arm_home").asList();
    yarp::os::Bottle* la_rp   = rf.find("body_left_arm_right_peg").asList();
    yarp::os::Bottle* la_mp   = rf.find("body_left_arm_mid_peg").asList();
    yarp::os::Bottle* ra_mp   = rf.find("body_right_arm_mid_peg").asList();
    yarp::os::Bottle* ra_lp   = rf.find("body_right_arm_left_peg").asList();

    ok &= loadBottleAsVec(la_home, _left_arm_home);
    ok &= loadBottleAsVec(ra_home, _right_arm_home);
    ok &= loadBottleAsVec(la_rp,   _left_arm_right_peg);
    ok &= loadBottleAsVec(la_mp,   _left_arm_mid_peg);
    ok &= loadBottleAsVec(ra_mp,   _right_arm_mid_peg);
    ok &= loadBottleAsVec(ra_lp,   _right_arm_left_peg);
    if (!ok) {
        yInfo("%s: Unable to joint positions!!", this->getName().c_str());
    }


    //-- Prepare the joint interfaces.
    yarp::os::Property opt_left_arm;
    opt_left_arm.put("device","remote_controlboard");
    opt_left_arm.put("remote","/"+_robot_name+"/left_arm");
    opt_left_arm.put("local","/"+this->getName()+"/left_arm");
    if (!_left_arm.open(opt_left_arm)) {
        yInfo("%s: Unable to open %s left arm!!", _robot_name.c_str(), this->getName().c_str());
        return false;
    }

    yarp::os::Property opt_right_arm;
    opt_right_arm.put("device","remote_controlboard");
    opt_right_arm.put("remote","/"+_robot_name+"/right_arm");
    opt_right_arm.put("local","/"+this->getName()+"/right_arm");
    if (!_right_arm.open(opt_right_arm)) {
        yInfo("%s: Unable to open %s right arm!!", _robot_name.c_str(), this->getName().c_str());
        return false;
    }


    //-- Attach the position controllers.
    ok &= _left_arm.view(_left_arm_pos);
    ok &= _right_arm.view(_right_arm_pos);
    if (!ok) {
        yInfo("%s: Unable to open position interfaces!!", this->getName().c_str());
        return false;
    }


    //-- Set the speed for all joints.
    _num_joints = rf.check("num_joints", yarp::os::Value(16),   "num joints (int)").asInt32();
    _body_speed = rf.check("body_speed", yarp::os::Value(30.0), "body speed (double)").asFloat64();
    for (int joint = 0; joint < _num_joints; ++joint) {
        _left_arm_pos->setRefSpeed(joint, _body_speed);
        _right_arm_pos->setRefSpeed(joint, _body_speed);
    }


    //-- Init the expression vars.
    _expr_timer = rf.check("expr_timer", yarp::os::Value(0.2), "expressions (double)").asFloat64();


    //-- Init the gaze vars.
    yarp::os::Property opt_gaze;
    opt_gaze.put("device","gazecontrollerclient");
    opt_gaze.put("remote","/iKinGazeCtrl");
    opt_gaze.put("local","/"+this->getName()+"/gaze");
    if (!_gaze_client.open(opt_gaze)) {
        yInfo("%s: Unable to open %s gaze control client!!", _robot_name.c_str(), this->getName().c_str());
        return false;
    }


    //-- Attach the gaze controller.
    ok &= _gaze_client.view(_gaze);
    if (!ok) {
        yInfo("%s: Unable to open %s gaze control client!!", _robot_name.c_str(), this->getName().c_str());
        return false;
    }


    //-- Set up a restoration point.
    _gaze->storeContext(&_gaze_startup_context);


    //-- Set trajectory times.
    _gaze_speed = rf.check("gaze_speed", yarp::os::Value(1.0), "gaze speed (double)").asFloat64();
    _gaze->setNeckTrajTime(_gaze_speed);
    _gaze->setEyesTrajTime(0.8);


    //-- Load variables for gaze positions
    _gaze_home.push_back(rf.check("gaze_home_x", yarp::os::Value(0.0), "gaze vars (double)").asFloat64());
    _gaze_home.push_back(rf.check("gaze_home_y", yarp::os::Value(0.0), "gaze vars (double)").asFloat64());
    _gaze_home.push_back(rf.check("gaze_home_z", yarp::os::Value(0.0), "gaze vars (double)").asFloat64());

    _gaze_left.push_back(rf.check("gaze_left_x", yarp::os::Value(0.0), "gaze vars (double)").asFloat64());
    _gaze_left.push_back(rf.check("gaze_left_y", yarp::os::Value(0.0), "gaze vars (double)").asFloat64());
    _gaze_left.push_back(rf.check("gaze_left_z", yarp::os::Value(0.0), "gaze vars (double)").asFloat64());

    _gaze_mid.push_back(rf.check("gaze_mid_x", yarp::os::Value(0.0), "gaze vars (double)").asFloat64());
    _gaze_mid.push_back(rf.check("gaze_mid_y", yarp::os::Value(0.0), "gaze vars (double)").asFloat64());
    _gaze_mid.push_back(rf.check("gaze_mid_z", yarp::os::Value(0.0), "gaze vars (double)").asFloat64());

    _gaze_right.push_back(rf.check("gaze_right_x", yarp::os::Value(0.0), "gaze vars (double)").asFloat64());
    _gaze_right.push_back(rf.check("gaze_right_y", yarp::os::Value(0.0), "gaze vars (double)").asFloat64());
    _gaze_right.push_back(rf.check("gaze_right_z", yarp::os::Value(0.0), "gaze vars (double)").asFloat64());


    //-- Init the speech vars.
    _spch_timer = rf.check("spch_timer", yarp::os::Value(3.0), "speech duration (double)").asFloat64();

    return true;
}


bool ClipMaker::interruptModule() {
    
    //-- Interrupt the ports.
    _rpc.interrupt();
    _expr_port.interrupt();

    _gaze->stopControl();
    _gaze->restoreContext(_gaze_startup_context);

    return true;
}


bool ClipMaker::close() {

    //-- Close the yarp ports.
    _rpc.close();
    _expr_port.close();

    _left_arm.close();
    _right_arm.close();

    _gaze_client.close();

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

    // TODO: <REMOVE>
    } else if (command == "gz") {

        std::string which_peg = cmd.get(1).asString();
        double x = cmd.get(2).asFloat64();
        double y = cmd.get(3).asFloat64();
        double z = cmd.get(4).asFloat64();

        if (which_peg == "left") {
            _gaze_left[0] = x;
            _gaze_left[1] = y;
            _gaze_left[2] = z;
        } else if (which_peg == "mid") {
            _gaze_mid[0] = x;
            _gaze_mid[1] = y;
            _gaze_mid[2] = z;
        } else if (which_peg == "right") {
            _gaze_right[0] = x;
            _gaze_right[1] = y;
            _gaze_right[2] = z;
        }
        

        reply.addString("ack");
    
    //TODO: <\REMOVE>

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

    //-- Set the body position to home.
    _left_arm_pos->positionMove(_left_arm_home.data());
    _right_arm_pos->positionMove(_right_arm_home.data());
    
    //-- Set gaze at home position.
    //_gaze->restoreContext(_gaze_startup_context);
    yarp::sig::Vector home_pos(_gaze_home.size(), _gaze_home.data());
    _gaze->lookAtAbsAngles(home_pos);

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

    double* r_arm_pos = ( right_arm_peg == 1 ? _right_arm_mid_peg.data() : _right_arm_left_peg.data() );
    double* l_arm_pos = ( left_arm_peg  == 1 ? _left_arm_mid_peg.data()  : _left_arm_right_peg.data() );

    //-- If the right arm goes first, do it.
    if (right_arm_first) {

        _right_arm_pos->positionMove(r_arm_pos);
        yarp::os::Time::delay(0.4);
        _left_arm_pos->positionMove(l_arm_pos);
        yarp::os::Time::delay(3.0);
        _left_arm_pos->positionMove(_left_arm_home.data());
        yarp::os::Time::delay(0.4);
        _right_arm_pos->positionMove(_right_arm_home.data());
        yarp::os::Time::delay(3.0);
        
    } else {

        _left_arm_pos->positionMove(l_arm_pos);
        yarp::os::Time::delay(0.4);
        _right_arm_pos->positionMove(r_arm_pos);
        yarp::os::Time::delay(3.0);
        _right_arm_pos->positionMove(_right_arm_home.data());
        yarp::os::Time::delay(0.4);
        _left_arm_pos->positionMove(_left_arm_home.data());
        yarp::os::Time::delay(3.0);

    }

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

    //-- Set the gaze to the home position.
    yarp::sig::Vector home_pos(_gaze_home.size(), _gaze_home.data());

    _gaze->lookAtAbsAngles(home_pos);
    yarp::os::Time::delay(3.0);

    //-- Figure out the two places we're looking at.
    double* from_data;
    if (from == 0) from_data = _gaze_left.data();
    else if (from == 1) from_data = _gaze_mid.data();
    else if (from == 2) from_data = _gaze_right.data();

    double* to_data;
    if (to == 0) to_data = _gaze_left.data();
    else if (to == 1) to_data = _gaze_mid.data();
    else if (to == 2) to_data = _gaze_right.data();

    yarp::sig::Vector first_pos(3, from_data);
    yarp::sig::Vector second_pos(3, to_data);

    for (int idx = 0; idx < 3; ++idx) {
        _gaze->lookAtAbsAngles(first_pos);
        yarp::os::Time::delay(1.2);
        _gaze->lookAtAbsAngles(second_pos);
        yarp::os::Time::delay(1.2);
    }
    
    yarp::os::Time::delay(3.0);
    _gaze->lookAtAbsAngles(home_pos);

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


bool ClipMaker::loadBottleAsVec(yarp::os::Bottle* bot, std::vector<double>& vec) {

    //-- If bottle couldn't be loaded, return false.
    if (bot == NULL) { return false; }

    //-- Clear out the vector and push the vectors contents into it.
    vec.clear();
    for (int idx = 0; idx < bot->size(); ++idx) {
        vec.push_back(bot->get(idx).asFloat64());
    }

    //-- Return success.
    return true;
}
