include(CheckCXXSourceCompiles)
# Propagate the project's warning/error flags so the probe matches real build
# conditions (-Wall -Wextra -Werror -pedantic are set via add_compile_options).
set(CMAKE_REQUIRED_FLAGS "-Wall -Wextra -Werror -pedantic")
check_cxx_source_compiles("
template <typename T>
struct A {
  ~A();
};

template <typename T>
A<T>::~A<T>() {}

int main() {
  (void)A<int>{};
  return 0;
}
" HEP_HPC_QUALIFY_DESTRUCTOR_NAMES)
unset(CMAKE_REQUIRED_FLAGS)

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
add_subdirectory(concat_hdf5)
