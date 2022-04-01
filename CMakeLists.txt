cmake_minimum_required(VERSION 3.21)
project(memLeakDetector)

set(CMAKE_CXX_STANDARD 11)

add_executable(memLeakDetector test.cpp memLeakDetector.h)
