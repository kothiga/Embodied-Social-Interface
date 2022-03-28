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

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <yarp/os/Network.h>
#include <yarp/os/LogStream.h>

#include <clipMaker.hpp>


int main (int argc, char **argv) {

    //-- Init the yarp network.
    yarp::os::Network yarp;
    if (!yarp.checkNetwork()) {
        yError() << "Cannot make connection with the YARP server!!";
        return EXIT_FAILURE;
    }

    //-- Config the resource finder.
    yarp::os::ResourceFinder rf;
    rf.setVerbose(false);
	rf.setDefaultConfigFile("config.ini");    // overridden by --from parameter
	rf.setDefaultContext("clip_maker");  // overridden by --context parameter
    rf.configure(argc,argv);

    //-- Run the interface and return its status.
    ClipMaker clip_maker;
    return clip_maker.runModule(rf);
}
