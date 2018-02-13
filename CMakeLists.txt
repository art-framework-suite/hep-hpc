cmake_minimum_required(VERSION 2.6)

project(hep_hpc)

set(HEP_HPC_VERSION 0.06.03)

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
if (NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
if (NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 11)
endif()
set(CMAKE_C_STANDARD_REQUIRED TRUE)

if (CMAKE_VERSION VERSION_LESS 3.1)
  if (${CMAKE_CXX_STANDARD} EQUAL 17)
    set(cxx_standard -std=gnu++1z)
  else()
    set(cxx_standard -std=gnu++${CMAKE_CXX_STANDARD})
  endif()
  set(c_standard -std=gnu${CMAKE_C_STANDARD}) # For _POSIX_C_SOURCE >= 200809L
endif()

# Set compiler flags
set(common_flags "-Wall -Wextra -Werror -pedantic -Wno-missing-braces")
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
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# Set output locations
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${EXECUTABLE_OUTPUT_PATH}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${LIBRARY_OUTPUT_PATH})

# MPI
if (WANT_MPI)
  find_package(MPI REQUIRED)
  if (MPI_FOUND)
    include_directories(${MPI_C_INCLUDE_PATH})
    set (HEP_HPC_USE_MPI TRUE)
  endif()
endif()

# HDF5
find_package(HDF5 REQUIRED COMPONENTS C)
include_directories(${HDF5_INCLUDE_DIRS})

# BOOST (C++11 only, not needed for C++ >11)
if (CMAKE_CXX_STANDARD EQUAL 11)
  set(BOOST_INCLUDEDIR $ENV{BOOST_INC})
  set(BOOST_LIBRARYDIR $ENV{BOOST_LIB})
  find_package(Boost REQUIRED)
  message(STATUS "Found Boost version ${Boost_VERSION} headers at ${Boost_INCLUDE_DIRS}, libraries at ${Boost_LIBRARY_DIRS}.")
  set(HEP_HPC_USE_BOOST_INDEX_SEQUENCE TRUE)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# Configuration variables.
include(SetConfigVariables)
set_config_variables(${CMAKE_PROJECT_NAME} ${HEP_HPC_VERSION})

# Testing facilities
include (CTest)

# Subdirectories.
add_subdirectory(memstream)
add_subdirectory(hep_hpc)
add_subdirectory(gtest EXCLUDE_FROM_ALL)
add_subdirectory(test)
add_subdirectory(ups)
#add_subdirectory(doc)

install(FILES LICENSE README.md
  DESTINATION ".")
