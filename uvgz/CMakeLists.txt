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
set(PREFIX_SRC prefix.cpp prefix.h)
add_library(prefix_lib STATIC ${PREFIX_SRC})

set(PREFIX_TEST ${PREFIX_SRC} test_prefix.cpp)
add_executable(test_prefix ${PREFIX_TEST})
target_link_libraries(test_prefix prefix_lib)

###
# Gzip
###
set(GZIP_SRC gzip.cpp gzip.h)
set(GZIP_INCLUDE CRC.h shared/output_stream.hpp shared/binary.h)
SET(GZIP_LIBS lzss_lib prefix_lib)
add_library(gzip_lib STATIC ${GZIP_SRC} ${GZIP_INCLUDE})
target_link_libraries(gzip_lib ${GZIP_LIBS})

set(GZIP_TEST ${GZIP_SRC} ${GZIP_INCLUDE} test_gzip.cpp)
add_executable(test_gzip ${GZIP_TEST})
target_link_libraries(test_gzip gzip_lib)

###
# Main
###
add_executable(uvgz main.cpp)
target_link_libraries(uvgz gzip_lib)
