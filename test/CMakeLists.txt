########################################################################
# Put functionality tests here.
#
########################################################################
include(Script)
include(AddMPITest)

# Install check_return_code in bin/.
script(${CMAKE_CURRENT_SOURCE_DIR}/check_return_code)

# Store for easy reference.
set(CHECK_RETURN_CODE ${EXECUTABLE_OUTPUT_PATH}/check_return_code)

####################################
# Tests of hep_hpc::MPI.
#
add_executable(MPI_t MPI_t.cpp)
target_link_libraries(MPI_t hep_hpc)
add_executable(MPI_except_t MPI_except_t.cpp)
target_link_libraries(MPI_except_t hep_hpc ${MPI_C_LIBRARIES})

add_mpi_test(MPI_t NP 1 ${EXECUTABLE_OUTPUT_PATH}/MPI_t)
add_mpi_test(MPI_t ${EXECUTABLE_OUTPUT_PATH}/MPI_t)
add_mpi_test(MPI_except_t NP 1
  WRAP_COMMAND ${CHECK_RETURN_CODE} 5
  COMMAND ${EXECUTABLE_OUTPUT_PATH}/MPI_except_t)
add_mpi_test(MPI_except_t
  WRAP_COMMAND ${CHECK_RETURN_CODE} 5
  COMMAND ${EXECUTABLE_OUTPUT_PATH}/MPI_except_t)
####################################

####################################
# Test of some simple HDF5 behaviors.
#
add_executable(basic_hdf5_mpi_t basic_hdf5_mpi_t.c)
target_link_libraries(basic_hdf5_mpi_t ${HDF5_C_LIBRARIES} ${MPI_C_LIBRARIES})

add_mpi_test(basic_hdf5_mpi_t NP 1 ${EXECUTABLE_OUTPUT_PATH}/basic_hdf5_mpi_t)
add_mpi_test(basic_hdf5_mpi_t ${EXECUTABLE_OUTPUT_PATH}/basic_hdf5_mpi_t)
####################################
