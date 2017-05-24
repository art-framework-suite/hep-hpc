enable_testing()

add_library(memstream SHARED memstream.c)
# Make sure that asserts are evaluated in test.c, and we are permissive
# about known warnings.

if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  set_source_files_properties(test.c PROPERTIES COMPILE_FLAGS
    "-UNDEBUG -Wno-error=sign-compare -Wno-error=format-pedantic")
else()
  set_source_files_properties(test.c PROPERTIES COMPILE_FLAGS
    "-UNDEBUG -Wno-error=sign-compare -Wno-error=format=")
endif()
add_executable(memstream_t test.c)
target_link_libraries(memstream_t memstream)
add_test(NAME memstream_t COMMAND memstream_t)

install(TARGETS memstream
  LIBRARY DESTINATION "lib"
  NAMELINK_SKIP
  )
install(FILES "memstream.h"
  DESTINATION "include/memstream"
)
