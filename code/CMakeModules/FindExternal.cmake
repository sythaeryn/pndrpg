# Look for a directory containing external libraries.
#
# The following values are defined
# EXTERNAL_PATH         - where to find external
# EXTERNAL_INCLUDE_PATH - where to find external includes
# EXTERNAL_LIBRARY_PATH - where to find external libraries
# EXTERNAL_FOUND        - True if the external libraries are available

SET(EXTERNAL_TEMP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external ${CMAKE_CURRENT_SOURCE_DIR}/../external ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty ${EXTERNAL_PATH})
SET(EXTERNAL_TEMP_FILE "include/zlib.h")
SET(EXTERNAL_NAME "external")

# If using STLport preprend external_stlport
IF(WITH_STLPORT)
  SET(EXTERNAL_TEMP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external_stlport ${CMAKE_CURRENT_SOURCE_DIR}/../external_stlport ${EXTERNAL_TEMP_PATH})
  SET(EXTERNAL_TEMP_FILE "include/stlport/string")
  SET(EXTERNAL_NAME "external with STLport")
ENDIF(WITH_STLPORT)

FIND_PATH(EXTERNAL_PATH
  ${EXTERNAL_TEMP_FILE}
  PATHS
  $ENV{EXTERNAL_PATH}
  ${EXTERNAL_TEMP_PATH}
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

IF(EXTERNAL_PATH)
  SET(EXTERNAL_FOUND TRUE)
  SET(EXTERNAL_INCLUDE_PATH "${EXTERNAL_PATH}/include")

  # Using 32 or 64 bits libraries
  IF(TARGET_X64)
    SET(EXTERNAL_LIBRARY_PATH "${EXTERNAL_PATH}/lib64")
  ELSE(TARGET_X64)
    SET(EXTERNAL_LIBRARY_PATH "${EXTERNAL_PATH}/lib")
  ENDIF(TARGET_X64)

  SET(CMAKE_INCLUDE_PATH "${EXTERNAL_INCLUDE_PATH};${CMAKE_INCLUDE_PATH}")
  # Stupid hack for FindOpenAL.cmake
  SET(CMAKE_INCLUDE_PATH "${EXTERNAL_PATH};${CMAKE_INCLUDE_PATH}")
  SET(CMAKE_LIBRARY_PATH "${EXTERNAL_LIBRARY_PATH};${CMAKE_LIBRARY_PATH}")
ENDIF(EXTERNAL_PATH)

IF(EXTERNAL_FOUND)
  IF(NOT External_FIND_QUIETLY)
    MESSAGE(STATUS "Found ${EXTERNAL_NAME}: ${EXTERNAL_PATH}")
  ENDIF(NOT External_FIND_QUIETLY)
ELSE(EXTERNAL_FOUND)
  IF(External_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Unable to find ${EXTERNAL_NAME}!")
  ELSE(External_FIND_REQUIRED)
    IF(NOT External_FIND_QUIETLY)
      MESSAGE(STATUS "Warning: Unable to find ${EXTERNAL_NAME}!")
    ENDIF(NOT External_FIND_QUIETLY)
  ENDIF(External_FIND_REQUIRED)
ENDIF(EXTERNAL_FOUND)

MARK_AS_ADVANCED(EXTERNAL_INCLUDE_PATH EXTERNAL_LIBRARY_PATH)
