set (source_files
  MPIInstance.cpp
  detail/throwOnMPIError.cpp
  )

add_library(hep_hpc SHARED ${source_files})

target_link_libraries(hep_hpc ${MPI_C_LIBRARIES})
