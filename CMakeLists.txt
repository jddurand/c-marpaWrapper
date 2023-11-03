CMAKE_MINIMUM_REQUIRED (VERSION 3.26.0 FATAL_ERROR)
PROJECT(marpaWrapper VERSION 1.0.103 LANGUAGES C)
IF (NOT MYPACKAGEBOOTSTRAP_DONE)
  INCLUDE ("3rdparty/github/cmake-utils/cmake/MyPackageBootstrap.cmake")
ENDIF ()
MYPACKAGESTART ()

##############################################################################################
# For performance reason (compiler optimization of unit) we want everything in a single file #
##############################################################################################

MYPACKAGELIBRARY(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in
  ${INCLUDE_OUTPUT_PATH}/config.h
  amalgamation/marpaWrapper.c)
#
# Because of Marpa, the cmake framework does not know about it
#
FOREACH (_target ${PROJECT_NAME}_shared ${PROJECT_NAME}_static)
  TARGET_INCLUDE_DIRECTORIES(${_target} PRIVATE libmarpa/work/stage)
  FOREACH (_type MAJOR MICRO MINOR)
    TARGET_COMPILE_DEFINITIONS(${_target} PRIVATE -DMARPA_LIB_${_type}_VERSION=MARPA_${_type}_VERSION)
  ENDFOREACH ()
ENDFOREACH ()

################
# Dependencies #
################
MYPACKAGEDEPENDENCY (genericStack       "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/github/genericStack"       LOCAL IFACE  PUBLIC  IFACE_OBJECTS) # present in public API
MYPACKAGEDEPENDENCY (genericLogger      "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/github/genericLogger"      LOCAL LIBS   PUBLIC  SHARED_LIB)    # present in public API
MYPACKAGEDEPENDENCY (genericHash        "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/github/genericHash"        LOCAL LIBS   PRIVATE IFACE_OBJECTS) # not present in public API
MYPACKAGEDEPENDENCY (genericSparseArray "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/github/genericSparseArray" LOCAL LIBS   PRIVATE IFACE_OBJECTS) # not present in public API

###############
# Executables #
###############
MYPACKAGETESTEXECUTABLE(marpaWrapperTester             test/marpaWrapperTester.c)
MYPACKAGETESTEXECUTABLE(marpaWrapperAsfTester          test/marpaWrapperAsfTester.c)
MYPACKAGETESTEXECUTABLE(marpaWrapperAsfValueTester     test/marpaWrapperAsfValueTester.c)
MYPACKAGETESTEXECUTABLE(marpaWrapperAsfAmbiguousTester test/marpaWrapperAsfAmbiguousTester.c)

#########
# Tests #
#########
MYPACKAGECHECK(marpaWrapperTester)
MYPACKAGECHECK(marpaWrapperAsfTester)
MYPACKAGECHECK(marpaWrapperAsfValueTester)
MYPACKAGECHECK(marpaWrapperAsfAmbiguousTester)

##########
# Export #
##########
MYPACKAGECMAKEEXPORT()
MYPACKAGEPKGCONFIGEXPORT()

#############
# Packaging #
#############
MYPACKAGEPACK("" "Marpa Wrapper")

#########
# Setup #
#########
MYPACKAGEPRINTSETUP()
