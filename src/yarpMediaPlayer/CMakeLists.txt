# Copyright: (C) 2022, SIRRL Social and Intelligent Robotics Research Laboratory, University of Waterloo
# Authors: Austin Kothig <austin.kothig@uwaterloo.ca>
# CopyPolicy: Released under the terms of the MIT License.

cmake_minimum_required(VERSION 3.12)


set(TARGET_NAME yarpMediaPlayer)

set(PROGRAM_PERMISSIONS_DEFAULT
    OWNER_WRITE OWNER_READ OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE)

install(
    PROGRAMS ./${TARGET_NAME}.py 
    PERMISSIONS ${PROGRAM_PERMISSIONS_DEFAULT} 
    DESTINATION bin
)

############################################################
