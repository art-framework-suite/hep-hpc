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
else()
  message(FATAL_ERROR "Unable to ascertain number of cores on this system.")
endif()

function (add_mpi_test TARGET_STEM)
  cmake_parse_arguments(amt "" "NP" "WRAP_COMMAND;COMMAND" ${ARGN})
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
  set_tests_properties(${TARGET_STEM}_${amt_NP} PROPERTIES PROCESSORS ${amt_NP})
endfunction()
