cmake_minimum_required(VERSION 3.13)
project(uvgz)
set(CMAKE_CXX_STANDARD 17)

set(BOOST_ROOT "/usr/include/boost")
find_package(Boost)

###
# LZSS
###
set(LZSS_SRC lzss.cpp lzss.h lzss_backref.h)
add_library(lzss_lib STATIC ${LZSS_SRC})

set(LZSS_TEST ${LZSS_SRC} test_lzss.cpp)
add_executable(test_lzss ${LZSS_TEST})
target_link_libraries(test_lzss lzss_lib)

###
# Prefix Coding
###
set(PREFIX_SRC prefix.h)
# TODO add implementation and tests

###
# Gzip
###
set(GZIP_SRC gzip.cpp gzip.h)
set(GZIP_INCLUDE CRC.h shared/output_stream.hpp shared/binary.h)
add_library(gzip_lib STATIC ${GZIP_SRC} ${GZIP_INCLUDE})
# TODO add testing target
# TODO link lzss and prefix libs

###
# Main
###
add_executable(uvgz main.cpp)
# todo link libraries as needed