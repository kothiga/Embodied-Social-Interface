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

#ifndef CLIP_MAKER_HPP
#define CLIP_MAKER_HPP

//#include <atomic>
//#include <chrono>
//#include <map>
//#include <memory>

#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include <yarp/os/Network.h>

#include <yarp/os/Bottle.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>


class ClipMaker : public yarp::os::RFModule {

    private:
    /* ============================================================================
    **  Yarp RPC server for sending commands and receiving responses.
    ** ============================================================================ */
    yarp::os::RpcServer _rpc;
    std::string _module_name;

    //-- Expression vars.
    double _expr_timer;


    /* ============================================================================
    **  Yarp ports for controlling behavior flow of interface.
    ** ============================================================================ */
    yarp::os::Port _body_port;
    yarp::os::Port _expr_port;
    yarp::os::Port _gaze_port;


    /* ============================================================================
    **  Control variables for the interface.
    ** ============================================================================ */
    std::mutex lock;


    public:
    /* ============================================================================
    **  Configure the resource finder module.
    **
    ** @param rf
    **
    ** @return success status of opening the rf module.
    ** ============================================================================ */
    bool configure(yarp::os::ResourceFinder &rf);


    /* ============================================================================
    **  Interrupt the resource finder module.
    **
    ** @return success status of interrupting the rf module.
    ** ============================================================================ */
    bool interruptModule();


    /* ============================================================================
    **  Close the resource finder module gracefully.
    **
    ** @return success status of closing the rf module.
    ** ============================================================================ */
    bool close();


    /* ============================================================================
    **  
    ** ============================================================================ */
    bool respond(const yarp::os::Bottle &cmd, yarp::os::Bottle &reply);


    /* ============================================================================
    **  
    ** ============================================================================ */
    double getPeriod();


    /* ============================================================================
    **  
    ** ============================================================================ */
    bool updateModule();


    private:
    /* ============================================================================
    **  
    ** ============================================================================ */
    bool runHome();


    /* ============================================================================
    **  
    ** ============================================================================ */
    bool runBehavior(const std::string behavior, const int from, const int to);


    /* ============================================================================
    **  
    ** ============================================================================ */
    bool body(const int from, const int to);
    bool expression(const int from, const int to);
    bool gaze(const int from, const int to);


    /* ============================================================================
    **  
    ** ============================================================================ */
    //std::string communicate(const std::string msg, yarp::os::Bottle& command, yarp::os::Bottle& response);


    /* ============================================================================
    **  
    ** ============================================================================ */
    //void sendMessage(yarp::os::Port& port, const std::string msg);

};

#endif /* CLIP_MAKER_HPP */
