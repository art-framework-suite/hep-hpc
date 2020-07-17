set (source_files
  Dataspace.cpp
  File.cpp
  Group.cpp
  Ntuple.cpp
  PropertyList.cpp
  errorHandling.cpp
  write_attribute.cpp
  detail/NtupleDataStructure.cpp
  )

set (headers
  Column.hpp
  Dataset.hpp
  Dataspace.hpp
  Datatype.hpp
  Exception.hpp
  File.hpp
  Group.hpp
  HID_t.hpp
  Ntuple.hpp
  PropertyList.hpp
  Resource.hpp
  ResourceStrategy.hpp
  errorHandling.hpp
  make_column.hpp
  make_ntuple.hpp
  write_attribute.hpp
  )

add_library(hep_hpc_hdf5 SHARED ${source_files})

set(HEP_HPC_HDF5_LIBRARIES
  hep_hpc_Utilities
  ${HDF5_C_LIBRARIES}
  )
if (NOT HAS_OPEN_MEMSTREAM)
  list(APPEND HEP_HPC_HDF5_LIBRARIES memstream)
endif()
target_link_libraries(hep_hpc_hdf5 ${HEP_HPC_HDF5_LIBRARIES})

install(TARGETS hep_hpc_hdf5
  LIBRARY DESTINATION "lib"
  NAMELINK_SKIP
)

install(FILES ${headers}
  DESTINATION "include/hep_hpc/hdf5"
  )

install(FILES detail/NtupleDataStructure.hpp
  detail/hdf5_compat.h
  DESTINATION "include/hep_hpc/hdf5/detail"
  )
