cmake_minimum_required(VERSION 3.13)   # CMake version check
project(uvgz)                         # Create project "uvgz"
set(CMAKE_CXX_STANDARD 17)

set(BOOST_ROOT "/usr/include/boost")
find_package(Boost)

set(SOURCE_FILES uvgz.cpp binary.h)

# Add executable target
add_executable(uvgz ${SOURCE_FILES})