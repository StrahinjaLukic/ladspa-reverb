cmake_minimum_required(VERSION 3.20)
project(ladspa_reverb)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} SHARED
    ladspa_reverb.cpp
    ladspa_reverb.h
    lifetime_handler.cpp
        port_id.h)

enable_testing()
add_subdirectory(test)