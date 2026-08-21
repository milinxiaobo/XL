cmake_minimum_required(VERSION 3.20)

project(XL VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(xl main.cc)
