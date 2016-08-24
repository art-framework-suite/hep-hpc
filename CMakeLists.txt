cmake_minimum_required(VERSION 2.6)

project(hdffilestructurestudy)

set(HDFFILESTRUCTURESTUDY_VERSION 0.1.0)

set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules
  ${CMAKE_MODULE_PATH}
  )

# Ensure out-of-source build.
include(EnsureOutOfSourceBuild)
cet_ensure_out_of_source_build()

enable_testing()

# Set language standard for the C and CXX compilers.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

# Set compiler flags
set(common_flags "-Wall -Wextra -Werror -pedantic")
set(CMAKE_CXX_FLAGS "${common_flags}")
set(CMAKE_C_FLAGS "${common_flags}")
string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UC)
if (BUILD_TYPE_UC STREQUAL "RELEASE" OR
    BUILD_TYPE_UC STREQUAL "RELWITHDEBINFO")
  add_definitions("-DNDEBUG")
endif()
set(CMAKE_C_FLAGS_RELEASE "-O3 ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -fno-omit-frame-pointer ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -fno-omit-frame-pointer ${CMAKE_CXX_FLAGS}")

# Set include base directory.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Subdirectories.
add_subdirectory(hdffilestructurestudy)
#add_subdirectory(test)
#add_subdirectory(doc)
