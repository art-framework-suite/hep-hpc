set (source_files
  MPICommunicator.cpp
  MPIGroup.cpp
  MPIInstance.cpp
  detail/throwMPIError.cpp
  )

add_library(hep_hpc_MPI SHARED ${source_files})

target_link_libraries(hep_hpc_MPI
  hep_hpc_Utilities
  ${MPI_C_LIBRARIES}
)
