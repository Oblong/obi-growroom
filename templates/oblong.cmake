# Boilerplate cmake options and compiler flags for projects using g-speak
# Copyright (C) Oblong Industries
# Before including this, set the following variable:
#   PLATFORM_LIBS    - g-speak or system libraries to link to
#
# Defines cmake commandline options settable with -Dfoo=bar:
#   CMAKE_BUILD_TYPE - Debug, RelWithDebInfo, Release
#   ASAN             - whether to use address sanitizer
#   COVERAGE         - whether to use code coverage
#   TSAN             - whether to use thread sanitizer
#   BUILD_TESTS      - whether to build tests
# and sets the variables
#   G_SPEAK_CFLAGS   - compiler flags for PLATFORM_LIBS
#   G_SPEAK_LDFLAGS  - compiler flags for PLATFORM_LIBS when linking
#   OPTIMIZE_FLAGS   - compiler flags for CMAKE_BUILD_TYPE
#   COVERAGE_FLAGS   - compiler flags for COVERAGE
#   SANITIZER_FLAGS  - compiler flags for ASAN/TSAN
#   G_SPEAK_HOME     - where g-speak is installed
#   YOVERSION        - which version of yobuild g-speak uses
#   G_SPEAK_YOBUILD_HOME - where yobuild is installed
# as well as G_SPEAK_STATIC_(LDFLAGS,CFLAGS,INCLUDE_PATHS) for when
# you want to link statically to g-speak.
# Defines the helper function:
#   oblong_append_supported_cxx_flags(RESULTVAR flag ...)
#
# If libWebThing is in PLATFORM_LIBS, it also:
# - sets CEF_BRANCH
# - defines functions needed for building and installing webthing apps:
#   cef_bless_app()
#   cef_install_blessed_app()
#
# See the files it includes for more detail:
#   ${G_SPEAK_HOME}/lib/cmake/FindGSpeak.cmake
#   ${G_SPEAK_YOBUILD_HOME}/lib/cmake/FindCEF${CEF_BRANCH}.cmake
#
# NOTE: There are two ways to change which g-speak most projects use:
# 1. Best, but only handles g-speak installed at the standard location:
#      ob-set-defaults --g-speak 3.22
#    This edits debian/rules; this code below reads settings from that file.
#    It also may rename files in debian/* that have the g-speak version
#    in the filename.
# 2. Like #1, but doesn't support building packages: run cmake with
#      -DG_SPEAK_HOME=/opt/oblong/g-speak3.22

#---------- Options settable from cmake commandline with -Dfoo=bar ---------

option(BUILD_TESTS     "Build tests, too"                          ON)
option(COVERAGE        "Generate coverage report after test run"   OFF)
option(ASAN            "Enable Address Sanitizer"                  OFF)
option(TSAN            "Enable Thread Sanitizer (experimental)"    OFF)
option(CMAKE_VERBOSE_MAKEFILE "Verbose output from make"           OFF)

# This is the amazingly complicated idiom for providing a default value for cache variables set from the commandline with -D
# (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT makes this easier in just one special case, but general case is harder.)
IF(DEFINED CMAKE_BUILD_TYPE AND (NOT ${CMAKE_BUILD_TYPE} STREQUAL "None"))
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo")
ELSE()
   SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo" FORCE)
ENDIF()

IF(DEFINED G_SPEAK_HOME AND (NOT ${G_SPEAK_HOME} STREQUAL "None"))
   SET(G_SPEAK_HOME ${G_SPEAK_HOME} CACHE STRING "Choose where g-speak was installed")
ELSE()
   # Default to value specified in debian/rules, set earlier by ob-set-defaults
   execute_process(COMMAND awk "-F=" "/^G_SPEAK_HOME=/ { print $2 }"
                   INPUT_FILE "${PROJECT_SOURCE_DIR}/debian/rules"
                   OUTPUT_VARIABLE G_SPEAK_HOME
                   OUTPUT_STRIP_TRAILING_WHITESPACE)
   SET(G_SPEAK_HOME "${G_SPEAK_HOME}" CACHE STRING "Choose where g-speak was installed" FORCE)
ENDIF()

# If no install prefix set, default to G_SPEAK_HOME
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   SET(CMAKE_INSTALL_PREFIX ${G_SPEAK_HOME} CACHE PATH "Install prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#---------- Dependencies and Compiler Flags ---------------

include(CheckCXXCompilerFlag)

function(oblong_append_supported_cxx_flags RESULTVAR)
    foreach(flag ${ARGN})
       string(REGEX REPLACE "[^a-zA-Z0-9_]" _ SANITIZED_FLAG ${flag})
       set(FLAGFLAG "HAS_CXXFLAG${SANITIZED_FLAG}")
       check_cxx_compiler_flag(${flag} ${FLAGFLAG})
       if (${${FLAGFLAG}})
          set(${RESULTVAR} "${${RESULTVAR}} ${flag}" PARENT_SCOPE)
          set(${RESULTVAR} "${${RESULTVAR}} ${flag}")
       endif()
    endforeach()
endfunction()

# Calling FindGSpeak with a set of pkgconfig library names
# adds the needed directories to the include path, and sets the variables
#   G_SPEAK_LDFLAGS
#   G_SPEAK_CFLAGS
#   G_SPEAK_YOBUILD_HOME
# If you specified libWebThing, it also sets CEF_BRANCH.
INCLUDE("${G_SPEAK_HOME}/lib/cmake/FindGSpeak.cmake")

FindGSpeak(${PLATFORM_LIBS})

# If you specified libWebThing (or set CEF_BRANCH yourself), grab cef helper functions
if (NOT "${CEF_BRANCH}" STREQUAL "")
  # Following line only needed for projects that use webthing or cef; it defines
  # the functions cef_bless_app and cef_install_blessed_app.
  INCLUDE("${G_SPEAK_YOBUILD_HOME}/lib/cmake/FindCEF${CEF_BRANCH}.cmake")
endif()

if (APPLE)
    # FIXME: insulate ourselves from packages that use -pthread on osx
    STRING(REPLACE "-pthread" "" G_SPEAK_CFLAGS "${G_SPEAK_CFLAGS}")
endif()

set(OPTIMIZE_FLAGS )
if (NOT WIN32)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
       set(OPTIMIZE_FLAGS "-g0 -O3 -DNDEBUG")
    elseif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
       if (TSAN OR ASAN)
           # Sanitizers have overhead, compensate with a little optimization
           set(OPTIMIZE_FLAGS "-g -O1 -UNDEBUG")
       else()
           set(OPTIMIZE_FLAGS "-g -O0 -UNDEBUG")
       endif()
    elseif (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
       set(OPTIMIZE_FLAGS "-g -O3 -UNDEBUG")
    else()
       message(FATAL_ERROR "Unknown build type ${CMAKE_BUILD_TYPE}")
    endif()
endif()

set(SANITIZER_FLAGS "")
if (ASAN AND TSAN)
  message(FATAL "ASAN and TSAN are mutually exclusive.")
endif()
if (ASAN)
  set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
elseif (TSAN)
  set(SANITIZER_FLAGS "-fsanitize=thread -fno-omit-frame-pointer")
endif()

SET(COVERAGE_FLAGS "")
if (COVERAGE)
  FIND_PROGRAM( GCOV gcov )
  IF (NOT GCOV)
    MESSAGE(FATAL_ERROR "COVERAGE selected, but gcov not found (part of compiler")
  ENDIF()
  FIND_PROGRAM( LCOV lcov )
  IF (NOT LCOV)
    MESSAGE(FATAL_ERROR "COVERAGE selected, but lcov not found (package lcov)")
  ENDIF()
  SET(COVERAGE_FLAGS "--coverage")
endif ()

# Kludge: oblong's boost doesn't know where it lives, so turn off autolinking...
add_definitions("-DBOOST_ALL_NO_LIB")

# Kludge: on Windows, add g-speak install directories to linker search path
if (WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "/LIBPATH:\"${G_SPEAK_YOBUILD_HOME}/binaries\";\"${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_BUILD_TYPE}\"")
  set(CMAKE_SHARED_LINKER_FLAGS "/LIBPATH:\"${G_SPEAK_YOBUILD_HOME}/binaries\"")
endif()
