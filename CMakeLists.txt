# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang         <@liangbin>
#
# Summary: Top level CMake listfile for zeta project

# ----------------------------------------------------------------------------
# Specify the version being used as well as the language
cmake_minimum_required (VERSION 3.17)

# ----------------------------------------------------------------------------
# Make sure we are building out-of-source
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "DO NOT BUILD in-tree.")
endif()
message(STATUS "Top level CMakeLists.txt")

# ----------------------------------------------------------------------------
# Set a default build type if none was specified BEFORE project() command
# The simplest way to set the optimization level and debug symbol inclusion for the builds
# Use -DCMAKE_BUILD_TYPE=Release CLI option to override default set here
# CMAKE_BUILD_TYPE=Debug              # defaults to -g
# CMAKE_BUILD_TYPE=Release            # defaults to -O3 -DNDEBUG
# CMAKE_BUILD_TYPE=RelWithDebInfo     # defaults to -O2 -g
# CMAKE_BUILD_TYPE=MinSizeRel         # defaults to -Os -DNDEBUG
set(default_build_type "Debug")
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Release")
endif()
 
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# ----------------------------------------------------------------------------
# Set project as early as possible, but after cmake_minimal_required
# Including package information
set (PACKAGE_NAME      "zeta")
set (PACKAGE_VERSION   "0.0.1")
set (PACKAGE_STRING    "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set (PACKAGE_TARNAME   "${PACKAGE_NAME}-${PACKAGE_VERSION}")
set (PACKAGE_BUGREPORT "https://github.com/futurewei-cloud/zeta/issues")
message(STATUS "Start constructing project " ${PACKAGE_NAME} " Version:" ${PACKAGE_VERSION})
project (${PACKAGE_NAME} VERSION ${PACKAGE_VERSION} LANGUAGES C)

# ----------------------------------------------------------------------------
# includes
#set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#include (utils)

# ----------------------------------------------------------------------------
# Boolean options
option (BUILD_SHARED_LIBS          "Request build of shared libraries."                                       OFF)
option (BUILD_STATIC_LIBS          "Request build of static libraries (default if BUILD_SHARED_LIBS is OFF)." OFF)
option (BUILD_PACKAGING            "Enable build of distribution packages using CPack."                       OFF)
option (BUILD_TESTING              "Enable build of the unit tests and their execution using CTest."          OFF)
option (INSTALL_HEADERS            "Request packaging of headers and other development files."                ON)
if (NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
  set (BUILD_STATIC_LIBS ON)
endif ()

# ----------------------------------------------------------------------------
# Set default architecture [x86_64|aarch64], override by -DARCH cli option
set(ARCH "x86_64" CACHE STRING "Architecture to tell gcc to optimize for (-march)")

# ----------------------------------------------------------------------------
# Set debug options for compiler
# Options for debug build, -O -g already set through CMAKE_BUILD_TYPE=Debug
set(SANITIZE.x86_64 "")
set(SANITIZE.aarch64 "")
set(SANITIZE "${SANITIZE.${ARCH}}-fsanitize=leak \
  -fsanitize=undefined \
  -fsanitize=shift \
  -fsanitize=integer-divide-by-zero \
  -fsanitize=unreachable \
  -fsanitize=vla-bound \
  -fsanitize=null \
  -fsanitize=return \
  -fsanitize=signed-integer-overflow \
  -fsanitize=bounds \
  -fsanitize=alignment \
  -fsanitize=object-size \
  -fsanitize=float-divide-by-zero \
  -fsanitize=float-cast-overflow \
  -fsanitize=nonnull-attribute \
  -fsanitize=returns-nonnull-attribute \
  -fsanitize=bool \
  -fsanitize=enum \
  -fsanitize=vptr \
  -fno-omit-frame-pointer")
#set(C_DBG_FLAGS "-fprofile-arcs -ftest-coverage ${SANITIZE}")
set(C_DBG_FLAGS "${SANITIZE} -ggdb3")
set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} ${C_DBG_FLAGS}")

# ----------------------------------------------------------------------------
# Set non-debug common compiler options
set(C_FLAGS "-I. \
  -I${CMAKE_SOURCE_DIR}/src \
  -I${CMAKE_SOURCE_DIR}/src/include \
  -I${CMAKE_BINARY_DIR}/lib/usr/include \
  -std=c11 \
  -D_POSIX_C_SOURCE=199309L \
  -Wall \
  -Wextra \
  -Werror \
  -pedantic -Wpedantic \
  -Wno-cast-function-type -Wno-error=cast-function-type \
  -fno-common \
  -fstrict-aliasing")
set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} ${C_FLAGS}")

# ----------------------------------------------------------------------------
# Set SANITIZER debug options for linker
set(LD_DBG_FLAGS "-static-liblsan -static-libubsan")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${LD_DBG_FLAGS}")

# ----------------------------------------------------------------------------
# Set common linker options
set(LD_FLAGS.x86_64 -L${CMAKE_BINARY_DIR}/lib/usr/lib64)
set(LD_FLAGS.aarch64 -L${CMAKE_BINARY_DIR}/lib/usr/lib)
set(LD_FLAGS "${LD_FLAGS.${ARCH}}")
set(CMAKE_EXE_LINKER_FLAGS       "${CMAKE_EXE_LINKER_FLAGS} ${LD_FLAGS}")

# ----------------------------------------------------------------------------
# Enable Unit test if we are NOT Inside other project 
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()

add_subdirectory(src)
