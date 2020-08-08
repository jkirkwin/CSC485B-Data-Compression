# Basic setup
cmake_minimum_required(VERSION 3.13)
project(uvid)
set(CMAKE_CXX_STANDARD 17)

# Include Boost
set(BOOST_ROOT "/usr/include/boost")
find_package(Boost)

# Add include directories
include_directories(catch) # Catch2 testing library
include_directories(include) # Common utilities

# Add sub-modules
add_subdirectory(dct)
add_subdirectory(delta) # todo rename this to "binary"?
add_subdirectory(include) # Add tests for common headers

# Shared decompression logic
add_library(decode uvid_decode.cpp uvid_decode.h)


# Compress and Decompress utilities
set(LIBS delta_lib dct_lib)

add_executable(uvid_compress uvid_compress.cpp)
target_link_libraries(uvid_compress ${LIBS})

add_executable(uvid_decompress uvid_decompress.cpp)
target_link_libraries(uvid_decompress ${LIBS})

# Add meta-target for convenience
add_custom_target(uvid)
add_dependencies(uvid uvid_compress uvid_decompress)
