enable_testing()

# Make sure that asserts are evaluated in test.c, and we are permissive
# about known warnings.
set(CMAKE_C_EXTENSIONS ON) # Need GNU11 rather than C11.
if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  set_source_files_properties(test.c PROPERTIES COMPILE_FLAGS
    "-UNDEBUG -Wno-error=sign-compare -Wno-error=format-pedantic")
else()
  set_source_files_properties(test.c PROPERTIES COMPILE_FLAGS
    "-UNDEBUG -Wno-error=sign-compare -Wno-error=format=")
endif()

add_library(memstream SHARED memstream.c)
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
