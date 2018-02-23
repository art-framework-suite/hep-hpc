add_executable(make_concat_test_file make_concat_test_file.cpp)
target_link_libraries(make_concat_test_file hep_hpc_hdf5)

foreach(op write read)
  add_executable(test-no-${op}-ranks test-no-${op}-ranks.c)
  target_link_libraries(test-no-${op}-ranks ${HDF5_C_LIBRARIES} ${MPI_C_LIBRARIES})
endforeach()
