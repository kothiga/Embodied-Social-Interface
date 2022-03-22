# Copyright: (C) 2022, SIRRL Social and Intelligent Robotics Research Laboratory, University of Waterloo
# Authors: Austin Kothig <austin.kothig@uwaterloo.ca>
# CopyPolicy: Released under the terms of the MIT License.

cmake_minimum_required(VERSION 3.12)


set(TARGET_NAME clipMaker)

find_package(YARP REQUIRED)

set(${TARGET_NAME}_SRC
    src/main.cpp
    src/clipMaker.cpp
)

set(${TARGET_NAME}_HDR
    include/clipMaker.hpp
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
)

install(
    TARGETS        ${TARGET_NAME}
    DESTINATION    bin  
)

############################################################
