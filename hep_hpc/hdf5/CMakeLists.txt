set (source_files
  Dataspace.cpp
  File.cpp
  Group.cpp
  Ntuple.cpp
  PropertyList.cpp
  errorHandling.cpp
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
  )

add_library(hep_hpc_hdf5 SHARED ${source_files})

target_link_libraries(hep_hpc_hdf5
  hep_hpc_Utilities
  memstream
  ${HDF5_C_LIBRARIES}
  )

install(TARGETS hep_hpc_hdf5
  LIBRARY DESTINATION "lib"
  NAMELINK_SKIP
)

install(FILES ${headers}
  DESTINATION "include/hep_hpc/hdf5"
  )

install(FILES detail/NtupleDataStructure.hpp
  DESTINATION "include/hep_hpc/hdf5/detail"
  )
