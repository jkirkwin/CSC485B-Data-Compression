set(MTF_SRC mtf.cpp mtf.h)
add_library(mtf_lib STATIC ${MTF_SRC})

set(MTF_TEST ${MTF_SRC} test_mtf.cpp)
add_executable(test_mtf ${MTF_TEST})
target_link_libraries(test_mtf mtf_lib)
