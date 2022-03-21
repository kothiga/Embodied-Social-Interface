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

#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

#include <algorithm>
#include <cstdlib>
#include <map>
#include <sstream>
#include <string>
#include <vector>


class StateMachine {

    private:
    /* ============================================================================
    **  Internal members for the state machine.
    ** ============================================================================ */
    std::vector<std::string>  _states;
    int _rand_seed;
    int _min_steps,    _max_steps;
    int _current_step, _current_state;
    int _hint_from,    _hint_to;


    public:
    /* ============================================================================
    **  Main Constructor.
    ** ============================================================================ */
    StateMachine(int seed=0);


    /* ============================================================================
    **  Destructor.
    ** ============================================================================ */
    ~StateMachine();


    /* ============================================================================
    **  Advance the step and maintain the current state.
    ** ============================================================================ */
    void step();


    /* ============================================================================
    **  Add the state to the list.
    ** ============================================================================ */
    void addState(std::string state);


    /* ============================================================================
    **  Set the current hint for the board state.
    ** ============================================================================ */
    void setCurrentHint(std::string hint);


    /* ============================================================================
    **  Set the min number of steps for a state (inclusive).
    ** ============================================================================ */
    void setMinSteps(int val);


    /* ============================================================================
    **  Set the max number of steps for a state (inclusive).
    ** ============================================================================ */
    void setMaxSteps(int val);


    /* ============================================================================
    **  Get the current state information.
    ** ============================================================================ */
    std::string getCurrentState();


    /* ============================================================================
    **  Concatenate the current state with the hint and return.
    ** ============================================================================ */
    std::string getStateHint(std::string direction="");


    private:
    /* ============================================================================
    **  Get a random number within range [min, max]
    **
    ** @param min : Minimum value to receive (inclusive).
    ** @param max : Maximum value to receive (inclusive).
    **
    ** @return random integer.
    ** ============================================================================ */
    int randInRange(const int min, const int max);

};

#endif /* STATE_MACHINE_HPP */
