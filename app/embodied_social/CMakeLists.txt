# Copyright: (C) 2022, SIRRL Social and Intelligent Robotics Research Laboratory, University of Waterloo
# Authors: Austin Kothig <austin.kothig@uwaterloo.ca>
# CopyPolicy: Released under the terms of the MIT License.

cmake_minimum_required(VERSION 3.12)


set(appname embodied_social)

file(GLOB conf    ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.ini     ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.xml)
file(GLOB scripts ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.xml  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.sh )

yarp_install(FILES ${conf}    DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${appname})
yarp_install(FILES ${scripts} DESTINATION ${ICUBCONTRIB_APPLICATIONS_TEMPLATES_INSTALL_DIR})
