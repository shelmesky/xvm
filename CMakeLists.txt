cmake_minimum_required(VERSION 3.9)
project(XtrmeScript)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE OFF)

add_executable(xasm XASM/xasm.cpp)
add_executable(xvm-console XVM-Console/xvm.cpp XVM-Console/console.cpp)
target_compile_options(xvm-console PRIVATE -Wall -Wuninitialized -fexceptions)
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "XSC/*.cpp")
add_executable(xsc ${SOURCES})
