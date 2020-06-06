cmake_minimum_required(VERSION 3.13)   # CMake version check
project(uvgz)                         # Create project "uvgz"
set(CMAKE_CXX_STANDARD 17)

# Add main.cpp file of project root directory as source file
set(SOURCE_FILES uvgz.cpp)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(uvgz ${SOURCE_FILES})