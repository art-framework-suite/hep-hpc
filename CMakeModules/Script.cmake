include(CMakeParseArguments)

function (script)
  cmake_parse_arguments(script "" "DEPENDENCIES" "" ${ARGN})
  foreach (source ${script_UNPARSED_ARGUMENTS})
    get_filename_component(source_base "${source}" NAME)
    set(dest "${EXECUTABLE_OUTPUT_PATH}/${source_base}")
    add_custom_command(OUTPUT "${dest}"
      COMMAND ${CMAKE_COMMAND} -E copy "${source}" "${EXECUTABLE_OUTPUT_PATH}"
      COMMAND chmod +x "${dest}"
      COMMENT "Copying ${source} to ${EXECUTABLE_OUTPUT_PATH}"
      DEPENDS "${source}" ${script_DEPENDENCIES}
      )
    add_custom_target(${source_base} ALL DEPENDS "${dest}")
  endforeach()
endfunction()
