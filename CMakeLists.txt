cmake_minimum_required(VERSION 3.3)
project(Fluid3D)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    include/Resources.h
    src/Fluid3DApp.cpp
    xcode/Fluid3D.h)

add_executable(Fluid3D ${SOURCE_FILES})