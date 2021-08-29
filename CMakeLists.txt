cmake_minimum_required(VERSION 3.20)
project(ladspa_reverb)

set(CMAKE_CXX_STANDARD 17)

add_library(ladspa_reverb SHARED ladspa_reverb.cpp port_description.cpp port_description.hpp)
