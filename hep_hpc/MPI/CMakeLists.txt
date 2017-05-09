set (source_files
  MPICommunicator.cpp
  MPIGroup.cpp
  MPIInstance.cpp
  detail/throwMPIError.cpp
  )

set (headers
  MPICommunicator.hpp
  MPIComparison_t.hpp
  MPIGroup.hpp
  MPIInstance.hpp
  throwOnMPIError.hpp
  )

add_library(hep_hpc_MPI SHARED ${source_files})

target_link_libraries(hep_hpc_MPI
  hep_hpc_Utilities
  ${MPI_C_LIBRARIES}
)

install(TARGETS hep_hpc_MPI
  LIBRARY DESTINATION "lib"
  NAMELINK_SKIP
  )

install(FILES ${headers}
  DESTINATION "include/hep_hpc/MPI"
)

install(FILES "detail/throwMPIError.hpp"
  DESTINATION "include/hep_hpc/MPI/detail"
)
