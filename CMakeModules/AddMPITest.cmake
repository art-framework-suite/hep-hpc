########################################################################
# add_mpi_test(TARGET_STEM [NP #] [WRAP_COMMAND <wrapper-cmd-and-args>]
#              [COMMAND] <cmd-and-args>)
#
# Add a test to be run under the MPI launcher.
#
########################################################################
# Options
#
# NP <#>
#
#   Specify the number of processors to use. Defaults to NCORES if not
#   specified.
#
# WRAP_COMMAND <wrapper-cmd-and-args>
#
#   Pass the composed MPI launch command and its arguments to the
#   specified wrapper.
#
# [COMMAND] <cmd-and-args>
#
#   The command and its arguments to run under the MPI launcher. The
#   COMMAND keyword is optional unless necessary to disambiguate from
#   WRAP_COMMAND.
#
########################################################################
# Notes
#
# Additionally, this module defines NCORES to be the number of cores
# found on the system.
########################################################################
include (CMakeParseArguments)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  execute_process(COMMAND sysctl -n hw.ncpu
    OUTPUT_VARIABLE NCORES
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  execute_process(COMMAND cat /proc/cpuinfo
    COMMAND grep -c -e ^processor
    OUTPUT_VARIABLE NCORES
    ERROR_QUIET
    )
endif()

if (NOT NCORES)
  message(FATAL_ERROR "Unable to ascertain number of cores on this system.")
endif()

function (add_mpi_test TARGET_STEM)
  if (NOT MPI_FOUND)
    message(FATAL_ERROR "add_mpi_test requires that find_package(MPI) has been executed successfully on this system.")
  endif()

  cmake_parse_arguments(amt "VALGRIND" "NP" "WRAP_COMMAND;COMMAND" ${ARGN})
  if (NOT amt_NP)
    set(amt_NP ${NCORES})
  endif()
  if (NOT amt_COMMAND)
    set (amt_COMMAND ${amt_UNPARSED_ARGUMENTS})
  endif()
  add_test(NAME ${TARGET_STEM}_${amt_NP} COMMAND
    ${amt_WRAP_COMMAND} ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${amt_NP}
    ${MPIEXEC_PREFLAGS} ${MPIEXEC_POSTFLAGS}
    ${amt_COMMAND}
    )
  if (amt_VALGRIND)
    add_test(NAME ${TARGET_STEM}_${amt_NP}_MEMCHECK COMMAND
      ${amt_WRAP_COMMAND} ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${amt_NP}
      ${MPIEXEC_PREFLAGS} ${MPIEXEC_POSTFLAGS}
      ${MEMORYCHECK_COMMAND} --error-exitcode=1 --leak-check=no ${amt_COMMAND}
      )
  endif()

  set_tests_properties(${TARGET_STEM}_${amt_NP} PROPERTIES PROCESSORS ${amt_NP})
endfunction()
