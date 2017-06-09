configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hep_hpcConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/hep_hpcConfig.cmake
  @ONLY
  )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hep_hpcConfigVersion.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/hep_hpcConfigVersion.cmake
  @ONLY
  )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hep_hpcConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/hep_hpcConfigVersion.cmake
  DESTINATION "lib/hep_hpc/cmake"
)

if (WANT_UPS)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.in
    ${CMAKE_CURRENT_BINARY_DIR}/${UPS_VERSION_FILE}
    @ONLY
    )
  install(FILES hep_hpc.table DESTINATION "../ups")
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${UPS_VERSION_FILE}
    DESTINATION "../../${UPS_PROD_VERSION}.version"
    )
endif()
