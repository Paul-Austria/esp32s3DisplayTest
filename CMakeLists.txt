cmake_minimum_required(VERSION 3.16)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESP32-S3-Touch-LCD-2.8C-Test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(main/2DSofwareRenderer)
