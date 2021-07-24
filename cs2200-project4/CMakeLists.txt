#
# Multi-threaded OS Simulation for CS 2200, Project 4
# Summer 2021
#
# This file contains the CMakeLists stubs in case you wish to debug your program
# with IDEs such as Clion.
#
# The default configuration has Thread Sanitizer enabled. Remove the
# '-fsanitize=thread' argument at the end of 'set(CMAKE_C_FLAGS...' to disable
# Thread Sanitizer. We strongly suggest that you have thread sanitizer enabled
# when debugging your project.
#

cmake_minimum_required(VERSION 3.13)

project("project4" C)

set(CC /usr/bin/gcc)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wsign-conversion -Wpointer-arith -Wcast-qual -Wwrite-strings -Wshadow -Wmissing-prototypes -Wpedantic -Wwrite-strings -g -std=gnu99 -lm -fsanitize=thread")
file(GLOB sources src/*)

include_directories(src)

add_executable(cs2200_project4 ${sources})
