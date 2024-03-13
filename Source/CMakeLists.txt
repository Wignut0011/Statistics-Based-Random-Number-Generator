cmake_minimum_required(VERSION 3.27)
project(Source)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(Libraries)
include_directories(Libraries/FastNoiseLite)

add_executable(Source
        Libraries/FastNoiseLite/FastNoiseLite.h
        Generator.h
        Libraries.h
        main.cpp
        Picker.h
        Seed.h)
target_link_libraries(Source PRIVATE -static)