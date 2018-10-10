cmake_minimum_required(VERSION 3.11)
project(XtrmeScript)

add_executable(xasm XASM/xasm.cpp)
add_executable(xvm-console XVM-Console/xvm.cpp XVM-Console/console.cpp)
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "XSC/*.cpp")
add_executable(xsc ${SOURCES})