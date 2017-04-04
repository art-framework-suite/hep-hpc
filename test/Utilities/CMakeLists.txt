####################################
# Test of SimpleRAII.
add_executable(SimpleRAII_t SimpleRAII_t.cpp)
add_test(SimpleRAII_t ${EXECUTABLE_OUTPUT_PATH}/SimpleRAII_t)
target_link_libraries(SimpleRAII_t gtest)
####################################
