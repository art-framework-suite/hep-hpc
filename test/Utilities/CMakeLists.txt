####################################
# Test of SimpleRAII.
add_executable(SimpleRAII_t SimpleRAII_t.cpp)
target_link_libraries(SimpleRAII_t gtest)
add_test(SimpleRAII_t ${EXECUTABLE_OUTPUT_PATH}/SimpleRAII_t)
####################################

####################################
# Test of declval behavior.
add_executable(test-declval test-declval.cpp)
target_link_libraries(test-declval hep_hpc_Utilities gtest)
add_test(test-declval ${EXECUTABLE_OUTPUT_PATH}/test-declval)

