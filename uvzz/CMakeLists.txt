cmake_minimum_required(VERSION 3.13)
project(uvzz)
set(CMAKE_CXX_STANDARD 17)

set(BOOST_ROOT "/usr/include/boost")
find_package(Boost)

add_executable(uvzz_compress compress.cpp)
add_executable(uvzz_decompress decompress.cpp)
