# Inspired from ax_gcc_func_attribute.m4
MACRO (FINDGCCFUNCATTRIBUTE attribute)
  GET_PROPERTY(source_dir_set GLOBAL PROPERTY MYPACKAGE_SOURCE_DIR SET)
  IF (NOT ${source_dir_set})
    MESSAGE (WARNING "Cannot check ${attribute}, property MYPACKAGE_SOURCE_DIR is not set")
  ELSE ()
    STRING (TOUPPER ${attribute} ATTRIBUTE)
    IF (NOT C_GCC_FUNC_ATTRIBUTE_${ATTRIBUTE}_SINGLETON)
      GET_PROPERTY(source_dir GLOBAL PROPERTY MYPACKAGE_SOURCE_DIR)
      SET (_C_GCC_FUNC_ATTRIBUTE_${ATTRIBUTE}_FOUND FALSE)
      MESSAGE(STATUS "Looking for GCC function attribute ${attribute}")
      TRY_COMPILE (C_GCC_FUNC_ATTRIBUTE_HAS_${ATTRIBUTE} ${CMAKE_CURRENT_BINARY_DIR}
        ${source_dir}/gccfuncattribute.c
        COMPILE_DEFINITIONS -DC_GCC_FUNC_ATTRIBUTE_${ATTRIBUTE})
      IF (C_GCC_FUNC_ATTRIBUTE_HAS_${ATTRIBUTE})
        MESSAGE(STATUS "Looking for GCC function attribute ${attribute} - supported")
        SET (_C_GCC_FUNC_ATTRIBUTE_${ATTRIBUTE} "__attribute__((${attribute}))")
        SET (_C_GCC_FUNC_ATTRIBUTE_${ATTRIBUTE}_FOUND TRUE)
      ELSE ()
        MESSAGE(STATUS "Looking for GCC function attribute ${attribute} - not supported")
      ENDIF ()
      IF (_C_GCC_FUNC_ATTRIBUTE_${ATTRIBUTE}_FOUND)
        SET (C_GCC_FUNC_ATTRIBUTE_${ATTRIBUTE} "${_C_GCC_FUNC_ATTRIBUTE_${ATTRIBUTE}}" CACHE STRING "GCC function attribute ${attribute}")
        MARK_AS_ADVANCED (C_GCC_FUNC_ATTRIBUTE_${ATTRIBUTE})
      ENDIF ()
      SET (C_GCC_FUNC_ATTRIBUTE_${ATTRIBUTE}_SINGLETON TRUE CACHE BOOL "GCC function attribute ${attribute}")
      MARK_AS_ADVANCED (C_GCC_FUNC_ATTRIBUTE_${ATTRIBUTE}_SINGLETON)
    ENDIF ()
  ENDIF ()
ENDMACRO()
