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
    set(PACKAGE_PREFIX_ADJUSTMENT ../../../ PARENT_SCOPE)
    # INSTALLED_DIR_ADJUSTMENT.
    set(INSTALLED_DIR_ADJUSTMENT PARENT_SCOPE)
  else()
    ####################################
    # Settings only required if WANT_UPS is set.
    ####################################
    # UPS_FLAVOR.
    if (APPLE)
      set(FLAVOR_ARG -2)
    else()
      set(FLAVOR_ARG -4)
    endif()
    execute_process(COMMAND ups flavor ${FLAVOR_ARG}
      OUTPUT_VARIABLE UPS_FLAVOR
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(UPS_FLAVOR ${UPS_FLAVOR} PARENT_SCOPE)
    # UPS_QUALS.
    if (DEFINED ENV{SETUP_HDF5})
      string(REGEX MATCH "-q[ \\t]+(.*)$" quals "$ENV{SETUP_HDF5}")
      string(REPLACE ":" ";" quals "${CMAKE_MATCH_1}")
      foreach (qual ${quals})
        if (qual MATCHES "^[eic][0-9]+$")
          set(eQual "${qual}")
        endif()
      endforeach()
      if (NOT eQual)
        message(FATAL_ERROR "Unable to set eQual from HDF5 qualifier set ${quals}")
      endif()
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
      if (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.9.3)
        set(eQual e10)
      elseif (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 6.3.0)
        set(eQual e14)
      elseif (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 6.4.0)
        set(eQual e15)
      elseif (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 7.2.0)
        set(eQual e16)
      elseif (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 7.3.0)
        set(eQual e17)
      else()
        message(FATAL_ERROR "WANT_UPS is incompatible with ${CMAKE_CXX_COMPILER_ID} version ${CMAKE_CXX_COMPILER_VERSION}")
      endif()
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
      if (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 5.0.1)
        set(eQual c2)
      else()
        message(FATAL_ERROR "WANT_UPS is incompatible with ${CMAKE_CXX_COMPILER_ID} version ${CMAKE_CXX_COMPILER_VERSION}")
      endif()
    else()
      message(FATAL_ERROR "WANT_UPS is incompatible with ${CMAKE_CXX_COMPILER_ID} compilers")
    endif()
    message(STATUS "Assigned UPS base qualifier ${eQual}")
    if (WANT_MPI)
      if (DEFINED ENV{MPICH_DIR})
        set(mQual :mpich)
      elseif (DEFINED ENV{MVAPICH2_DIR})
        set(mQual :mvapich2)
      else()
        message("WANT_MPI set but could not find MPICH or MVAPICH2 UPS products")
      endif()
    endif()
    set(UPS_QUALS ${eQual}${mQual})
    set(UPS_QUALS ${UPS_QUALS} PARENT_SCOPE)
    # UPS_DATE.
    execute_process(COMMAND date
      OUTPUT_VARIABLE UPS_DATE
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    set(UPS_DATE ${UPS_DATE} PARENT_SCOPE)
    # UPS_VERSION_FILE.
    # Qualifiers in version file name must be sorted!
    string(REPLACE ":" ";" UPS_QUALS_LIST ${UPS_QUALS})
    list(SORT UPS_QUALS_LIST)
    string(REPLACE ";" "_" UPS_VERSION_FILE "${UPS_QUALS_LIST}")
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
    # PACKAGE_PREFIX_ADJUSTMENT.
    set(PACKAGE_PREFIX_ADJUSTMENT ../../../../../../ PARENT_SCOPE)
    # INSTALLED_DIR_ADJUSTMENT.
    set(INSTALLED_DIR_ADJUSTMENT ${UPS_PROD}/${UPS_PROD_VERSION}/${UPS_FLAVOR_DIR}/ PARENT_SCOPE)
  endif()
endfunction()
