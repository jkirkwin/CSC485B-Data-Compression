set(DCT_SRC dct.h dct.cpp)
add_library(dct_lib STATIC ${DCT_SRC})

set(DCT_TEST ${DCT_SRC} test_dct.cpp)
add_executable(test_dct ${DCT_TEST})
target_link_libraries(test_dct dct_lib)
