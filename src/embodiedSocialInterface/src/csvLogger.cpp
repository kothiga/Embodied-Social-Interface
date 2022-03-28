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

#include <csvLogger.hpp>


CsvLogger::CsvLogger() {
    _opened = false;
}


CsvLogger::~CsvLogger() {
    if (_opened) {
        _output.close();
    }
}


bool CsvLogger::openLogger(std::string fname) {

    //-- output stream already opened.
    if (_opened) {
        return false;
    }

    //-- Open the specified file for logging to.
    try {
        
        _output.open(fname);

    } catch (std::exception& e) { 
        std::cerr << "Got an exception: " << e.what() << std::endl; 
        return false; 
    }

    //-- Mark the stream as opened.
    _opened = true;

    //-- Init the header information.
    _output << "sys_time" << ","
            << "int_time" << ","
            << "user_id"  << ","
            << "channel"  << ","
            << "hint_id"  << ","
            << "hash"     << ","
            << "dist"     << ","
            << "move"     << ","
            << "from"     << ","
            << "to"            
            << std::endl;

    return true;
}


bool CsvLogger::closeLogger() {

    if (!_opened) {
        return false;
    }

    _output.close();

    return true;
}


void CsvLogger::log(double int_time, std::string user_id, std::string channel, std::string hint_id,  
    std::string hash, std::string distance, int move_number, int from, int to) {

    //-- "sys_time" 
    std::time_t t = std::time(nullptr);
    _output << std::put_time(std::localtime(&t), "%Y/%m/%d_%H:%M:%S") << ",";

    //-- "int_time"
    _output << std::setprecision(10) << int_time << ",";

    //-- "user_id", "channel", "hint_id", "hash",
    _output << user_id << "," << channel << "," << hint_id << "," << hash << ",";

    //-- "dist", "move", "from", "to"
    _output << distance << "," << move_number << "," << from << "," << to;
    
    //-- Move to the next line.
    _output << std::endl;

    return;
}

