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
set(LIBS delta_lib dct_lib)

# Compress and Decompress utilities
add_executable(uvid_compress uvid_compress.cpp)
target_link_libraries(uvid_compress ${LIBS})

add_executable(uvid_decompress uvid_decompress.cpp)
target_link_libraries(uvid_decompress ${LIBS})

# Add meta-target for convenience
add_custom_target(uvid)
add_dependencies(uvid uvid_compress uvid_decompress)
