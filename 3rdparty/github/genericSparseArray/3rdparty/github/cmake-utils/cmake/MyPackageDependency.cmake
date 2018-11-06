MACRO (MYPACKAGEDEPENDENCY packageDepend packageDependSourceDir)
  #
  # Optional argument: TESTS, LIBS, EXES, SYSTEM, LOCAL (forces ALL_IN_ONE for this dependency), STATIC
  #
  IF (ALL_IN_ONE)
    SET (_ALL_IN_ONE TRUE)
  ELSE ()
    SET (_ALL_IN_ONE FALSE)
  ENDIF ()
  SET (_ALL TRUE)
  SET (_TESTS FALSE)
  SET (_LIBS FALSE)
  SET (_EXES FALSE)
  SET (_STATIC FALSE)
  FOREACH (_var ${ARGN})
    IF (${_var} STREQUAL TESTS)
      IF (MYPACKAGE_DEBUG)
        MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] ${packageDepend} test scope argument")
      ENDIF ()
      SET (_ALL FALSE)
      SET (_TESTS TRUE)
    ENDIF ()
    IF (${_var} STREQUAL LIBS)
      IF (MYPACKAGE_DEBUG)
        MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] ${packageDepend} library scope argument, implies test")
      ENDIF ()
      SET (_ALL FALSE)
      SET (_LIBS TRUE)
      SET (_TESTS TRUE)
    ENDIF ()
    IF (${_var} STREQUAL EXES)
      IF (MYPACKAGE_DEBUG)
        MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] ${packageDepend} executables scope argument")
      ENDIF ()
      SET (_ALL FALSE)
      SET (_EXES TRUE)
    ENDIF ()
    IF (${_var} STREQUAL SYSTEM)
      IF (MYPACKAGE_DEBUG)
        MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] ${packageDepend} system search")
      ENDIF ()
      SET (_ALL_IN_ONE FALSE)
    ENDIF ()
    IF (${_var} STREQUAL LOCAL)
      IF (MYPACKAGE_DEBUG)
        MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] ${packageDepend} local mode")
      ENDIF ()
      SET (_ALL_IN_ONE TRUE)
    ENDIF ()
    IF (${_var} STREQUAL STATIC)
      IF (MYPACKAGE_DEBUG)
        MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] ${packageDepend} static mode")
      ENDIF ()
      SET (_STATIC TRUE)
    ENDIF ()
  ENDFOREACH ()
  IF (_ALL)
    SET (_TESTS TRUE)
    SET (_LIBS TRUE)
    SET (_EXES TRUE)
  ENDIF ()
  IF (MYPACKAGE_DEBUG)
    MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] ${packageDepend} dependency scopes are: ALL=${_ALL} TEST=${_TESTS} LIBS=${_LIBS} EXES=${_EXES}")
  ENDIF ()
  #
  # Set dependency scope: PUBLIC or PRIVATE depending on _STATIC
  #
  IF (_STATIC)
    SET (_package_dependency_scope "PRIVATE")
  ELSE ()
    SET (_package_dependency_scope "PUBLIC")
  ENDIF ()
  #
  # Check if inclusion was already done - via us or another mechanism... guessed with TARGET check
  #
  GET_PROPERTY(_packageDepend_set GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend} SET)
  IF (${_packageDepend_set})
    GET_PROPERTY(_packageDepend GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend})
  ELSE ()
    SET (_packageDepend "")
  ENDIF ()
  IF ((NOT ("${_packageDepend}" STREQUAL "")) OR (TARGET ${packageDepend}))
    IF (${_packageDepend_set})
      IF (${_packageDepend} STREQUAL "PENDING")
        IF (MYPACKAGE_DEBUG)
          MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-STATUS] ${packageDepend} is already being processed")
        ENDIF ()
      ELSE ()
        IF (${_packageDepend} STREQUAL "DONE")
          IF (MYPACKAGE_DEBUG)
            MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-STATUS] ${packageDepend} is already available")
          ENDIF ()
        ELSE ()
          MESSAGE (FATAL_ERROR "[${PROJECT_NAME}-DEPEND-STATUS] ${packageDepend} state is ${_packageDepend}, should be DONE or PENDING")
        ENDIF ()
      ENDIF ()
    ELSE ()
      # MESSAGE (WARNING "[${PROJECT_NAME}-DEPEND-WARNING] Target ${packageDepend} already exist - use MyPackageDependency to avoid this warning")
      IF (MYPACKAGE_DEBUG)
        MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Setting property MYPACKAGE_DEPENDENCY_${packageDepend} to DONE")
      ENDIF ()
      SET_PROPERTY(GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend} "DONE")
    ENDIF ()
  ELSE ()
    IF (MYPACKAGE_DEBUG)
      MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-STATUS] ${packageDepend} is not yet available")
      MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Setting property MYPACKAGE_DEPENDENCY_${packageDepend} to PENDING")
    ENDIF ()
    SET_PROPERTY(GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend} "PENDING")
    #
    # ===================================================
    # Do the dependency: ADD_SUBDIRECTORY or FIND_PACKAGE
    # ===================================================
    #
    STRING (TOUPPER ${packageDepend} _PACKAGEDEPEND)
    IF (_ALL_IN_ONE)
      GET_FILENAME_COMPONENT(packageDependSourceDirAbsolute ${packageDependSourceDir} ABSOLUTE)
      MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-STATUS] Adding subdirectory ${packageDependSourceDirAbsolute}")
      IF (_STATIC)
        ADD_SUBDIRECTORY(${packageDependSourceDirAbsolute} EXCLUDE_FROM_ALL)
      ELSE ()
        ADD_SUBDIRECTORY(${packageDependSourceDirAbsolute})
      ENDIF ()
    ELSE ()
      MESSAGE(STATUS "[${PROJECT_NAME}-DEPEND-STATUS] Looking for ${packageDepend}")
      FIND_PACKAGE (${packageDepend})
      IF (NOT ${_PACKAGEDEPEND}_FOUND)
        MESSAGE (FATAL_ERROR "[${PROJECT_NAME}-DEPEND-STATUS] Find ${packageDepend} failed")
      ENDIF ()
    ENDIF ()
    IF (MYPACKAGE_DEBUG)
      MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Setting property MYPACKAGE_DEPENDENCY_${packageDepend} to DONE")
    ENDIF ()
    SET_PROPERTY(GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend} "DONE")
    #
    # Remember all eventual packageDepend variables we depend upon
    #
    FOREACH (_what "INCLUDE_DIRS" "LIBRARIES" "C_FLAGS_SHARED" "LINK_FLAGS")
      IF (MYPACKAGE_DEBUG)
        MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Setting property MYPACKAGE_DEPENDENCY_${packageDepend}_${_what} to ${${_PACKAGEDEPEND}_${_what}}")
      ENDIF ()
      SET_PROPERTY(GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend}_${_what} ${${_PACKAGEDEPEND}_${_what}})
    ENDFOREACH ()
  ENDIF ()

  #
  # Manage dependencies
  #
  SET (_test_candidates ${${PROJECT_NAME}_TEST_EXECUTABLE})
  SET (_lib_candidates  ${PROJECT_NAME} ${PROJECT_NAME}_static)
  SET (_exe_candidates  ${${PROJECT_NAME}_EXECUTABLE})
  SET (_candidates)
  IF (_TESTS)
    LIST (APPEND _candidates ${_test_candidates})
  ENDIF ()
  IF (_LIBS)
    LIST (APPEND _candidates ${_lib_candidates})
  ENDIF ()
  IF (_EXES)
    LIST (APPEND _candidates ${_exe_candidates})
  ENDIF ()
  #
  # Loop on current project's target candidates
  #
  FOREACH (_target ${_candidates})
    IF (MYPACKAGE_DEBUG)
      MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Inspecting target candidate ${_target}")
    ENDIF ()
    IF (TARGET ${_target})
      #
      # Static mode ?
      #
      IF (_STATIC)
        IF (MYPACKAGE_DEBUG)
          MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Adding -D${packageDepend}_STATIC compile definition to target ${_target}")
        ENDIF ()
        TARGET_COMPILE_DEFINITIONS(${_target} ${_package_dependency_scope} -D${packageDepend}_STATIC)
        IF (MYPACKAGE_DEBUG)
          MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] ${packageDepend} dependency becomes ${packageDepend}_static for target ${_target}")
        ENDIF ()
        SET (realPackageDepend ${packageDepend}_static)
      ELSE ()
        SET (realPackageDepend ${packageDepend})
      ENDIF ()
      IF (_ALL_IN_ONE)
        #
        # Dependency by target
        #
        IF (TARGET ${realPackageDepend})
          IF (MYPACKAGE_DEBUG)
            MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Adding target ${realPackageDepend} dependency to target ${_target}")
          ENDIF ()
          TARGET_LINK_LIBRARIES(${_target} ${_package_dependency_scope} ${realPackageDepend})
          IF (_TESTS)
            #
            # A bit painful but the target locations are not known at this time.
            # We remember all targets for later use in the check generation command.
            #
            GET_PROPERTY(_targets_for_test_set GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${PROJECT_NAME}_TARGETS_FOR_TEST)
            IF (NOT _targets_for_test_set)
              SET (_targets_for_test ${realPackageDepend})
              IF (MYPACKAGE_DEBUG)
                MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-STATUS] Initialized MYPACKAGE_DEPENDENCY_${PROJECT_NAME}_TARGETS_FOR_TEST with ${realPackageDepend}")
              ENDIF ()
              SET_PROPERTY(GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${PROJECT_NAME}_TARGETS_FOR_TEST ${_targets_for_test})
            ELSE ()
              LIST (FIND _targets_for_test ${realPackageDepend} _targets_for_test_found)
              IF (${_targets_for_test_found} EQUAL -1)
                LIST (APPEND _targets_for_test ${realPackageDepend})
                IF (MYPACKAGE_DEBUG)
                  MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-STATUS] Appended MYPACKAGE_DEPENDENCY_${PROJECT_NAME}_TARGETS_FOR_TEST with ${realPackageDepend}")
                ENDIF ()
                SET_PROPERTY(GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${PROJECT_NAME}_TARGETS_FOR_TEST ${_targets_for_test})
              ENDIF ()
            ENDIF ()
          ENDIF ()
        ELSE ()
          #
          # Bad luck, this target does not generate a library
          # We use global properties.
          #
          GET_PROPERTY(_packageDepend_fake_include_dirs_set GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend}_FAKE_INCLUDE_DIRS SET)
          IF (${_packageDepend_fake_include_dirs_set})
            GET_PROPERTY(_packageDepend_fake_include_dirs GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend}_FAKE_INCLUDE_DIRS)
          ELSE ()
            SET (_packageDepend_fake_include_dirs)
            FOREACH (_include_directory ${packageDependSourceDir}/output/include ${packageDependSourceDir}/include)
              GET_FILENAME_COMPONENT(_absolute_include_directory ${_include_directory} ABSOLUTE)
              LIST (APPEND _packageDepend_fake_include_dirs ${_absolute_include_directory})
            ENDFOREACH ()
            SET_PROPERTY(GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend}_FAKE_INCLUDE_DIRS ${_packageDepend_fake_include_dirs})
            IF (MYPACKAGE_DEBUG)
              MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] MYPACKAGE_DEPENDENCY_${packageDepend}_FAKE_INCLUDE_DIRS initialized to ${_packageDepend_fake_include_dirs}")
              MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Use MyPackageStart to avoid this fallback")
            ENDIF ()
          ENDIF ()
          #
          # Apply ${_packageDepend_fake_include_dirs}
          #
          FOREACH (_include_directory ${_packageDepend_fake_include_dirs})
            IF (MYPACKAGE_DEBUG)
              MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Adding ${_include_directory} directory to ${_target}'s include directories")
            ENDIF ()
            TARGET_INCLUDE_DIRECTORIES(${_target} ${_package_dependency_scope} ${_include_directory})
            #
            # Add eventually this to our project's default include directories
            #
            LIST (FIND _project_fake_include_dirs ${_include_directory} _index)
            IF (${_index} EQUAL -1)
              IF (MYPACKAGE_DEBUG)
                MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Adding ${_include_directory} directory to property MYPACKAGE_DEPENDENCY_${PROJECT_NAME}_FAKE_INCLUDE_DIRS")
              ENDIF ()
              LIST (APPEND _project_fake_include_dirs ${_include_directory})
              SET_PROPERTY(GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${PROJECT_NAME}_FAKE_INCLUDE_DIRS ${_project_fake_include_dirs})
            ENDIF ()
          ENDFOREACH ()
        ENDIF ()
      ELSE ()
        #
        # Include dependency
        #
        GET_PROPERTY(_property GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend}_INCLUDE_DIRS)
        FOREACH (_include_directory ${_property})
          IF (MYPACKAGE_DEBUG)
            MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Adding ${_include_directory} directory to ${_target}'s include directories")
          ENDIF ()
          TARGET_INCLUDE_DIRECTORIES(${_target} ${_package_dependency_scope} ${_include_directory})
        ENDFOREACH ()
        #
        # Library dependency
        #
        GET_PROPERTY(_property GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend}_LIBRARIES)
        FOREACH (_library ${_property})
          IF (MYPACKAGE_DEBUG)
            MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Adding ${_library} library to ${_target}")
          ENDIF ()
          TARGET_LINK_LIBRARIES(${_target} ${_package_dependency_scope} ${_library})
        ENDFOREACH ()
        #
        # Compile definitions
        #
        GET_PROPERTY(_property GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend}_C_FLAGS_SHARED)
        FOREACH (_flag ${_property})
          IF (MYPACKAGE_DEBUG)
            MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Adding ${_flag} compile flag to ${_target}")
          ENDIF ()
          TARGET_COMPILE_DEFINITIONS(${_target} ${_package_dependency_scope} ${_library})
        ENDFOREACH ()
        #
        # Link flags
        #
        GET_PROPERTY(_property GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend}_LINK_FLAGS)
        FOREACH (_flag ${_property})
          IF (MYPACKAGE_DEBUG)
            MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-DEBUG] Adding ${_flag} link flag to ${_target}")
          ENDIF ()
          SET_TARGET_PROPERTIES(${_target}
            PROPERTIES
            LINK_FLAGS ${_flag}
            )
        ENDFOREACH ()
      ENDIF ()
    ENDIF ()
  ENDFOREACH ()
  #
  # Test path management
  #
  GET_PROPERTY(_test_path_set GLOBAL PROPERTY MYPACKAGE_TEST_PATH SET)
  IF (${_test_path_set})
    GET_PROPERTY(_test_path GLOBAL PROPERTY MYPACKAGE_TEST_PATH)
  ELSE ()
    SET (_test_path $ENV{PATH})
    IF ("${CMAKE_HOST_SYSTEM}" MATCHES ".*Windows.*")
      STRING(REGEX REPLACE "/" "\\\\"  _test_path "${_test_path}")
    ELSE ()
      STRING(REGEX REPLACE " " "\\\\ "  _test_path "${_test_path}")
    ENDIF ()
    IF (MYPACKAGE_DEBUG)
      MESSAGE(STATUS "[${PROJECT_NAME}-DEPEND-STATUS] Initializing TEST_PATH with PATH")
    ENDIF ()
    SET_PROPERTY(GLOBAL PROPERTY MYPACKAGE_TEST_PATH ${_test_path})
  ENDIF ()

  GET_PROPERTY(_dependLibraryRuntimeDirectories GLOBAL PROPERTY MYPACKAGE_DEPENDENCY_${packageDepend}_LIBRARIES)
  #
  # On Windows we want to make sure it contains a bin in the last component
  #
  SET (_have_bin FALSE)
  IF ("${CMAKE_HOST_SYSTEM}" MATCHES ".*Windows.*")
    FOREACH (_dir ${_dependLibraryRuntimeDirectories})
      GET_FILENAME_COMPONENT(_lastdir ${_dir} NAME)
      STRING (TOUPPER ${_lastdir} _lastdir)
      IF ("${_lastdir}" STREQUAL "BIN")
        SET (_have_bin TRUE)
        BREAK ()
      ENDIF ()
    ENDFOREACH ()
    IF (NOT _have_bin)
      SET (_dependLibraryRuntimeDirectoriesOld ${_dependLibraryRuntimeDirectories})
      FOREACH (_dir ${_dependLibraryRuntimeDirectoriesOld})
        GET_FILENAME_COMPONENT(_updir ${_dir} DIRECTORY)
        SET (_bindir "${_updir}/bin")
        IF (EXISTS "${_bindir}")
          LIST (APPEND _dependLibraryRuntimeDirectories "${_bindir}")
        ENDIF ()
      ENDFOREACH ()
    ENDIF ()
  ENDIF ()
  IF (NOT ("${_dependLibraryRuntimeDirectories}" STREQUAL ""))
    IF ("${CMAKE_HOST_SYSTEM}" MATCHES ".*Windows.*")
      SET (SEP "\\;")
    ELSE ()
      SET (SEP ":")
    ENDIF ()
    FOREACH (_dir ${_dependLibraryRuntimeDirectories})
      IF ("${CMAKE_HOST_SYSTEM}" MATCHES ".*Windows.*")
        STRING(REGEX REPLACE "/" "\\\\"  _dir "${_dir}")
      ELSE ()
        STRING(REGEX REPLACE " " "\\\\ "  _dir "${_dir}")
      ENDIF ()
      SET (_test_path "${_dir}${SEP}${_test_path}")
      IF (MYPACKAGE_DEBUG)
        MESSAGE (STATUS "[${PROJECT_NAME}-DEPEND-STATUS] Prepended ${_dir} to TEST_PATH")
      ENDIF ()
      SET_PROPERTY(GLOBAL PROPERTY MYPACKAGE_TEST_PATH ${_test_path})
    ENDFOREACH ()
  ENDIF ()
  SET (TEST_PATH ${_test_path} CACHE INTERNAL "Test Path" FORCE)
ENDMACRO()
