set(DELTA_SRC delta.h delta.cpp)
add_library(delta_lib STATIC ${DELTA_SRC})

set(DELTA_TEST ${DELTA_SRC} test_delta.cpp)
add_executable(test_delta ${DELTA_TEST})
target_link_libraries(test_delta delta_lib)
