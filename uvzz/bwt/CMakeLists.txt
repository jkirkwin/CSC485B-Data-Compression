set(BWT_SRC bwt.h bwt.cpp)
add_library(bwt_lib STATIC ${BWT_SRC})

set(BWT_TEST ${BWT_SRC} test_bwt.cpp)
add_executable(test_bwt ${BWT_TEST})
target_link_libraries(test_bwt bwt_lib)
