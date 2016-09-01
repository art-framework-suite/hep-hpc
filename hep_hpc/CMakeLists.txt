set (source_files
  H5File.cpp
  MPICommunicator.cpp
  MPIGroup.cpp
  MPIInstance.cpp
  detail/throwOnMPIError.cpp
  )

add_library(hep_hpc SHARED ${source_files})

target_link_libraries(hep_hpc ${MPI_C_LIBRARIES} ${HDF5_C_LIBRARIES})
