CMAKE_MINIMUM_REQUIRED(VERSION 2.6.3)

# ROOT_DIR should be set to root of the repository (where to find the .svn or .hg directory)
# SOURCE_DIR should be set to root of your code (where to find CMakeLists.txt)

# Replace spaces by semi-columns
IF(CMAKE_MODULE_PATH)
  STRING(REPLACE " " ";" CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
ENDIF(CMAKE_MODULE_PATH)

SET(CMAKE_MODULE_PATH ${SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})

IF(NOT ROOT_DIR AND SOURCE_DIR)
  SET(ROOT_DIR ${SOURCE_DIR})
ENDIF(NOT ROOT_DIR AND SOURCE_DIR)

IF(NOT SOURCE_DIR AND ROOT_DIR)
  SET(SOURCE_DIR ${ROOT_DIR})
ENDIF(NOT SOURCE_DIR AND ROOT_DIR)

MACRO(NOW RESULT)
  IF (WIN32)
    EXECUTE_PROCESS(COMMAND "wmic" "os" "get" "localdatetime" OUTPUT_VARIABLE DATETIME)
    IF(NOT DATETIME MATCHES "ERROR")
      STRING(REGEX REPLACE ".*\n([0-9][0-9][0-9][0-9])([0-9][0-9])([0-9][0-9])([0-9][0-9])([0-9][0-9])([0-9][0-9]).*" "\\1-\\2-\\3 \\4:\\5:\\6" ${RESULT} "${DATETIME}")
    ENDIF(NOT DATETIME MATCHES "ERROR")
  ELSEIF(UNIX)
    EXECUTE_PROCESS(COMMAND "date" "+'%Y-%m-%d %H:%M:%S'" OUTPUT_VARIABLE ${RESULT})
  ELSE (WIN32)
    MESSAGE(SEND_ERROR "date not implemented")
    SET(${RESULT} "0000-00-00 00:00:00")
  ENDIF (WIN32)
ENDMACRO(NOW)

IF(EXISTS "${ROOT_DIR}/.svn/")
  FIND_PACKAGE(Subversion)

  IF(SUBVERSION_FOUND)
    Subversion_WC_INFO(${ROOT_DIR} ER)
    SET(REVISION ${ER_WC_REVISION})
  ENDIF(SUBVERSION_FOUND)
ENDIF(EXISTS "${ROOT_DIR}/.svn/")

IF(EXISTS "${ROOT_DIR}/.hg/")
  FIND_PACKAGE(Mercurial)

  IF(MERCURIAL_FOUND)
    Mercurial_WC_INFO(${ROOT_DIR} ER)
    SET(REVISION ${ER_WC_REVISION})
  ENDIF(MERCURIAL_FOUND)
ENDIF(EXISTS "${ROOT_DIR}/.hg/")

IF(REVISION)
  IF(EXISTS ${SOURCE_DIR}/revision.h.in)
    NOW(BUILD_DATE)
    CONFIGURE_FILE(${SOURCE_DIR}/revision.h.in revision.h.txt)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy revision.h.txt revision.h) # copy_if_different
  ENDIF(EXISTS ${SOURCE_DIR}/revision.h.in)
ENDIF(REVISION)
