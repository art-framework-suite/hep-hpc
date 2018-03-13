add_executable(make_concat_test_file make_concat_test_file.cpp)
target_link_libraries(make_concat_test_file hep_hpc_hdf5)

foreach(tname no-write-ranks no-read-ranks resize)
  add_executable(test-${tname} test-${tname}.c)
  target_link_libraries(test-${tname} ${HDF5_C_LIBRARIES} ${MPI_C_LIBRARIES})
endforeach()
