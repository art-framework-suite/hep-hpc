add_subdirectory(Utilities)
add_subdirectory(hdf5)
add_subdirectory(MPI)

add_library(hep_hpc SHARED Ntuple.cpp)

target_link_libraries(hep_hpc
  hep_hpc_hdf5 hep_hpc_Utilities
  ${HDF5_C_LIBRARIES}
  )
