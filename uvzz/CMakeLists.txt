# Basic setup
cmake_minimum_required(VERSION 3.13)
project(uvzz)
set(CMAKE_CXX_STANDARD 17)

# Include Boost
set(BOOST_ROOT "/usr/include/boost")
find_package(Boost)

# Add common directories
include_directories(catch)
include_directories(include)

# Add sub-modules
add_subdirectory(rle)
add_subdirectory(bwt)

# Compress and Decompress utilities
add_executable(uvzz_compress uvzz_compress.cpp)
target_link_libraries(uvzz_compress rle_lib bwt_lib)

add_executable(uvzz_decompress uvzz_decompress.cpp)