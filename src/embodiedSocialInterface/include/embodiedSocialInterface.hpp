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

#ifndef EMBODIED_SOCIAL_INTERFACE_HPP
#define EMBODIED_SOCIAL_INTERFACE_HPP

//#include <atomic>
//#include <chrono>
//#include <map>
//#include <memory>
//#include <string>

#include <yarp/os/Network.h>
#include <yarp/os/RFModule>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Bottle.h>

//#include <ncurses.h>

//#include <stateMachine.hpp>
//#include <csvLogger.hpp>


class EmbodiedSocialInterface : public yarp::os::RFModule {
    private:
    /* ============================================================================
    **  Yarp RPC client for sending commands and receiving responses.
    ** ============================================================================ */
    yarp::os::RpcClient rpc;
    yarp::os::Port handler;


    public:
    /* ============================================================================
    **  Configure the resource finder module.
    **
    ** @param rf
    **
    ** @return success status of opening the rf module.
    ** ============================================================================ */
    bool configure(ResourceFinder &rf);


    /* ============================================================================
    **  Close the resource finder gracefully.
    **
    ** @return success status of closing the rf module.
    ** ============================================================================ */
    bool close();


    /* ============================================================================
    **  
    ** ============================================================================ */
    bool respond(const Bottle &cmd, Bottle &reply);


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
    void temp();

};

#endif /* EMBODIED_SOCIAL_INTERFACE_HPP */
