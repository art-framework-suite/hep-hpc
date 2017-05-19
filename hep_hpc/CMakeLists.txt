configure_file(${CMAKE_CURRENT_SOURCE_DIR}/detail/config.hpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/detail/config.hpp
  @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/detail/config.hpp
  DESTINATION "include/hep_hpc/detail"
  )

add_subdirectory(Utilities)
add_subdirectory(hdf5)
if (WANT_MPI AND MPI_FOUND)
  add_subdirectory(MPI)
endif()
