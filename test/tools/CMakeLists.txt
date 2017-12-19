set(TOOLS_DIR ${CMAKE_SOURCE_DIR}/tools)

find_program(MAKE_TEST_FILES NAME make_numerology_test_file.py
  HINTS ${TOOLS_DIR}
  )
find_program(CONCAT_FILES NAME concat-hdf5.py HINTS ${TOOLS_DIR})
find_program(VERIFY_TEST_FILE NAME verify_test_file.py HINTS ${TOOLS_DIR})

include(CMakeParseArguments)

function(concat_numerology TARGET NRANKS)
  cmake_parse_arguments(TN "" "CHUNK_SIZE" "" ${ARGN})
  set(TN_NROWS ${TN_UNPARSED_ARGUMENTS})
  if (TN_CHUNK_SIZE)
    set(TN_CHUNK_SIZE "-c ${TN_CHUNK_SIZE}")
  endif()
  execute_process(COMMAND ${MAKE_TEST_FILES}
    ${TN_CHUNK_SIZE} -o "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_in.hdf5"
    ${TN_NROWS}
    OUTPUT_VARIABLE TEST_FILES_IN
    RESULT_VARIABLE EC
    )
  separate_arguments(TEST_FILES_IN NATIVE_COMMAND "${TEST_FILES_IN}")
  if (EC)
    message(FATAL_ERROR "ERROR generating test data for target ${TARGET}")
  endif()
  add_test(NAME concat_numerology_${TARGET}_CONCAT_PREP
    COMMAND rm -f ${TARGET}_out.hdf5
    )
  add_test(NAME concat_numerology_${TARGET}_CONCAT
    COMMAND ${MPIEXEC} -np ${NRANKS}
    ${CONCAT_FILES} -F -o ${TARGET}_out.hdf5 ${TEST_FILES_IN}
    )
  set_property(TEST concat_numerology_${TARGET}_CONCAT
    PROPERTY DEPENDS concat_numerology_${TARGET}_CONCAT_PREP
    )
  set(NROWS_TOTAL 0)
  foreach (nrows_in_file ${TN_NROWS})
    math(EXPR NROWS_TOTAL "${NROWS_TOTAL} + ${nrows_in_file}")
  endforeach()
  add_test(NAME concat_numerology_${TARGET}_VERIFY
    COMMAND ${VERIFY_TEST_FILE} ${TARGET}_out.hdf5 ${NROWS_TOTAL}
    )
  set_property(TEST concat_numerology_${TARGET}_VERIFY
    PROPERTY DEPENDS concat_numerology_${TARGET}_CONCAT
    )
endfunction()

concat_numerology(two_rank_incomplete_chunk 2 CHUNK_SIZE 7 24 25)
concat_numerology(two_rank_simple 2 CHUNK_SIZE 7 49 56)
concat_numerology(two_rank_simple_dangling 2 CHUNK_SIZE 7 49 59)
concat_numerology(three_rank_incomplete 3 CHUNK_SIZE 7 24 25)
concat_numerology(three_rank_incomplete_dangling_A 3 CHUNK_SIZE 7 24 32)
concat_numerology(three_rank_incomplete_dangling_B 3 CHUNK_SIZE 7 24 39)
concat_numerology(three_rank_incomplete_dangling_C 3 CHUNK_SIZE 7 31 39)
concat_numerology(three_rank_incomplete_dangling_D 3 CHUNK_SIZE 7 38 39)
