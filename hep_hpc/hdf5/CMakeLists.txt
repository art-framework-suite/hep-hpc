set (source_files
  Dataset.cpp
  Dataspace.cpp
  File.cpp
  Group.cpp
  PropertyList.cpp
  ScopedErrorHandler.cpp
  errorHandling.cpp
  )

add_library(hep_hpc_hdf5 SHARED ${source_files})

target_link_libraries(hep_hpc_hdf5
  hep_hpc_Utilities
  ${HDF5_C_LIBRARIES}
  )
