include(Script)
include(AddMPITest)

script(${CMAKE_CURRENT_SOURCE_DIR}/check_return_code)

set(CHECK_RETURN_CODE ${EXECUTABLE_OUTPUT_PATH}/check_return_code)

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
