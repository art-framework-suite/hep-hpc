add_executable(make_concat_test_file make_concat_test_file.cpp)
target_link_libraries(make_concat_test_file hep_hpc_hdf5)

add_executable(test-no-write-ranks test-no-write-ranks.c)
target_link_libraries(test-no-write-ranks ${HDF5_C_LIBRARIES} ${MPI_C_LIBRARIES})
