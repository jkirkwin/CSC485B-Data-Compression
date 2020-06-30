# Basic setup
cmake_minimum_required(VERSION 3.13)
project(uvzz)
set(CMAKE_CXX_STANDARD 17)

# Include Boost
set(BOOST_ROOT "/usr/include/boost")
find_package(Boost)

# Add include directories
include_directories(catch) # Catch2 testing library
include_directories(include) # Common utilities

# Add sub-modules
add_subdirectory(rle) # Run Length encoding
add_subdirectory(bwt) # Burrows-Wheeler transform
add_subdirectory(mtf) # Move to Front transform

# Compress and Decompress utilities
add_executable(uvzz_compress uvzz_compress.cpp)
target_link_libraries(uvzz_compress rle_lib)

add_executable(uvzz_decompress uvzz_decompress.cpp)