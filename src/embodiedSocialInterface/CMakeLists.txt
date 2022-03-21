# Copyright: (C) 2022, SIRRL Social and Intelligent Robotics Research Laboratory, University of Waterloo
# Authors: Austin Kothig <austin.kothig@uwaterloo.ca>
# CopyPolicy: Released under the terms of the MIT License.

cmake_minimum_required(VERSION 3.12)


set(TARGET_NAME embodiedSocialInterface)

find_package(YARP REQUIRED)

set(${TARGET_NAME}_SRC
    src/main.cpp
    src/embodiedSocialInterface.cpp
    src/stateMachine.cpp
    src/csvLogger.cpp
)

set(${TARGET_NAME}_HDR
    include/embodiedSocialInterface.hpp
    include/stateMachine.hpp
    include/csvLogger.hpp
)

add_executable(
    ${TARGET_NAME} 
    ${${TARGET_NAME}_HDR}
    ${${TARGET_NAME}_SRC}
)

target_include_directories(
    ${TARGET_NAME}
    PRIVATE 
    include
)

target_link_libraries(
    ${TARGET_NAME}
    ${YARP_LIBRARIES}
    ncursesw
)

install(
    TARGETS        ${TARGET_NAME}
    DESTINATION    bin  
)

############################################################
