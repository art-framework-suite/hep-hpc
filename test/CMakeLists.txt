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
if (WANT_MPI AND MPI_FOUND)
  if (NOT MPIEXEC_EXECUTABLE STREQUAL "srun" OR MPIEXEC_PREFLAGS)
    add_subdirectory(MPI)
  else()
    message(STATUS "MPI tests skipped due to use of srun: specify -DMPIEXEC_PREFLAGS to activate.")
  endif()
  add_subdirectory(concat_hdf5)
endif()
