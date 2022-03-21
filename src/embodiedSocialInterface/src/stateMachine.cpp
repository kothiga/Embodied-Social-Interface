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

#include <stateMachine.hpp>


StateMachine::StateMachine(int seed/*=0*/) :
    _rand_seed(seed), _min_steps(5), _max_steps(5) {

    std::srand(_rand_seed);
    
    _states.clear();
    _hint_from = -1;
    _hint_to   = -1;

    _current_state = 0;
    _current_step  = 0;

    return;
}


StateMachine::~StateMachine() {
    _states.clear();
}


void StateMachine::step() {
    
    //-- Advance the step.
    _current_step++;

    //-- If trigger is met, advance to next state.
    int trigger = randInRange(_min_steps,_max_steps);
    if (_current_step >= trigger) {
        
        _current_step = 0;
        _current_state = (_current_state+1) % _states.size();

    }

    return;
}

#include<iostream>
void StateMachine::addState(std::string state) {
    std::cout << state << std::endl;
    _states.push_back(state);
    return;
}


void StateMachine::setCurrentHint(std::string hint) {
    std::istringstream ss(hint);
    ss >> _hint_from >> _hint_to;
    return;
}


void StateMachine::setMinSteps(int val) {
    _min_steps = val;
    return;
}


void StateMachine::setMaxSteps(int val) {
    _max_steps = val;
    return;
}


std::string StateMachine::getCurrentState() {
    return _states[_current_state];
}


std::string StateMachine::getStateHint(std::string direction/*=""*/) {

    if (_states[_current_state] == "none"      ||
        _states[_current_state] == "blob-none" ||
        _states[_current_state] == "icub-none") {
        return _states[_current_state];
    }

    return _states[_current_state]   + "_"
        + std::to_string(_hint_from) + "_" 
        + std::to_string(_hint_to)   + "_"
        + direction;
}


int StateMachine::randInRange(const int min, const int max) {
    return (std::rand() % ((max+1)-min)) + min;
}
