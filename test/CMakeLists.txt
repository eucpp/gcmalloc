enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_package(Threads REQUIRED)

set(precisegc_TESTS
        gc_inside_method_test.cpp)

include_directories(${PROJECT_SOURCE_DIR}/precisegc/include/)

add_executable(precisegc_test ${precisegc_TESTS})
target_link_libraries(precisegc_test ${GTEST_BOTH_LIBRARIES})
target_link_libraries(precisegc_test ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(precisegc_test libprecisegc)

add_test(all_tests precisegc_test)

add_custom_target(precisegc_test_mem
        COMMAND valgrind --leak-check=full --leak-resolution=med ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/precisegc_test
        DEPENDS precisegc_test)