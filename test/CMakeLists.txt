####################################
# Set up CTest
enable_testing()

####################################
# Set up Google Test.
include_directories(gtest/googletest/include)

####################################
# Make sure we can use assert() in tests.
remove_definitions("-DNDEBUG")

####################################
# Install test utility script.
include(Script)

# Install check_return_code in bin/.
script(${CMAKE_CURRENT_SOURCE_DIR}/check_return_code)

# Store for easy reference.
set(CHECK_RETURN_CODE ${EXECUTABLE_OUTPUT_PATH}/check_return_code)

########################################################################
# Tests in lower level subdirectories.
########################################################################
add_subdirectory(Utilities)
add_subdirectory(hdf5)
add_subdirectory(MPI)

########################################################################
# Put functionality tests here.
########################################################################

####################################
# Column test.
#add_executable(Column_t Column_t.cpp)
#add_test(NAME Column_t COMMAND ${EXECUTABLE_OUTPUT_PATH}/Column_t)
#target_link_libraries(Column_t hep_hpc)
####################################

####################################
# Ntuple test.
add_executable(Ntuple_t Ntuple_t.cpp)
add_test(NAME Ntuple_t COMMAND ${EXECUTABLE_OUTPUT_PATH}/Ntuple_t)
target_link_libraries(Ntuple_t hep_hpc)
####################################
