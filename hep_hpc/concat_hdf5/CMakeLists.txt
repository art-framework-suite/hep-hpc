set (source_files
  HDF5FileConcatenator.cpp
  maybe_report_rank.cpp
  )

set (headers
  ConcatenatedDSInfo.hpp
  FilenameColumnInfo.hpp
  HDF5FileConcatenator.hpp
  maybe_report_rank.hpp
  )

add_library(hep_hpc_concat_hdf5 SHARED ${source_files})

set(lib_link_libraries
  hep_hpc_hdf5
  hep_hpc_Utilities
)

if (HEP_HPC_USE_MPI)
  list(INSERT lib_link_libraries 0 hep_hpc_MPI)
  list(APPEND lib_link_libraries ${MPI_C_LIBRARIES})
endif()
target_link_libraries(hep_hpc_concat_hdf5 ${lib_link_libraries})

install(TARGETS hep_hpc_concat_hdf5
  LIBRARY DESTINATION "lib"
  NAMELINK_SKIP
  )

install(FILES ${headers}
  DESTINATION "include/hep_hpc/concat_hdf5"
)

add_executable(concat_hdf5 concat_hdf5.cpp)
target_link_libraries(concat_hdf5 hep_hpc_concat_hdf5)

install(TARGETS concat_hdf5 DESTINATION "bin")
