cmake_minimum_required(VERSION 3.6)
project(esp8266sensorBroadcaster)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(esp8266sensorBroadcaster ${SOURCE_FILES})