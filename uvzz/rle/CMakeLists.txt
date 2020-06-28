set(RLE_SRC rle.cpp rle.h)
add_library(rle_lib STATIC ${RLE_SRC})

set(RLE_TEST ${RLE_SRC} test_rle.cpp)
add_executable(test_rle ${RLE_TEST})
target_link_libraries(test_rle rle_lib)
