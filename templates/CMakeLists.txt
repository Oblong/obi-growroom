cmake_minimum_required (VERSION 2.8.7)
project({{project_name}} CXX)

# This is the amazingly complicated idiom for providing a default value for cache variables set from the commandline with -D
# (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT makes this easier in just one special case, but general case is harder.)
IF(DEFINED CMAKE_BUILD_TYPE AND (NOT ${CMAKE_BUILD_TYPE} STREQUAL "None"))
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo")
ELSE()
   SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo" FORCE)
ENDIF()


# The list of g-speak libraries this app needs (careful, this is case sensitive).
# It will pull in any other g-speak libraries needed by the ones you specify.
# Actually, any pkgconfig library from g-speak, yobuild, or the system may be listed here.
set(PLATFORM_LIBS libAfferent libTwillig libGreenhouse libWebThing libStaging libGrowroom)

# NOTE: To change which g-speak this project uses, edit g-speak.dat to say e.g.
#   G_SPEAK_HOME=/opt/oblong/g-speak3.22
# Be careful to not add spaces.
# You can also use -DG_SPEAK_HOME=/opt/oblong/g-speak3.22 on cmake commandline.
# TIP: If you're building debian packages, run
#   sh set-gspeak.sh 3.22
# instead.
IF(DEFINED G_SPEAK_HOME AND (NOT ${G_SPEAK_HOME} STREQUAL "None"))
   SET(G_SPEAK_HOME ${G_SPEAK_HOME} CACHE STRING "Choose where g-speak was installed")
ELSE()
   # But default to value specified in debian/rules, set by set-gspeak.sh
   execute_process(COMMAND awk "-F=" "/G_SPEAK_HOME=/ { print $2 }"
		   INPUT_FILE "${PROJECT_SOURCE_DIR}/g-speak.dat"
		   OUTPUT_VARIABLE G_SPEAK_HOME
		   OUTPUT_STRIP_TRAILING_WHITESPACE
		   )
   SET(G_SPEAK_HOME "${G_SPEAK_HOME}" CACHE STRING "Choose where g-speak was installed" FORCE)
ENDIF()

# Calling FindGSpeak with a set of pkgconfig library names
# adds the needed directories to the include path, and sets the variables
#   G_SPEAK_LDFLAGS
#   G_SPEAK_CFLAGS
#   G_SPEAK_YOBUILD_HOME
# If you specified libWebThing, it also sets CEF_BRANCH.
INCLUDE("${G_SPEAK_HOME}/lib/cmake/FindGSpeak.cmake")
FindGSpeak(${PLATFORM_LIBS})
# Following line only needed for projects that use cef; it defines
# the functions cef_bless_app and cef_install_blessed_app.
INCLUDE("${G_SPEAK_YOBUILD_HOME}/lib/cmake/FindCEF${CEF_BRANCH}.cmake")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${G_SPEAK_CFLAGS}")

#
# Debugging Options
#
set (CMAKE_VERBOSE_MAKEFILE 0) # Use 1 for debugging, 0 for release

#
# Ensure C++11 support
#
set(CMAKE_CXX_STANDARD 11)
# Abort if C++11 not available
set(CXX_STANDARD_REQUIRED YES)
# Deselect vendor extensions to avoid compiler lock-in
set(CMAKE_CXX_EXTENSIONS OFF)

#
# Compiler flags
#
include(CheckCXXCompilerFlag)

# Common compiler flags for Windows, OSX and Linux
if (WIN32)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CRT_SECURE_NO_WARNINGS")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
  # Developers will probably prefer to use #pragma's in a .h file to
  # suppress most warnings... but some warnings don't suppress that way,
  # so do them here if needed.
  #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")
else ()
  # Flags commonly used on Linux and Mac.  Adjust to taste.
  FOREACH(FLAG
        -fdiagnostics-show-option
        -Qunused-arguments
        -Wall
        -Wempty-body
        -Werror
        -Wno-error=unused-variable
        -Wno-overloaded-virtual
        -Wno-parentheses
        -Wno-strict-overflow
        -Wno-unused-but-set-variable
        -Wno-unused-command-line-argument
        -Wpointer-arith
        -Wshadow
        -Wtype-limits
        -Wvla
        -Wwrite-strings
    )
    string(REGEX REPLACE "[^a-zA-Z0-9_]" _ SANITIZED_FLAG ${FLAG})
    CHECK_CXX_COMPILER_FLAG(${FLAG} COMPILER_SUPPORTS_${SANITIZED_FLAG})
    IF(COMPILER_SUPPORTS_${SANITIZED_FLAG})
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
    ENDIF()
  ENDFOREACH()
endif ()

#
# Growroom binding generation
#

# Add more YAML files here as you please.
set (BINDING_NAME {{project_name_symbol}})
set (GSPEAK_JS_BINDING_DEFNS
  ${BINDING_NAME}.yaml
)

set (GROWROOM_BINDING_GENERATOR "${G_SPEAK_HOME}/bin/gen-growroom-code.py")
set (GROWROOM_BINDING_DEFNS_DIR "${G_SPEAK_HOME}/src/growroom/doc")
set (GROWROOM_TEMPLATES_DIR "${G_SPEAK_HOME}/src/growroom/templates")
set (GSPEAK_JS_BINDING_SOURCE "${PROJECT_SOURCE_DIR}/src/generated/${BINDING_NAME}_binding.cpp")
set (GSPEAK_JS_BINDING_HEADER "${PROJECT_SOURCE_DIR}/src/generated/${BINDING_NAME}_binding.h")

set (GSPEAK_JS_BINDING_INVOCATION
  python ${GROWROOM_BINDING_GENERATOR}
  -e ${GROWROOM_BINDING_DEFNS_DIR} -t ${GROWROOM_TEMPLATES_DIR}
  -n ${BINDING_NAME}_binding -o src/generated ${GSPEAK_JS_BINDING_DEFNS})

add_custom_command (
  OUTPUT ${GSPEAK_JS_BINDING_SOURCE} ${GSPEAK_JS_BINDING_HEADER}
  COMMAND ${GSPEAK_JS_BINDING_INVOCATION}
  DEPENDS ${GSPEAK_JS_BINDING_DEFNS} ${GROWROOM_BINDING_GENERATOR} ${GROWROOM_TEMPLATES}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  VERBATIM
  COMMENT "Generate ${PROJECT_NAME} libGrowroom bindings")

#
# Project Search Paths
#
SET (project_source_files
  src/main.cpp
  src/PushHandler.cpp
  src/Swiveler.cpp
  ${GSPEAK_JS_BINDING_SOURCE}
  )
include_directories("include" "src")

# Finally, build the app!
add_executable({{project_name}} ${project_source_files})
target_link_libraries({{project_name}} ${G_SPEAK_LDFLAGS})

# Apps that use cef need this to work around awkwardness about
# how CEF finds its data files:
cef_bless_app({{project_name}})

# And, if user does 'make install', install it.
# We'd normally do
#INSTALL(TARGETS {{project_name}} RUNTIME DESTINATION "bin")
# but apps that use cef have to do this instead:
cef_install_blessed_app({{project_name}})
