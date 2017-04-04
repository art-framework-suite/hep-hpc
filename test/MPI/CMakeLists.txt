include(AddMPITest)

####################################
# Tests of hep_hpc::MPI.
#
add_executable(MPIInstance_t MPIInstance_t.cpp)
target_link_libraries(MPIInstance_t hep_hpc_MPI)
add_executable(MPIInstance_except_t MPIInstance_except_t.cpp)
target_link_libraries(MPIInstance_except_t hep_hpc_MPI ${MPI_C_LIBRARIES})

add_mpi_test(MPIInstance_t NP 1 ${EXECUTABLE_OUTPUT_PATH}/MPIInstance_t)
add_mpi_test(MPIInstance_t ${EXECUTABLE_OUTPUT_PATH}/MPIInstance_t)
add_mpi_test(MPIInstance_except_t NP 1
  WRAP_COMMAND ${CHECK_RETURN_CODE} 5
  COMMAND ${EXECUTABLE_OUTPUT_PATH}/MPIInstance_except_t)
add_mpi_test(MPIInstance_except_t
  WRAP_COMMAND ${CHECK_RETURN_CODE} 5
  COMMAND ${EXECUTABLE_OUTPUT_PATH}/MPIInstance_except_t)
####################################

####################################
# Test of some simple HDF5 behaviors.
#
add_executable(basic_hdf5_mpi_t basic_hdf5_mpi_t.c)
target_link_libraries(basic_hdf5_mpi_t ${HDF5_C_LIBRARIES} ${MPI_C_LIBRARIES})

add_mpi_test(basic_hdf5_mpi_t NP 1 ${EXECUTABLE_OUTPUT_PATH}/basic_hdf5_mpi_t)
add_mpi_test(basic_hdf5_mpi_t ${EXECUTABLE_OUTPUT_PATH}/basic_hdf5_mpi_t)
####################################

####################################
# Test of heritability of communicator error handlers.
add_executable(CommErrorsReturn_t CommErrorsReturn_t.cpp)
target_link_libraries(CommErrorsReturn_t hep_hpc_MPI ${MPI_C_LIBRARIES})
####################################

####################################
# Static library containing MPI-aware main.
add_library(mpitest_main STATIC MPITestMain.cpp)
target_link_libraries(mpitest_main hep_hpc_MPI gtest)

####################################
# MPIGroup test.
add_executable(MPIGroup_t MPIGroup_t.cpp)
target_link_libraries(MPIGroup_t mpitest_main hep_hpc_MPI gtest)
add_mpi_test(MPIGroup_t NP 5 ${EXECUTABLE_OUTPUT_PATH}/MPIGroup_t)
####################################

####################################
# MPICommunicator test.
add_executable(MPICommunicator_t MPICommunicator_t.cpp)
target_link_libraries(MPICommunicator_t mpitest_main hep_hpc_MPI gtest)
add_mpi_test(MPICommunicator_t NP 5 ${EXECUTABLE_OUTPUT_PATH}/MPICommunicator_t)
####################################

