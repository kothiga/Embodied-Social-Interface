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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <yarp/os/Network.h>

#include <yarp/os/Bottle.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>

#include <ncurses.h>

//#include <stateMachine.hpp>
//#include <csvLogger.hpp>


class EmbodiedSocialInterface : public yarp::os::RFModule {
    private:
    /* ============================================================================
    **  Yarp RPC client for sending commands and receiving responses.
    ** ============================================================================ */
    yarp::os::RpcClient rpc;
    yarp::os::Port handler;

    std::string module_name;

    int max_tower_height;
    int window_height;
    int window_width;


    /* ============================================================================
    **  Yarp ports for controlling behavior flow of interface.
    ** ============================================================================ */
    yarp::os::Port audio_port;
    yarp::os::Port video_port;
    yarp::os::Port web_port;


    /* ============================================================================
    **  Control variables for the interface.
    ** ============================================================================ */
    std::vector<std::string> showable_rows;
    int selected_from, selected_to;
    
    int move_count;
    int waiting_count;
    bool game_complete;


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
    std::string communicate(const std::string msg, yarp::os::Bottle& command, yarp::os::Bottle& response);


    /* ============================================================================
    **  
    ** ============================================================================ */
    void keyPressed(int key_num);


    /* ============================================================================
    **  
    ** ============================================================================ */
    void parseShowable(std::string str);


    /* ============================================================================
    **  
    ** ============================================================================ */
    void drawInterface();


    /* ============================================================================
    **  
    ** ============================================================================ */
    void drawWaiting();

};

#endif /* EMBODIED_SOCIAL_INTERFACE_HPP */
