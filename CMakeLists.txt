cmake_minimum_required(VERSION 2.6)

project(hdffilestructurestudy)

set(HDFFILESTRUCTURESTUDY_VERSION 0.1.0)

if (POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW) # @rpath on OS X.
endif()

set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules
  ${CMAKE_MODULE_PATH}
  )


# Ensure out-of-source build.
include(EnsureOutOfSourceBuild)
cet_ensure_out_of_source_build()

enable_testing()

# Set language standard for the C and CXX compilers.
if (NOT CMAKE_VERSION VERSION_LESS 3.1)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_C_STANDARD_REQUIRED TRUE)
else()
  set(cxx_standard -std=c++14)
  set(c_standard -std=gnu11) # For _POSIX_C_SOURCE >= 200809L
endif()

# Set compiler flags
set(common_flags "-Wall -Wextra -Werror -pedantic")
set(CMAKE_CXX_FLAGS "${common_flags} ${cxx_standard}")
set(CMAKE_C_FLAGS "${common_flags} ${c_standard}")
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

# Set output locations
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${EXECUTABLE_OUTPUT_PATH}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${LIBRARY_OUTPUT_PATH})

# MPI
if (WANT_MPI) # Set at CMake invocation time.
  find_package(MPI QUIET)
endif()

if (MPI_FOUND)
  include_directories(${MPI_C_INCLUDE_PATH})
endif()

# HDF5
find_package(HDF5 COMPONENTS C)
if (NOT HDF5_FOUND)
  message(FATAL_ERROR "Unable to find a suitable HDF5 distribution")
endif()
include_directories(${HDF5_INCLUDE_DIRS})

include (CTest)

# Subdirectories.
add_subdirectory(memstream)
add_subdirectory(hep_hpc)
add_subdirectory(gtest EXCLUDE_FROM_ALL)
add_subdirectory(test)
#add_subdirectory(doc)

install(FILES LICENSE README.md
  DESTINATION ".")
