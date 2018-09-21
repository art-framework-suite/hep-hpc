add_executable(make_concat_test_file make_concat_test_file.cpp)
target_link_libraries(make_concat_test_file hep_hpc_hdf5)

foreach(tname no-write-ranks no-read-ranks resize)
  add_executable(test-${tname} test-${tname}.c)
  target_link_libraries(test-${tname} ${HDF5_C_LIBRARIES} ${MPI_C_LIBRARIES})
endforeach()

find_program(MAKE_TEST_FILES NAME make_numerology_test_file.py
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}
  )
find_program(CONCAT_FILES NAME concat-hdf5.py HINTS ${CMAKE_SOURCE_DIR}/hep_hpc/concat_hdf5)
find_program(VERIFY_TEST_FILE NAME verify_test_file.py
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}
  )

if (NOT WANT_H5PY)
  message (WARNING "Define WANT_H5PY to enable tests of concat-hdf5.py and concat_hdf5 C++ application.")
  return()
endif()

include(CMakeParseArguments)

function(make_input_files TEST_FILES_IN_VAR OUTPUT_STEM)
  if (3.9.0 VERSION_GREATER CMAKE_VERSION)
    # Older CMake.
    set (SA_OPT UNIX_COMMAND)
  else()
    set (SA_OPT NATIVE_COMMAND)
  endif()
  cmake_parse_arguments(MIF "" "CHUNK_SIZE" "" ${ARGN})
  if (MIF_CHUNK_SIZE)
    set(MIF_CHUNK_SIZE "-c ${MIF_CHUNK_SIZE}")
  endif()
  execute_process(COMMAND ${MAKE_TEST_FILES}
    ${MIF_CHUNK_SIZE} -o "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_STEM}_in.hdf5"
    ${MIF_UNPARSED_ARGUMENTS}
    OUTPUT_VARIABLE TEST_FILES_IN
    RESULT_VARIABLE EC
    )
  if (EC)
    message(FATAL_ERROR "ERROR generating test data for target ${TARGET}")
  endif()
  separate_arguments(TEST_FILES_IN ${SA_OPT} "${TEST_FILES_IN}")
  set(${TEST_FILES_IN_VAR} ${TEST_FILES_IN} PARENT_SCOPE)
endfunction()

function(concat_numerology TARGET NRANKS)
  cmake_parse_arguments(TN "NO_PYTHON;NO_CPP" "CHUNK_SIZE" "" ${ARGN})
  set(TN_NROWS ${TN_UNPARSED_ARGUMENTS})
  list(REMOVE_ITEM ARGN NO_PYTHON NO_CPP)
  make_input_files(TEST_FILES_IN ${TARGET} ${ARGN})
  set(NROWS_TOTAL 0)
  foreach (nrows_in_file ${TN_NROWS})
    math(EXPR NROWS_TOTAL "${NROWS_TOTAL} + ${nrows_in_file}")
  endforeach()
  if (NOT TN_NO_PYTHON)
    add_test(NAME concat_numerology_${TARGET}_CONCAT_PY
      COMMAND ${MPIEXEC} -np ${NRANKS}
      ${CONCAT_FILES} -F -f -o ${TARGET}_py_out.hdf5 ${TEST_FILES_IN}
      )
    add_test(NAME concat_numerology_${TARGET}_VERIFY_PY
      COMMAND ${VERIFY_TEST_FILE} ${TARGET}_py_out.hdf5 ${NROWS_TOTAL}
      )
    set_property(TEST concat_numerology_${TARGET}_VERIFY_PY
      PROPERTY DEPENDS concat_numerology_${TARGET}_CONCAT_PY
      )
  endif()
  if (NOT TN_NO_CPP)
    add_test(NAME concat_numerology_${TARGET}_CONCAT_CPP
      COMMAND ${MPIEXEC} -np ${NRANKS}
      ${EXECUTABLE_OUTPUT_PATH}/concat_hdf5 -F -C
      -o ${TARGET}_cpp_out.hdf5 ${TEST_FILES_IN}
      )
    add_test(NAME concat_numerology_${TARGET}_VERIFY_CPP
      COMMAND ${VERIFY_TEST_FILE} ${TARGET}_cpp_out.hdf5 ${NROWS_TOTAL}
      )
    set_property(TEST concat_numerology_${TARGET}_VERIFY_CPP
      PROPERTY DEPENDS concat_numerology_${TARGET}_CONCAT_CPP
      )
  endif()
endfunction()

concat_numerology(two_rank_incomplete_chunk 2 CHUNK_SIZE 7 24 25)
concat_numerology(two_rank_simple 2 CHUNK_SIZE 7 49 56)
concat_numerology(two_rank_simple_dangling 2 CHUNK_SIZE 7 49 59)
concat_numerology(three_rank_incomplete 3 CHUNK_SIZE 7 24 25)
concat_numerology(three_rank_incomplete_dangling_A 3 CHUNK_SIZE 7 24 32)
concat_numerology(three_rank_incomplete_dangling_B 3 CHUNK_SIZE 7 24 39)
concat_numerology(three_rank_incomplete_dangling_C 3 CHUNK_SIZE 7 31 39)
concat_numerology(three_rank_incomplete_dangling_D 3 CHUNK_SIZE 7 38 39)
# As at 2.8.0, h5py is not capable of dealing with collective writes
# with one or more ranks writing no data. When this is no longer the
# case, remove the NO_PYTHON keyword below.
concat_numerology(four_rank_zero_write 4 CHUNK_SIZE 7 NO_PYTHON 21 3)
