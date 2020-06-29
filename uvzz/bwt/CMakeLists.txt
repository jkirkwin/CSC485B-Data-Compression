set(BWT_SRC bwt.h)

set(BWT_TEST ${BWT_SRC} test_bwt.cpp)
add_executable(test_bwt ${BWT_TEST})