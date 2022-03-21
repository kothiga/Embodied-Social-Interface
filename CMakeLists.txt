# Copyright: (C) 2022, SIRRL Social and Intelligent Robotics Research Laboratory, University of Waterloo
# Authors: Austin Kothig <austin.kothig@uwaterloo.ca>
# CopyPolicy: Released under the terms of the MIT License.

cmake_minimum_required(VERSION 3.12)


# Main project.
project(EmbodiedSocialInterface VERSION 0.1.0)
set(PROJECT_DESCRIPTION 
    "Implementation of experimental scenarios regarding social embodiment."
)

include(GNUInstallDirs)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)


# We use
# - AddUninstallTarget (http://robotology.github.io/ycm/gh-pages/v0.8/module/AddUninstallTarget.html)
# from YCM. Files are under the cmake subdirectory.
# See https://github.com/robotology/ycm/
# YCM aquired from icub-common (apt)
find_package(YCM QUIET)
if(YCM_FOUND)
    list(APPEND CMAKE_MODULE_PATH ${YCM_MODULE_PATH})
else(YCM_FOUND)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
endif(YCM_FOUND)


find_package(ICUBcontrib QUIET)
if (ICUBcontrib_FOUND)

    #message("ICUBcontrib found")
    list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})
    #include(ICUBcontribOptions)
    include(ICUBcontribHelpers)
    icubcontrib_set_default_prefix()

else(ICUBcontrib_FOUND)

    message("ICUBcontrib not found")

endif()


# Enable RPATH support for installed binaries and libraries

#include( AddInstallRPATHSupport )
#add_install_rpath_support(
#    BIN_DIRS          "${CMAKE_INSTALL_FULL_BINDIR}"
#    LIB_DIRS          "${CMAKE_INSTALL_FULL_LIBDIR}"
#    INSTALL_NAME_DIR  "${CMAKE_INSTALL_FULL_LIBDIR}"
#    USE_LINK_PATH
#)


# Encourage user to specify a build type (e.g. Release, Debug, etc.), 
#  otherwise set it to Release.
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    endif()
endif()


# Add the uninstall target
include(AddUninstallTarget)

# Add the app directory for the project.
add_subdirectory(app)

# Add the source directory for the project.
add_subdirectory(src)


############################################################
