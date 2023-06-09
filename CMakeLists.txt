# For CMAKE_C_EXTENSIONS support.
cmake_minimum_required(VERSION 3.16...3.26 FATAL_ERROR)

project(hep_hpc LANGUAGES C CXX)

set(HEP_HPC_VERSION 0.14.02)

set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules
  ${CMAKE_MODULE_PATH}
  )

# Ensure out-of-source build.
include(EnsureOutOfSourceBuild)
cet_ensure_out_of_source_build()

enable_testing()

# Set language standard for the C and CXX compilers.
set(CMAKE_CXX_STANDARD 17 CACHE STRING "ISO C++ language standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11 CACHE STRING "ISO C language standard")
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set compiler flags
add_compile_options(-Wall -Wextra -Werror -pedantic -Wno-missing-braces)
string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UC)
if (BUILD_TYPE_UC STREQUAL "RELEASE" OR
    BUILD_TYPE_UC STREQUAL "RELWITHDEBINFO")
  add_definitions("-DNDEBUG")
endif()
if ((CMAKE_CXX_COMPILER_ID STREQUAL GNU) AND
    (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0) AND
    (CMAKE_CXX_STANDARD LESS 17))
  string(APPEND CMAKE_CXX_FLAGS " -Wno-error=noexcept-type")
endif()
set(CMAKE_C_FLAGS_RELEASE "-O3 ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -fno-omit-frame-pointer ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -fno-omit-frame-pointer ${CMAKE_CXX_FLAGS}")
include(CheckSymbolExists)
set(CMAKE_REQUIRED_DEFINITIONS "-D_POSIX_C_SOURCE=200809L")
check_symbol_exists(open_memstream stdio.h HAS_OPEN_MEMSTREAM)
unset(CMAKE_REQUIRED_DEFINITIONS)

# Set include base directory.
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# Set output locations
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${EXECUTABLE_OUTPUT_PATH}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${LIBRARY_OUTPUT_PATH})

# MPI
if (WANT_MPI)
  find_package(MPI REQUIRED COMPONENTS C)
  if (MPI_FOUND)
    include_directories(${MPI_C_INCLUDE_PATH})
    set (HEP_HPC_USE_MPI TRUE)
  endif()
endif()

# HDF5
find_package(HDF5 REQUIRED COMPONENTS C)
include_directories(${HDF5_INCLUDE_DIRS})

# Configuration variables.
include(SetConfigVariables)
set_config_variables(${CMAKE_PROJECT_NAME} ${HEP_HPC_VERSION})

# Testing facilities
include (CTest)

# Subdirectories.
if (HAS_OPEN_MEMSTREAM)
  add_definitions("-D_POSIX_C_SOURCE=200809L")
else()
  add_subdirectory(memstream)
endif()
add_subdirectory(hep_hpc)
add_subdirectory(gtest EXCLUDE_FROM_ALL)
add_subdirectory(test)
add_subdirectory(ups)
add_subdirectory(examples)

install(FILES LICENSE README.md
  DESTINATION ".")
