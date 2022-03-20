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

#ifndef CSV_LOGGER_HPP
#define CSV_LOGGER_HPP

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>


typedef unsigned long long ull;


class CsvLogger {

    private:
    /* ============================================================================
    **  Internal members for the logger.
    ** ============================================================================ */
    std::ofstream _output;
    bool _opened;


    public:
    /* ============================================================================
    **  Main Constructor.
    ** ============================================================================ */
    CsvLogger();


    /* ============================================================================
    **  Destructor.
    ** ============================================================================ */
    ~CsvLogger();


    /* ============================================================================
    **  Open the logger.
    ** 
    ** @param fname  file name for csv output.
    **
    ** @return success of opening the file stream.
    ** ============================================================================ */
    bool openLogger(std::string fname);


    /* ============================================================================
    **  Close the logger.
    **
    ** @return success of closing the file stream.
    ** ============================================================================ */
    bool closeLogger();


    /* ============================================================================
    **  Log an entry into the output stream.
    ** ============================================================================ */
    void log(double int_time, std::string user_id, std::string channel, std::string hint_id, 
             std::string hash, std::string distance, int move_number, int from, int to);

};

#endif /* CSV_LOGGER_HPP */
