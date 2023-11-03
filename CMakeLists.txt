CMAKE_MINIMUM_REQUIRED (VERSION 3.26.0 FATAL_ERROR)
PROJECT(genericSparseArray VERSION 1.0.53 LANGUAGES C)
IF (NOT MYPACKAGEBOOTSTRAP_DONE)
  INCLUDE ("3rdparty/github/cmake-utils/cmake/MyPackageBootstrap.cmake")
ENDIF ()
MYPACKAGESTART ()

###############
# Executables #
###############
MYPACKAGETESTEXECUTABLE(genericSparseArrayTester test/genericSparseArray.c)

################
# Dependencies #
################
MYPACKAGEDEPENDENCY (genericHash   "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/github/genericHash" LOCAL IFACE PUBLIC IFACE_OBJECTS)  # present in public API
MYPACKAGEDEPENDENCY (genericLogger "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/github/genericLogger" LOCAL TESTS PUBLIC SHARED_LIB) # used in tests

#########
# Tests #
#########
MYPACKAGECHECK(genericSparseArrayTester)

##########
# Export #
##########
MYPACKAGECMAKEEXPORT()
MYPACKAGEPKGCONFIGEXPORT()

#############
# Packaging #
#############
MYPACKAGEPACK("" "Generic Sparse Array")

#########
# Setup #
#########
MYPACKAGEPRINTSETUP()
