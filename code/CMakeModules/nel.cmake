# Force Release configuration for compiler checks
SET(CMAKE_TRY_COMPILE_CONFIGURATION "Release")

# Force Release configuration by default
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

###
# Helper macro that generates .pc and installs it.
# Argument: name - the name of the .pc package, e.g. "nel-pacs.pc"
###
MACRO(NL_GEN_PC name)
  IF(NOT WIN32 AND WITH_INSTALL_LIBRARIES)
    CONFIGURE_FILE(${name}.in "${CMAKE_CURRENT_BINARY_DIR}/${name}")
    IF(CMAKE_LIBRARY_ARCHITECTURE)
      INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${name}" DESTINATION lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig)
    ELSE(CMAKE_LIBRARY_ARCHITECTURE)
      INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${name}" DESTINATION lib/pkgconfig)
    ENDIF(CMAKE_LIBRARY_ARCHITECTURE)
  ENDIF(NOT WIN32 AND WITH_INSTALL_LIBRARIES)
ENDMACRO(NL_GEN_PC)

###
# Helper macro that generates revision.h from revision.h.in
###
MACRO(NL_GEN_REVISION_H)
  IF(EXISTS ${CMAKE_SOURCE_DIR}/revision.h.in)
    INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
    ADD_DEFINITIONS(-DHAVE_REVISION_H)
    SET(HAVE_REVISION_H ON)

    # if already generated
    IF(EXISTS ${CMAKE_SOURCE_DIR}/revision.h)
      # copy it
      MESSAGE(STATUS "Copying provided revision.h...")
      FILE(COPY ${CMAKE_SOURCE_DIR}/revision.h DESTINATION ${CMAKE_BINARY_DIR})
    ELSE(EXISTS ${CMAKE_SOURCE_DIR}/revision.h)
      # a custom target that is always built
      ADD_CUSTOM_TARGET(revision ALL
        DEPENDS ${CMAKE_BINARY_DIR}/revision.h)

      # creates revision.h using cmake script
      ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/revision.h
        COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DROOT_DIR=${CMAKE_SOURCE_DIR}/..
        -P ${CMAKE_SOURCE_DIR}/CMakeModules/GetRevision.cmake)

      # revision.h is a generated file
      SET_SOURCE_FILES_PROPERTIES(${CMAKE_BINARY_DIR}/revision.h
        PROPERTIES GENERATED TRUE
        HEADER_FILE_ONLY TRUE)
    ENDIF(EXISTS ${CMAKE_SOURCE_DIR}/revision.h)
  ENDIF(EXISTS ${CMAKE_SOURCE_DIR}/revision.h.in)
ENDMACRO(NL_GEN_REVISION_H)

###
#
###
MACRO(NL_TARGET_LIB name)
  IF(WITH_STATIC)
    ADD_LIBRARY(${name} STATIC ${ARGN})
  ELSE(WITH_STATIC)
    ADD_LIBRARY(${name} SHARED ${ARGN})
  ENDIF(WITH_STATIC)
ENDMACRO(NL_TARGET_LIB)

###
#
###
MACRO(NL_TARGET_DRIVER name)
  IF(WITH_STATIC_DRIVERS)
    ADD_LIBRARY(${name} STATIC ${ARGN})
  ELSE(WITH_STATIC_DRIVERS)
    ADD_LIBRARY(${name} MODULE ${ARGN})
  ENDIF(WITH_STATIC_DRIVERS)
ENDMACRO(NL_TARGET_DRIVER)

###
# Helper macro that sets the default library properties.
# Argument: name - the target name whose properties are being set
# Argument:
###
MACRO(NL_DEFAULT_PROPS name label)
  # Note: This is just a workaround for a CMake bug generating VS10 files with a colon in the project name.
  # CMake Bug ID: http://www.cmake.org/Bug/view.php?id=11819
  STRING(REGEX REPLACE "\\:" " -" proj_label ${label})
  SET_TARGET_PROPERTIES(${name} PROPERTIES PROJECT_LABEL ${proj_label})
  GET_TARGET_PROPERTY(type ${name} TYPE)
  IF(${type} STREQUAL SHARED_LIBRARY)
    # Set versions only if target is a shared library
    SET_TARGET_PROPERTIES(${name} PROPERTIES
      VERSION ${NL_VERSION} SOVERSION ${NL_VERSION_MAJOR})
    IF(NL_LIB_PREFIX)
      SET_TARGET_PROPERTIES(${name} PROPERTIES INSTALL_NAME_DIR ${NL_LIB_PREFIX})
    ENDIF(NL_LIB_PREFIX)
  ENDIF(${type} STREQUAL SHARED_LIBRARY)

  IF(${type} STREQUAL EXECUTABLE AND WIN32)
    SET_TARGET_PROPERTIES(${name} PROPERTIES
      VERSION ${NL_VERSION}
      SOVERSION ${NL_VERSION_MAJOR}
      COMPILE_FLAGS "/GA"
      LINK_FLAGS "/VERSION:${NL_VERSION}")
  ENDIF(${type} STREQUAL EXECUTABLE AND WIN32)

  IF(WITH_STLPORT AND WIN32)
    SET_TARGET_PROPERTIES(${name} PROPERTIES COMPILE_FLAGS "/X")
  ENDIF(WITH_STLPORT AND WIN32)
ENDMACRO(NL_DEFAULT_PROPS)

###
# Adds the target suffix on Windows.
# Argument: name - the library's target name.
###
MACRO(NL_ADD_LIB_SUFFIX name)
  IF(WIN32)
    SET_TARGET_PROPERTIES(${name} PROPERTIES DEBUG_POSTFIX "_d" RELEASE_POSTFIX "_r")
  ENDIF(WIN32)
ENDMACRO(NL_ADD_LIB_SUFFIX)

###
# Adds the runtime link flags for Win32 binaries and links STLport.
# Argument: name - the target to add the link flags to.
###
MACRO(NL_ADD_RUNTIME_FLAGS name)
  IF(WIN32)
#    SET_TARGET_PROPERTIES(${name} PROPERTIES
#      LINK_FLAGS_DEBUG "${CMAKE_LINK_FLAGS_DEBUG}"
#      LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE}")
  ENDIF(WIN32)
  IF(WITH_STLPORT)
    TARGET_LINK_LIBRARIES(${name} ${STLPORT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  ENDIF(WITH_STLPORT)
ENDMACRO(NL_ADD_RUNTIME_FLAGS)

MACRO(NL_ADD_STATIC_VID_DRIVERS name)
  IF(WITH_STATIC_DRIVERS)
    IF(WIN32)
      IF(WITH_DRIVER_DIRECT3D)
        TARGET_LINK_LIBRARIES(${name} nel_drv_direct3d_win)
      ENDIF(WITH_DRIVER_DIRECT3D)
    ENDIF(WIN32)

    IF(WITH_DRIVER_OPENGL)
      IF(WIN32)
        TARGET_LINK_LIBRARIES(${name} nel_drv_opengl_win)
      ELSE(WIN32)
        TARGET_LINK_LIBRARIES(${name} nel_drv_opengl)
      ENDIF(WIN32)
    ENDIF(WITH_DRIVER_OPENGL)

    IF(WITH_DRIVER_OPENGLES)
      IF(WIN32)
        TARGET_LINK_LIBRARIES(${name} nel_drv_opengles_win)
      ELSE(WIN32)
        TARGET_LINK_LIBRARIES(${name} nel_drv_opengles)
      ENDIF(WIN32)
    ENDIF(WITH_DRIVER_OPENGLES)
  ENDIF(WITH_STATIC_DRIVERS)
ENDMACRO(NL_ADD_STATIC_VID_DRIVERS)

MACRO(NL_ADD_STATIC_SND_DRIVERS name)
  IF(WITH_STATIC_DRIVERS)
    IF(WIN32)
      IF(WITH_DRIVER_DSOUND)
        TARGET_LINK_LIBRARIES(${name} nel_drv_dsound_win)
      ENDIF(WITH_DRIVER_DSOUND)

      IF(WITH_DRIVER_XAUDIO2)
        TARGET_LINK_LIBRARIES(${name} nel_drv_xaudio2_win)
      ENDIF(WITH_DRIVER_XAUDIO2)

      IF(WITH_DRIVER_OPENAL)
        TARGET_LINK_LIBRARIES(${name} nel_drv_openal_win)
      ENDIF(WITH_DRIVER_OPENAL)

      IF(WITH_DRIVER_FMOD)
        TARGET_LINK_LIBRARIES(${name} nel_drv_fmod_win)
      ENDIF(WITH_DRIVER_FMOD)
    ELSE(WIN32)
      IF(WITH_DRIVER_OPENAL)
        TARGET_LINK_LIBRARIES(${name} nel_drv_openal)
      ENDIF(WITH_DRIVER_OPENAL)

      IF(WITH_DRIVER_FMOD)
        TARGET_LINK_LIBRARIES(${name} nel_drv_fmod)
      ENDIF(WITH_DRIVER_FMOD)
    ENDIF(WIN32)

  ENDIF(WITH_STATIC_DRIVERS)
ENDMACRO(NL_ADD_STATIC_SND_DRIVERS)

###
# Checks build vs. source location. Prevents In-Source builds.
###
MACRO(CHECK_OUT_OF_SOURCE)
  IF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    MESSAGE(FATAL_ERROR "

CMake generation for this project is not allowed within the source directory!
Remove the CMakeCache.txt file and try again from another folder, e.g.:

   rm CMakeCache.txt
   mkdir cmake
   cd cmake
   cmake ..
    ")
  ENDIF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

ENDMACRO(CHECK_OUT_OF_SOURCE)

MACRO(NL_SETUP_DEFAULT_OPTIONS)
  ###
  # Features
  ###
  OPTION(WITH_LOGGING             "With Logging"                                  ON )
  OPTION(WITH_COVERAGE            "With Code Coverage Support"                    OFF)
  OPTION(WITH_PCH                 "With Precompiled Headers"                      ON )
  OPTION(FINAL_VERSION            "Build in Final Version mode"                   ON )

  # Default to static building on Windows.
  IF(WIN32)
    OPTION(WITH_STATIC            "With static libraries."                        ON )
  ELSE(WIN32)
    OPTION(WITH_STATIC            "With static libraries."                        OFF)
  ENDIF(WIN32)
  OPTION(WITH_STATIC_DRIVERS      "With static drivers."                          OFF)
  IF(WIN32)
    OPTION(WITH_EXTERNAL          "With provided external."                       ON )
  ELSE(WIN32)
    OPTION(WITH_EXTERNAL          "With provided external."                       OFF)
  ENDIF(WIN32)
  OPTION(WITH_STATIC_EXTERNAL     "With static external libraries"                OFF)
  OPTION(WITH_INSTALL_LIBRARIES   "Install development files."                    ON )

  ###
  # GUI toolkits
  ###
  OPTION(WITH_GTK                 "With GTK Support"                              OFF)
  OPTION(WITH_QT                  "With QT Support"                               OFF)

  IF(WIN32 AND MFC_FOUND)
    OPTION(WITH_MFC               "With MFC Support"                              ON )
  ELSE(WIN32 AND MFC_FOUND)
    OPTION(WITH_MFC               "With MFC Support"                              OFF)
  ENDIF(WIN32 AND MFC_FOUND)

  ###
  # Optional support
  ###

  # Check if CMake is launched from a Debian packaging script
  SET(DEB_HOST_GNU_CPU $ENV{DEB_HOST_GNU_CPU})

  # Don't strip if generating a .deb
  IF(DEB_HOST_GNU_CPU)
    OPTION(WITH_SYMBOLS           "Keep debug symbols in binaries"                 ON )
  ELSE(DEB_HOST_GNU_CPU)
    OPTION(WITH_SYMBOLS           "Keep debug symbols in binaries"                 OFF)
  ENDIF(DEB_HOST_GNU_CPU)

  IF(WIN32)
    OPTION(WITH_STLPORT           "With STLport support."                         ON )
  ELSE(WIN32)
    OPTION(WITH_STLPORT           "With STLport support."                         OFF)
  ENDIF(WIN32)

  OPTION(BUILD_DASHBOARD          "Build to the CDash dashboard"                  OFF)

  OPTION(WITH_NEL                 "Build NeL (nearly always required)."           ON )
  OPTION(WITH_NELNS               "Build NeL Network Services."                   OFF)
  OPTION(WITH_RYZOM               "Build Ryzom Core."                             ON )
  OPTION(WITH_SNOWBALLS           "Build Snowballs."                              OFF)
ENDMACRO(NL_SETUP_DEFAULT_OPTIONS)

MACRO(NL_SETUP_NEL_DEFAULT_OPTIONS)
  ###
  # Core libraries
  ###
  OPTION(WITH_NET                 "Build NLNET"                                   ON )
  OPTION(WITH_3D                  "Build NL3D"                                    ON )
  OPTION(WITH_PACS                "Build NLPACS"                                  ON )
  OPTION(WITH_GEORGES             "Build NLGEORGES"                               ON )
  OPTION(WITH_LIGO                "Build NLLIGO"                                  ON )
  OPTION(WITH_LOGIC               "Build NLLOGIC"                                 ON )
  OPTION(WITH_SOUND               "Build NLSOUND"                                 ON )

  ###
  # Drivers Support
  ###
  OPTION(WITH_DRIVER_OPENGL       "Build OpenGL Driver (3D)"                      ON )
  OPTION(WITH_DRIVER_OPENGLES     "Build OpenGL ES Driver (3D)"                   OFF)
  OPTION(WITH_DRIVER_DIRECT3D     "Build Direct3D Driver (3D)"                    OFF)
  OPTION(WITH_DRIVER_OPENAL       "Build OpenAL Driver (Sound)"                   ON )
  OPTION(WITH_DRIVER_FMOD         "Build FMOD Driver (Sound)"                     OFF)
  OPTION(WITH_DRIVER_DSOUND       "Build DirectSound Driver (Sound)"              OFF)
  OPTION(WITH_DRIVER_XAUDIO2      "Build XAudio2 Driver (Sound)"                  OFF)

  ###
  # Optional support
  ###
  OPTION(WITH_NEL_CEGUI           "Build CEGUI Renderer"                          OFF)
  OPTION(WITH_NEL_TOOLS           "Build NeL Tools"                               ON )
  OPTION(WITH_NEL_MAXPLUGIN       "Build NeL 3dsMax Plugin"                       OFF)
  OPTION(WITH_NEL_SAMPLES         "Build NeL Samples"                             ON )
  OPTION(WITH_NEL_TESTS           "Build NeL Unit Tests"                          ON )
ENDMACRO(NL_SETUP_NEL_DEFAULT_OPTIONS)

MACRO(NL_SETUP_NELNS_DEFAULT_OPTIONS)
  ###
  # Core libraries
  ###
  OPTION(WITH_NELNS_SERVER        "Build NeLNS Services"                          ON )
  OPTION(WITH_NELNS_LOGIN_SYSTEM  "Build NeLNS Login System Tools"                ON )
ENDMACRO(NL_SETUP_NELNS_DEFAULT_OPTIONS)

MACRO(NL_SETUP_RYZOM_DEFAULT_OPTIONS)
  ###
  # Core libraries
  ###
  OPTION(WITH_RYZOM_CLIENT        "Build Ryzom Core Client"                       ON )
  OPTION(WITH_RYZOM_TOOLS         "Build Ryzom Core Tools"                        ON )
  OPTION(WITH_RYZOM_SERVER        "Build Ryzom Core Services"                     ON )
  OPTION(WITH_RYZOM_SOUND         "Enable Ryzom Core Sound"                       ON )

  ###
  # Optional support
  ###
  OPTION(WITH_LUA51               "Build Ryzom Core using Lua51"                  ON )
ENDMACRO(NL_SETUP_RYZOM_DEFAULT_OPTIONS)

MACRO(NL_SETUP_SNOWBALLS_DEFAULT_OPTIONS)
  ###
  # Core libraries
  ###
  OPTION(WITH_SNOWBALLS_CLIENT    "Build Snowballs Client"                        ON )
  OPTION(WITH_SNOWBALLS_SERVER    "Build Snowballs Services"                      ON )
ENDMACRO(NL_SETUP_SNOWBALLS_DEFAULT_OPTIONS)

MACRO(NL_SETUP_BUILD)

  #-----------------------------------------------------------------------------
  # Setup the buildmode variables.
  #
  # None                  = NL_RELEASE
  # Debug                 = NL_DEBUG
  # Release               = NL_RELEASE

  SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

  IF(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(NL_BUILD_MODE "NL_DEBUG")
  ELSE(CMAKE_BUILD_TYPE MATCHES "Debug")
    IF(CMAKE_BUILD_TYPE MATCHES "Release")
      SET(NL_BUILD_MODE "NL_RELEASE")
    ELSE(CMAKE_BUILD_TYPE MATCHES "Release")
      SET(NL_BUILD_MODE "NL_RELEASE")
      # enforce release mode if it's neither Debug nor Release
      SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    ENDIF(CMAKE_BUILD_TYPE MATCHES "Release")
  ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

  SET(HOST_CPU ${CMAKE_HOST_SYSTEM_PROCESSOR})

  IF(HOST_CPU MATCHES "amd64")
    SET(HOST_CPU "x86_64")
  ELSEIF(HOST_CPU MATCHES "i.86")
    SET(HOST_CPU "x86")
  ENDIF(HOST_CPU MATCHES "amd64")
  
  # Determine target CPU
  IF(NOT TARGET_CPU)
    SET(TARGET_CPU $ENV{DEB_HOST_GNU_CPU})
  ENDIF(NOT TARGET_CPU)

  # If not specified, use the same CPU as host
  IF(NOT TARGET_CPU)
    SET(TARGET_CPU ${CMAKE_SYSTEM_PROCESSOR})
  ENDIF(NOT TARGET_CPU)

  IF(TARGET_CPU MATCHES "amd64")
    SET(TARGET_CPU "x86_64")
  ELSEIF(TARGET_CPU MATCHES "i.86")
    SET(TARGET_CPU "x86")
  ENDIF(TARGET_CPU MATCHES "amd64")

  # DEB_HOST_ARCH_ENDIAN is 'little' or 'big'
  # DEB_HOST_ARCH_BITS is '32' or '64'

  IF(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    SET(CLANG ON)
    MESSAGE(STATUS "Using Clang compiler")
  ENDIF(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")

  IF(CMAKE_GENERATOR MATCHES "Xcode")
    SET(XCODE ON)
    MESSAGE(STATUS "Generating Xcode project")
  ENDIF(CMAKE_GENERATOR MATCHES "Xcode")

  # If target and host CPU are the same
  IF("${HOST_CPU}" STREQUAL "${TARGET_CPU}")
    # x86-compatible CPU
    IF(HOST_CPU MATCHES "x86")
      IF(NOT CMAKE_SIZEOF_VOID_P)
        INCLUDE (CheckTypeSize)
        CHECK_TYPE_SIZE("void*"  CMAKE_SIZEOF_VOID_P)
      ENDIF(NOT CMAKE_SIZEOF_VOID_P)

      # Using 32 or 64 bits libraries
      IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(TARGET_CPU "x86_64")
      ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(TARGET_CPU "x86")
      ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    ELSEIF(HOST_CPU MATCHES "arm")
      SET(TARGET_CPU "arm")
    ELSE(HOST_CPU MATCHES "x86")
      SET(TARGET_CPU "unknown")
      MESSAGE(STATUS "Unknown architecture: ${HOST_CPU}")
    ENDIF(HOST_CPU MATCHES "x86")
    # TODO: add checks for PPC
  ELSE("${HOST_CPU}" STREQUAL "${TARGET_CPU}")
    MESSAGE(STATUS "Compiling on ${HOST_CPU} for ${TARGET_CPU}")
  ENDIF("${HOST_CPU}" STREQUAL "${TARGET_CPU}")

  # Use values from environment variables
  SET(PLATFORM_CFLAGS "$ENV{CFLAGS} ${PLATFORM_CFLAGS}")
  SET(PLATFORM_LINKFLAGS "$ENV{LDFLAGS} ${PLATFORM_LINKFLAGS}")

  # Remove -g and -O flag because we are managing them ourself
  STRING(REPLACE "-g" "" PLATFORM_CFLAGS ${PLATFORM_CFLAGS})
  STRING(REGEX REPLACE "-O[0-9s]" "" PLATFORM_CFLAGS ${PLATFORM_CFLAGS})

  # Strip spaces
  STRING(STRIP ${PLATFORM_CFLAGS} PLATFORM_CFLAGS)
  STRING(STRIP ${PLATFORM_LINKFLAGS} PLATFORM_LINKFLAGS)

  IF(TARGET_CPU STREQUAL "x86_64")
    SET(TARGET_X64 1)
    SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -DHAVE_X86_64")
  ELSEIF(TARGET_CPU STREQUAL "x86")
    SET(TARGET_X86 1)
    SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -DHAVE_X86")
  ELSEIF(TARGET_CPU STREQUAL "arm")
    SET(TARGET_ARM 1)
    SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -DHAVE_ARM")
  ENDIF(TARGET_CPU STREQUAL "x86_64")

  # Fix library paths suffixes for Debian MultiArch
  SET(DEBIAN_MULTIARCH $ENV{DEB_HOST_MULTIARCH})

  IF(DEBIAN_MULTIARCH)
    SET(CMAKE_LIBRARY_ARCHITECTURE ${DEBIAN_MULTIARCH})
  ENDIF(DEBIAN_MULTIARCH)

  IF(CMAKE_LIBRARY_ARCHITECTURE)
    SET(CMAKE_LIBRARY_PATH /lib/${CMAKE_LIBRARY_ARCHITECTURE} /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_PATH})
    IF(TARGET_X64)
      SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /lib64 /usr/lib64)
    ENDIF(TARGET_X64)
    IF(TARGET_X86)
      SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /lib32 /usr/lib32)
    ENDIF(TARGET_X86)
  ENDIF(CMAKE_LIBRARY_ARCHITECTURE)

  IF(MSVC)
    IF(MSVC10)
      # /Ox is working with VC++ 2010, but custom optimizations don't exist
      SET(SPEED_OPTIMIZATIONS "/Ox /GF /GS-")
      # without inlining it's unusable, use custom optimizations again
      SET(MIN_OPTIMIZATIONS "/Od /Ob1")
    ELSEIF(MSVC90)
      # don't use a /O[012x] flag if you want custom optimizations
      SET(SPEED_OPTIMIZATIONS "/Ob2 /Oi /Ot /Oy /GT /GF /GS-")
      # without inlining it's unusable, use custom optimizations again
      SET(MIN_OPTIMIZATIONS "/Ob1")
    ELSEIF(MSVC80)
      # don't use a /O[012x] flag if you want custom optimizations
      SET(SPEED_OPTIMIZATIONS "/Ox /GF /GS-")
      # without inlining it's unusable, use custom optimizations again
      SET(MIN_OPTIMIZATIONS "/Od /Ob1")
    ELSE(MSVC10)
      MESSAGE(FATAL_ERROR "Can't determine compiler version ${MSVC_VERSION}")
    ENDIF(MSVC10)

    SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /DWIN32 /D_WINDOWS /W3 /Zm1000 /MP /Gy-")

    IF(TARGET_X64)
      # Fix a bug with Intellisense
      SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} /D_WIN64")
      # Fix a compilation error for some big C++ files
      SET(MIN_OPTIMIZATIONS "${MIN_OPTIMIZATIONS} /bigobj")
    ELSE(TARGET_X64)
      # Allows 32 bits applications to use 3 GB of RAM
      SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} /LARGEADDRESSAWARE")
    ENDIF(TARGET_X64)

    # Exceptions are only set for C++
    SET(PLATFORM_CXXFLAGS "${PLATFORM_CFLAGS} /EHa")

    IF(WITH_SYMBOLS)
      SET(NL_RELEASE_CFLAGS "/Zi ${NL_RELEASE_CFLAGS}")
      SET(NL_RELEASE_LINKFLAGS "/DEBUG ${NL_RELEASE_LINKFLAGS}")
    ELSE(WITH_SYMBOLS)
      SET(NL_RELEASE_LINKFLAGS "/RELEASE ${NL_RELEASE_LINKFLAGS}")
    ENDIF(WITH_SYMBOLS)

    SET(NL_DEBUG_CFLAGS "/Zi /MDd /RTC1 /D_DEBUG ${MIN_OPTIMIZATIONS} ${NL_DEBUG_CFLAGS}")
    SET(NL_RELEASE_CFLAGS "/MD /DNDEBUG ${SPEED_OPTIMIZATIONS} ${NL_RELEASE_CFLAGS}")
    SET(NL_DEBUG_LINKFLAGS "/DEBUG /OPT:NOREF /OPT:NOICF /NODEFAULTLIB:msvcrt /INCREMENTAL:YES ${NL_DEBUG_LINKFLAGS}")
    SET(NL_RELEASE_LINKFLAGS "/OPT:REF /OPT:ICF /INCREMENTAL:NO ${NL_RELEASE_LINKFLAGS}")
  ELSE(MSVC)
    IF(WIN32)
      SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -DWIN32 -D_WIN32")
    ENDIF(WIN32)

    IF(APPLE)
      IF(TARGET_CPU STREQUAL "x86")
        SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -arch i386")
      ENDIF(TARGET_CPU STREQUAL "x86")

      IF(TARGET_CPU STREQUAL "x86_64")
        SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -arch x86_64")
      ENDIF(TARGET_CPU STREQUAL "x86_64")
    ELSE(APPLE)
      IF(HOST_CPU STREQUAL "x86_64" AND TARGET_CPU STREQUAL "x86")
        SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -m32 -march=i686")
      ENDIF(HOST_CPU STREQUAL "x86_64" AND TARGET_CPU STREQUAL "x86")

      IF(HOST_CPU STREQUAL "x86" AND TARGET_CPU STREQUAL "x86_64")
        SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -m64")
      ENDIF(HOST_CPU STREQUAL "x86" AND TARGET_CPU STREQUAL "x86_64")
    ENDIF(APPLE)

    SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -D_REENTRANT -pipe -Wall -W -Wpointer-arith -Wsign-compare -Wno-deprecated-declarations -Wno-multichar -Wno-unused -fno-strict-aliasing")

    IF(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
      SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -ansi")
    ENDIF(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")

    IF(WITH_COVERAGE)
      SET(PLATFORM_CFLAGS "-fprofile-arcs -ftest-coverage ${PLATFORM_CFLAGS}")
    ENDIF(WITH_COVERAGE)

    IF(APPLE)
      SET(PLATFORM_CFLAGS "-gdwarf-2 ${PLATFORM_CFLAGS}")
    ENDIF(APPLE)

    IF(APPLE AND XCODE)
#      SET(CMAKE_OSX_SYSROOT "macosx" CACHE PATH "" FORCE)
    ELSEIF(APPLE AND NOT XCODE)
      IF(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.6")
      ENDIF(NOT CMAKE_OSX_DEPLOYMENT_TARGET)

      FOREACH(_SDK ${_CMAKE_OSX_SDKS})
        IF(${_SDK} MATCHES "MacOSX${CMAKE_OSX_DEPLOYMENT_TARGET}\\.sdk")
          SET(CMAKE_OSX_SYSROOT ${_SDK} CACHE PATH "" FORCE)
        ENDIF(${_SDK} MATCHES "MacOSX${CMAKE_OSX_DEPLOYMENT_TARGET}\\.sdk")
      ENDFOREACH(_SDK)

      IF(CMAKE_OSX_SYSROOT)
        SET(PLATFORM_CFLAGS "-isysroot ${CMAKE_OSX_SYSROOT} ${PLATFORM_CFLAGS}")
      ELSE(CMAKE_OSX_SYSROOT)
        MESSAGE(FATAL_ERROR "CMAKE_OSX_SYSROOT can't be determinated")
      ENDIF(CMAKE_OSX_SYSROOT)

      IF(CMAKE_OSX_ARCHITECTURES)
        FOREACH(_ARCH ${CMAKE_OSX_ARCHITECTURES})
          SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -arch ${_ARCH}")
        ENDFOREACH(_ARCH)
      ENDIF(CMAKE_OSX_ARCHITECTURES)
      IF(CMAKE_C_OSX_DEPLOYMENT_TARGET_FLAG)
        SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
      ENDIF(CMAKE_C_OSX_DEPLOYMENT_TARGET_FLAG)

      SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} -Wl,-headerpad_max_install_names")

      IF(HAVE_FLAG_SEARCH_PATHS_FIRST)
        SET(PLATFORM_LINKFLAGS "-Wl,-search_paths_first ${PLATFORM_LINKFLAGS}")
      ENDIF(HAVE_FLAG_SEARCH_PATHS_FIRST)
    ENDIF(APPLE AND XCODE)

    # Fix "relocation R_X86_64_32 against.." error on x64 platforms
    IF(TARGET_X64 AND WITH_STATIC AND NOT WITH_STATIC_DRIVERS)
      SET(PLATFORM_CFLAGS "-fPIC ${PLATFORM_CFLAGS}")
    ENDIF(TARGET_X64 AND WITH_STATIC AND NOT WITH_STATIC_DRIVERS)

    SET(PLATFORM_CXXFLAGS "${PLATFORM_CFLAGS} -ftemplate-depth-48")

    IF(NOT APPLE)
      SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} -Wl,--no-undefined -Wl,--as-needed")
    ENDIF(NOT APPLE)

    IF(WITH_SYMBOLS)
      SET(NL_RELEASE_CFLAGS "${NL_RELEASE_CFLAGS} -g")
    ELSE(WITH_SYMBOLS)
      IF(APPLE)
        SET(NL_RELEASE_LINKFLAGS "-Wl,-dead_strip -Wl,-x ${NL_RELEASE_LINKFLAGS}")
      ELSE(APPLE)
        SET(NL_RELEASE_LINKFLAGS "-Wl,-s ${NL_RELEASE_LINKFLAGS}")
      ENDIF(APPLE)
    ENDIF(WITH_SYMBOLS)

    SET(NL_DEBUG_CFLAGS "-g -DNL_DEBUG -D_DEBUG ${NL_DEBUG_CFLAGS}")
    SET(NL_RELEASE_CFLAGS "-DNL_RELEASE -DNDEBUG -O3 ${NL_RELEASE_CFLAGS}")
  ENDIF(MSVC)
ENDMACRO(NL_SETUP_BUILD)

MACRO(NL_SETUP_BUILD_FLAGS)
  SET(CMAKE_C_FLAGS ${PLATFORM_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_CXX_FLAGS ${PLATFORM_CXXFLAGS} CACHE STRING "" FORCE)

  ## Debug
  SET(CMAKE_C_FLAGS_DEBUG ${NL_DEBUG_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_CXX_FLAGS_DEBUG ${NL_DEBUG_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${PLATFORM_LINKFLAGS} ${NL_DEBUG_LINKFLAGS}" CACHE STRING "" FORCE)
  SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${PLATFORM_LINKFLAGS} ${NL_DEBUG_LINKFLAGS}" CACHE STRING "" FORCE)
  SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${PLATFORM_LINKFLAGS} ${NL_DEBUG_LINKFLAGS}" CACHE STRING "" FORCE)

  ## Release
  SET(CMAKE_C_FLAGS_RELEASE ${NL_RELEASE_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_CXX_FLAGS_RELEASE ${NL_RELEASE_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${PLATFORM_LINKFLAGS} ${NL_RELEASE_LINKFLAGS}" CACHE STRING "" FORCE)
  SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${PLATFORM_LINKFLAGS} ${NL_RELEASE_LINKFLAGS}" CACHE STRING "" FORCE)
  SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${PLATFORM_LINKFLAGS} ${NL_RELEASE_LINKFLAGS}" CACHE STRING "" FORCE)
ENDMACRO(NL_SETUP_BUILD_FLAGS)

# Macro to create x_ABSOLUTE_PREFIX from x_PREFIX
MACRO(NL_MAKE_ABSOLUTE_PREFIX NAME_RELATIVE NAME_ABSOLUTE)
  IF(IS_ABSOLUTE "${${NAME_RELATIVE}}")
    SET(${NAME_ABSOLUTE} ${${NAME_RELATIVE}})
  ELSE(IS_ABSOLUTE "${${{NAME_RELATIVE}}")
    IF(WIN32)
      SET(${NAME_ABSOLUTE} ${${NAME_RELATIVE}})
    ELSE(WIN32)
      SET(${NAME_ABSOLUTE} ${CMAKE_INSTALL_PREFIX}/${${NAME_RELATIVE}})
    ENDIF(WIN32)
  ENDIF(IS_ABSOLUTE "${${NAME_RELATIVE}}")
ENDMACRO(NL_MAKE_ABSOLUTE_PREFIX)

MACRO(NL_SETUP_PREFIX_PATHS)
  ## Allow override of install_prefix/etc path.
  IF(NOT NL_ETC_PREFIX)
    IF(WIN32)
      SET(NL_ETC_PREFIX "." CACHE PATH "Installation path for configurations")
    ELSE(WIN32)
      SET(NL_ETC_PREFIX "etc/nel" CACHE PATH "Installation path for configurations")
    ENDIF(WIN32)
  ENDIF(NOT NL_ETC_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(NL_ETC_PREFIX NL_ETC_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/share path.
  IF(NOT NL_SHARE_PREFIX)
    IF(WIN32)
      SET(NL_SHARE_PREFIX "." CACHE PATH "Installation path for data.")
    ELSE(WIN32)
      SET(NL_SHARE_PREFIX "share/nel" CACHE PATH "Installation path for data.")
    ENDIF(WIN32)
  ENDIF(NOT NL_SHARE_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(NL_SHARE_PREFIX NL_SHARE_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/sbin path.
  IF(NOT NL_SBIN_PREFIX)
    IF(WIN32)
      SET(NL_SBIN_PREFIX "." CACHE PATH "Installation path for admin tools and services.")
    ELSE(WIN32)
      SET(NL_SBIN_PREFIX "sbin" CACHE PATH "Installation path for admin tools and services.")
    ENDIF(WIN32)
  ENDIF(NOT NL_SBIN_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(NL_SBIN_PREFIX NL_SBIN_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/bin path.
  IF(NOT NL_BIN_PREFIX)
    IF(WIN32)
      SET(NL_BIN_PREFIX "." CACHE PATH "Installation path for tools and applications.")
    ELSE(WIN32)
      SET(NL_BIN_PREFIX "bin" CACHE PATH "Installation path for tools and applications.")
    ENDIF(WIN32)
  ENDIF(NOT NL_BIN_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(NL_BIN_PREFIX NL_BIN_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/lib path.
  IF(NOT NL_LIB_PREFIX)
    IF(CMAKE_LIBRARY_ARCHITECTURE)
      SET(NL_LIB_PREFIX "lib/${CMAKE_LIBRARY_ARCHITECTURE}" CACHE PATH "Installation path for libraries.")
    ELSE(CMAKE_LIBRARY_ARCHITECTURE)
      SET(NL_LIB_PREFIX "lib" CACHE PATH "Installation path for libraries.")
    ENDIF(CMAKE_LIBRARY_ARCHITECTURE)
  ENDIF(NOT NL_LIB_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(NL_LIB_PREFIX NL_LIB_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/lib path.
  IF(NOT NL_DRIVER_PREFIX)
    IF(WIN32)
      SET(NL_DRIVER_PREFIX "." CACHE PATH "Installation path for drivers.")
    ELSE(WIN32)
      IF(CMAKE_LIBRARY_ARCHITECTURE)
        SET(NL_DRIVER_PREFIX "lib/${CMAKE_LIBRARY_ARCHITECTURE}/nel" CACHE PATH "Installation path for drivers.")
      ELSE(CMAKE_LIBRARY_ARCHITECTURE)
        SET(NL_DRIVER_PREFIX "lib/nel" CACHE PATH "Installation path for drivers.")
      ENDIF(CMAKE_LIBRARY_ARCHITECTURE)
    ENDIF(WIN32)
  ENDIF(NOT NL_DRIVER_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(NL_DRIVER_PREFIX NL_DRIVER_ABSOLUTE_PREFIX)

ENDMACRO(NL_SETUP_PREFIX_PATHS)

MACRO(RYZOM_SETUP_PREFIX_PATHS)
  ## Allow override of install_prefix/etc path.
  IF(NOT RYZOM_ETC_PREFIX)
    IF(WIN32)
      SET(RYZOM_ETC_PREFIX "." CACHE PATH "Installation path for configurations")
    ELSE(WIN32)
      SET(RYZOM_ETC_PREFIX "etc/ryzom" CACHE PATH "Installation path for configurations")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_ETC_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_ETC_PREFIX RYZOM_ETC_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/share path.
  IF(NOT RYZOM_SHARE_PREFIX)
    IF(WIN32)
      SET(RYZOM_SHARE_PREFIX "." CACHE PATH "Installation path for data.")
    ELSE(WIN32)
      SET(RYZOM_SHARE_PREFIX "share/ryzom" CACHE PATH "Installation path for data.")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_SHARE_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_SHARE_PREFIX RYZOM_SHARE_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/sbin path.
  IF(NOT RYZOM_SBIN_PREFIX)
    IF(WIN32)
      SET(RYZOM_SBIN_PREFIX "." CACHE PATH "Installation path for admin tools and services.")
    ELSE(WIN32)
      SET(RYZOM_SBIN_PREFIX "sbin" CACHE PATH "Installation path for admin tools and services.")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_SBIN_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_SBIN_PREFIX RYZOM_SBIN_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/bin path.
  IF(NOT RYZOM_BIN_PREFIX)
    IF(WIN32)
      SET(RYZOM_BIN_PREFIX "." CACHE PATH "Installation path for tools and applications.")
    ELSE(WIN32)
      SET(RYZOM_BIN_PREFIX "bin" CACHE PATH "Installation path for tools.")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_BIN_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_BIN_PREFIX RYZOM_BIN_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/lib path.
  IF(NOT RYZOM_LIB_PREFIX)
    IF(CMAKE_LIBRARY_ARCHITECTURE)
      SET(RYZOM_LIB_PREFIX "lib/${CMAKE_LIBRARY_ARCHITECTURE}" CACHE PATH "Installation path for libraries.")
    ELSE(CMAKE_LIBRARY_ARCHITECTURE)
      SET(RYZOM_LIB_PREFIX "lib" CACHE PATH "Installation path for libraries.")
    ENDIF(CMAKE_LIBRARY_ARCHITECTURE)
  ENDIF(NOT RYZOM_LIB_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_LIB_PREFIX RYZOM_LIB_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/games path.
  IF(NOT RYZOM_GAMES_PREFIX)
    IF(WIN32)
      SET(RYZOM_GAMES_PREFIX "." CACHE PATH "Installation path for tools and applications.")
    ELSE(WIN32)
      SET(RYZOM_GAMES_PREFIX "games" CACHE PATH "Installation path for client.")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_GAMES_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_GAMES_PREFIX RYZOM_GAMES_ABSOLUTE_PREFIX)

ENDMACRO(RYZOM_SETUP_PREFIX_PATHS)

MACRO(SETUP_EXTERNAL)
  IF(WITH_EXTERNAL)
    FIND_PACKAGE(External REQUIRED)
  ENDIF(WITH_EXTERNAL)

  IF(WIN32)
    FIND_PACKAGE(External REQUIRED)

    IF(MSVC10)
      IF(NOT MSVC10_REDIST_DIR)
        # If you have VC++ 2010 Express, put x64/Microsoft.VC100.CRT/*.dll in ${EXTERNAL_PATH}/redist
        SET(MSVC10_REDIST_DIR "${EXTERNAL_PATH}/redist")
      ENDIF(NOT MSVC10_REDIST_DIR)

      IF(NOT VC_DIR)
        IF(NOT VC_ROOT_DIR)
          GET_FILENAME_COMPONENT(VC_ROOT_DIR "[HKEY_CURRENT_USER\\Software\\Microsoft\\VisualStudio\\10.0_Config;InstallDir]" ABSOLUTE)
          # VC_ROOT_DIR is set to "registry" when a key is not found
          IF(VC_ROOT_DIR MATCHES "registry")
            GET_FILENAME_COMPONENT(VC_ROOT_DIR "[HKEY_CURRENT_USER\\Software\\Microsoft\\VCExpress\\10.0_Config;InstallDir]" ABSOLUTE)
            IF(VC_ROOT_DIR MATCHES "registry")
              FILE(TO_CMAKE_PATH $ENV{VS100COMNTOOLS} VC_ROOT_DIR)
              IF(NOT VC_ROOT_DIR)
                MESSAGE(FATAL_ERROR "Unable to find VC++ 2010 directory!")
              ENDIF(NOT VC_ROOT_DIR)
            ENDIF(VC_ROOT_DIR MATCHES "registry")
          ENDIF(VC_ROOT_DIR MATCHES "registry")
        ENDIF(NOT VC_ROOT_DIR)
        # convert IDE fullpath to VC++ path
        STRING(REGEX REPLACE "Common7/.*" "VC" VC_DIR ${VC_ROOT_DIR})
      ENDIF(NOT VC_DIR)
    ELSE(MSVC10)
      IF(NOT VC_DIR)
        IF(${CMAKE_MAKE_PROGRAM} MATCHES "Common7")
          # convert IDE fullpath to VC++ path
          STRING(REGEX REPLACE "Common7/.*" "VC" VC_DIR ${CMAKE_MAKE_PROGRAM})
        ELSE(${CMAKE_MAKE_PROGRAM} MATCHES "Common7")
          # convert compiler fullpath to VC++ path
          STRING(REGEX REPLACE "VC/bin/.+" "VC" VC_DIR ${CMAKE_CXX_COMPILER})
        ENDIF(${CMAKE_MAKE_PROGRAM} MATCHES "Common7")
      ENDIF(NOT VC_DIR)
    ENDIF(MSVC10)
  ELSE(WIN32)
    IF(APPLE)
      IF(WITH_STATIC_EXTERNAL)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .a)
      ELSE(WITH_STATIC_EXTERNAL)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .dylib .so .a)
      ENDIF(WITH_STATIC_EXTERNAL)
    ELSE(APPLE)
      IF(WITH_STATIC_EXTERNAL)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .a .so)
      ELSE(WITH_STATIC_EXTERNAL)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .so .a)
      ENDIF(WITH_STATIC_EXTERNAL)
    ENDIF(APPLE)
  ENDIF(WIN32)

  IF(WITH_STLPORT)
    FIND_PACKAGE(STLport REQUIRED)
    INCLUDE_DIRECTORIES(${STLPORT_INCLUDE_DIR})
    IF(MSVC)
      SET(VC_INCLUDE_DIR "${VC_DIR}/include")

      FIND_PACKAGE(WindowsSDK REQUIRED)
      # use VC++ and Windows SDK include paths
      INCLUDE_DIRECTORIES(${VC_INCLUDE_DIR} ${WINSDK_INCLUDE_DIR})
    ENDIF(MSVC)
  ENDIF(WITH_STLPORT)
ENDMACRO(SETUP_EXTERNAL)
