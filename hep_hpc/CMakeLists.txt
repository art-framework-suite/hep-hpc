set (source_files
  H5File.cpp
  H5PropertyList.cpp
  MPICommunicator.cpp
  MPIGroup.cpp
  MPIInstance.cpp
  ScopedErrorHandler.cpp
  detail/demangle_symbol.cpp
  detail/throwOnMPIError.cpp
  errorHandling.cpp
  )

add_library(hep_hpc SHARED ${source_files})

target_link_libraries(hep_hpc ${MPI_C_LIBRARIES} ${HDF5_C_LIBRARIES})
