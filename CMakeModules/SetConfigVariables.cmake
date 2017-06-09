function(set_config_variables UPS_PROD PROD_VERSION)
  ####################################
  # "Standard" variable settings for Config, ConfigVersion files.
  ####################################
  # UPS_PROD, UPS_PROD_UC.
  string(TOUPPER ${UPS_PROD} UPS_PROD_UC)
  set(UPS_PROD ${UPS_PROD} PARENT_SCOPE)
  set(UPS_PROD_UC ${UPS_PROD_UC} PARENT_SCOPE)
  # UPS_PROD_VERSION
  string(REPLACE "." "_" UPS_PROD_VERSION ${PROD_VERSION})
  set(UPS_PROD_VERSION v${UPS_PROD_VERSION})
  set(UPS_PROD_VERSION ${UPS_PROD_VERSION} PARENT_SCOPE)
  if (NOT WANT_UPS)
    # PACKAGE_PREFIX_ADJUSTMENT.
    set(PACKAGE_PREFIX_ADJUSTMENT ../../../)
    # INSTALLED_DIR_ADJUSTMENT.
    set(INSTALLED_DIR_ADJUSTMENT)
  else()
    ####################################
    # Settings only required if WANT_UPS is set.
    ####################################
    # UPS_FLAVOR.
    if (APPLE)
      set(FLAVOR_ARG -2)
    endif()
    execute_process(COMMAND ups flavor ${FLAVOR_ARG}
      OUTPUT_VARIABLE UPS_FLAVOR
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(UPS_FLAVOR ${UPS_FLAVOR} PARENT_SCOPE)
    # UPS_QUALS.
    if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
      if (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.9.3)
        set(eQual e10)
      elseif (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 6.3.0)
        set(eQual e14)
      else()
        message(FATAL_ERROR "WANT_UPS is incompatible with compiler version ${CMAKE_CXX_COMPILER_VERSION}")
      endif()
    else()
      message(FATAL_ERROR "WANT_UPS is incompatible with ${CMAKE_CXX_COMPILER_ID} compilers")
    endif()
    if (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
      set(bQual prof)
    elseif (CMAKE_BUILD_TYPE STREQUAL Debug)
      set(bQual debug)
    elseif (CMAKE_BUILD_TYPE STREQUAL Release)
      set(bQual opt)
    else()
      message("WANT_UPS is incompatible with build type ${CMAKE_BUILD_TYPE}")
    endif()
    if (WANT_MPI)
      if (ENV{MPICH_DIR})
        set(mQual :mpich)
      elseif (ENV{MVAPICH2_DIR})
        set(mQual :mvapich2)
      else()
        message("WANT_MPI set but could not find MPICH or MVAPICH2 UPS products")
      endif()
    endif()
    set(UPS_QUALS ${eQual}${mQual}:${bQual})
    set(UPS_QUALS ${UPS_QUALS} PARENT_SCOPE)
    # UPS_DATE.
    execute_process(COMMAND date
      OUTPUT_VARIABLE UPS_DATE
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    set(UPS_DATE ${UPS_DATE} PARENT_SCOPE)
    # UPS_VERSION_FILE.
    string(REPLACE ":" "_" UPS_VERSION_FILE ${UPS_QUALS})
    set(UPS_VERSION_FILE "${UPS_FLAVOR}_${UPS_VERSION_FILE}")
    set(UPS_VERSION_FILE ${UPS_VERSION_FILE} PARENT_SCOPE)
    # UPS_FLAVOR_DIR.
    string(REPLACE ":" "-" UPS_FLAVOR_DIR ${UPS_QUALS})
    set(UPS_FLAVOR_DIR "${UPS_FLAVOR}-${UPS_FLAVOR_DIR}")
    set(UPS_FLAVOR_DIR ${UPS_FLAVOR_DIR} PARENT_SCOPE)
    # CMAKE_INSTALL_PREFIX.
    set(CMAKE_INSTALL_PREFIX 
      "${CMAKE_INSTALL_PREFIX}/${UPS_PROD}/${UPS_PROD_VERSION}/${UPS_FLAVOR_DIR}"
      PARENT_SCOPE)
    # PACKAGE_PREDIX_ADJUSTMENT.
    set(PACKAGE_PREFIX_ADJUSTMENT ../../../../../../)
    # INSTALLED_DIR_ADJUSTMENT.
    set(INSTALLED_DIR_ADJUSTMENT @UPS_PROD@/@UPS_PROD_VERSION@/@UPS_FLAVOR_DIR@/)
  endif()
endfunction()
