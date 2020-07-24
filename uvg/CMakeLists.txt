# Basic setup
cmake_minimum_required(VERSION 3.13)
project(uvg)
set(CMAKE_CXX_STANDARD 17)

# Include Boost
set(BOOST_ROOT "/usr/include/boost")
find_package(Boost)

# Add include directories
include_directories(catch) # Catch2 testing library
include_directories(eigen) # Matrix library
#include_directories(include) # Common utilities

# Add sub-modules
add_subdirectory(dct)

# Compress and Decompress utilities
add_executable(uvg_compress uvg_compress.cpp)
target_link_libraries(uvg_compress dct_lib)

add_executable(uvg_decompress uvg_decompress.cpp)
target_link_libraries(uvg_decompress dct_lib)

# Add meta-target for convenience
add_custom_target(uvg)
add_dependencies(uvg uvg_compress uvg_decompress)
