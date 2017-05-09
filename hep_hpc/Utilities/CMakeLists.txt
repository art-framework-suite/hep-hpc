set (source_files
  demangle_symbol.cpp
  )

set (headers
  DefaultedSimpleType.hpp
  SimpleRAII.hpp
  demangle_symbol.hpp
  is_nothrow_swappable_all.hpp
  )

add_library(hep_hpc_Utilities SHARED ${source_files})

install(TARGETS hep_hpc_Utilities
  LIBRARY DESTINATION "lib"
  NAMELINK_SKIP
  )

install(FILES ${headers}
  DESTINATION "include/hep_hpc/Utilities"
)
