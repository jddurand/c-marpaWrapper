#
# Multi-config generator, e.g. Visual Studio on Windows:
#
# cmake -S c-marpaWrapper -B c-marpaWrapper-build
# cmake --build c-marpaWrapper-build --config RelWithDebInfo
# ctest --test-dir c-marpaWrapper-build -C RelWithDebInfo
# Windows:
#   cmake --install c-marpaWrapper-build --config RelWithDebInfo --prefix %cd%/c-marpaWrapper-install
# Others:
#   cmake --install c-marpaWrapper-build --config RelWithDebInfo --prefix `pwd`/c-marpaWrapper-install
# cmake --build c-marpaWrapper-build --config RelWithDebInfo --target package
#
# Single-config generator, e.g. NMake Makefiles on Windows, Unix Makefiles on Linxu:
#
# cmake -S c-marpaWrapper -B c-marpaWrapper-build -DCMAKE_BUILD_TYPE=RelWithDebInfo
# cmake --build c-marpaWrapper-build
# ctest --test-dir c-marpaWrapper-build
# Windows:
#   cmake --install c-marpaWrapper-build --prefix %cd%/c-marpaWrapper-install
# Others:
#   cmake --install c-marpaWrapper-build --prefix `pwd`/c-marpaWrapper-install
# cmake --build c-marpaWrapper-build --target package
#
# Local tests done with: cmake -S c-marpaWrapper -B c-marpaWrapper-build -DCMAKE_HELPERS_DEBUG=OFF -DICU_ROOT=C:\icu4c-74_2-Win64-MSVC2019
#
cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)
project(marpaWrapper VERSION 1.0.104 LANGUAGES C)
#
# Get library helper
#
include(FetchContent)
FetchContent_Declare(cmake-helpers GIT_REPOSITORY https://github.com/jddurand/cmake-helpers.git GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(cmake-helpers)
#
# Dependencies
#
cmake_helpers_depend(genericStack       EXTERNALPROJECT_ADD_ARGS GIT_REPOSITORY https://github.com/jddurand/c-genericStack.git       GIT_SHALLOW TRUE FIND_PACKAGE_ARGS REQUIRED CONFIG)
cmake_helpers_depend(genericLogger      EXTERNALPROJECT_ADD_ARGS GIT_REPOSITORY https://github.com/jddurand/c-genericLogger.git      GIT_SHALLOW TRUE FIND_PACKAGE_ARGS REQUIRED CONFIG)
cmake_helpers_depend(genericHash        EXTERNALPROJECT_ADD_ARGS GIT_REPOSITORY https://github.com/jddurand/c-genericHash.git        GIT_SHALLOW TRUE FIND_PACKAGE_ARGS REQUIRED CONFIG)
cmake_helpers_depend(genericSparseArray EXTERNALPROJECT_ADD_ARGS GIT_REPOSITORY https://github.com/jddurand/c-genericSparseArray.git GIT_SHALLOW TRUE FIND_PACKAGE_ARGS REQUIRED CONFIG)
#
# Create library
#
cmake_helpers_library(marpaWrapper
  CONFIG_ARGS                    include/marpaWrapper/internal/config.h.in include/marpaWrapper/internal/config.h
  SOURCES_BASE_DIRS              ${CMAKE_CURRENT_SOURCE_DIR}/amalgamation
  FIND_DEPENDENCIES              "genericStack REQUIRED CONFIG" "genericLogger REQUIRED CONFIG" "genericHash REQUIRED CONFIG" "genericSparseArray REQUIRED CONFIG" 
  DEPENDS                        genericStack::genericStack genericLogger::genericLogger genericHash::genericHash genericSparseArray::genericSparseArray
  TARGETS_OUTVAR                 targets
)
#
# CMake framework does not know about it
#
foreach(_target IN LISTS targets)
  target_include_directories(${_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libmarpa/work/stage)
  target_include_directories(${_target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include/marpaWrapper/internal)
  foreach(_type MAJOR MICRO MINOR)
    target_compile_definitions(${_target} PRIVATE -DMARPA_LIB_${_type}_VERSION=MARPA_${_type}_VERSION)
  endforeach()
endforeach()
#
# EXEs
#
include(CTest)
cmake_helpers_exe(marpaWrapperTester             SOURCES test/marpaWrapperTester.c INSTALL FALSE TEST TRUE)
cmake_helpers_exe(marpaWrapperAsfTester          SOURCES test/marpaWrapperAsfTester.c INSTALL FALSE TEST TRUE)
cmake_helpers_exe(marpaWrapperAsfValueTester     SOURCES test/marpaWrapperAsfValueTester.c INSTALL FALSE TEST TRUE)
cmake_helpers_exe(marpaWrapperAsfAmbiguousTester SOURCES test/marpaWrapperAsfAmbiguousTester.c INSTALL FALSE TEST TRUE)
#
# Package
#
cmake_helpers_package()
