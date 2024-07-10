cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(pipe_state_machine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)

option(BUILD_TESTS "To compile tests" OFF)
message(STATUS "BUILD_TESTS: ${BUILD_TESTS}")
if (BUILD_TESTS)
    add_subdirectory(test)
endif()
