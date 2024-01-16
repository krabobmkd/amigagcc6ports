cmake_minimum_required (VERSION 3.3.2)

project (libde265
    LANGUAGES C CXX
    VERSION 1.0.15
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(AMIGA)
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
else()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()
# The version number.
set (NUMERIC_VERSION 0x01001500)
set (PACKAGE_VERSION ${PROJECT_VERSION})

#include (${CMAKE_ROOT}/Modules/CheckCCompilerFlag.cmake)
#include (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)

include(GNUInstallDirs)
include(CheckFunctionExists)


#Vf-Amiga
if(AMIGA)
	message(ERROR "woooo")
	# turn SDL examples as off
	option(ENABLE_SDL "Enable SDL" OFF)
	add_definitions(-O2 -m68060 -mhard-float -ffast-math -save-temps)
	add_definitions(-DHAVE_ALLOCA_H=1)
else()
	# vf test
	#add_definitions(-funsigned-char -ansi)

	option(ENABLE_SDL "Enable SDL" ON)
endif()


if (ENABLE_SDL)
	find_package(SDL2)
endif()

#VF-Amiga remove  find_package(Threads REQUIRED)
#find_package(Threads REQUIRED)

#CHECK_INCLUDE_FILE(malloc.h HAVE_MALLOC_H)
#CHECK_FUNCTION_EXISTS(posix_memalign HAVE_POSIX_MEMALIGN)

if (HAVE_MALLOC_H)
  add_definitions(-DHAVE_MALLOC_H)
endif()
if (HAVE_POSIX_MEMALIGN)
  add_definitions(-DHAVE_POSIX_MEMALIGN)
endif()

configure_file (libde265/de265-version.h.in libde265/de265-version.h)

if(CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  add_definitions(-Wall)
endif()


#include(CheckCXXSymbolExists)
#check_cxx_symbol_exists(_LIBCPP_VERSION cstdlib HAVE_LIBCPP)

#if(HAVE_LIBCPP)
#  set(LIBS_PRIVATE "-lc++")
#else()
  set(LIBS_PRIVATE "-lstdc++")
#endif()

option(BUILD_SHARED_LIBS "Build shared library" OFF)
if(NOT BUILD_SHARED_LIBS)
  add_definitions(-DLIBDE265_STATIC_BUILD)
endif()

include_directories ("${PROJECT_SOURCE_DIR}")
include_directories ("${PROJECT_BINARY_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}/libde265")
if(MSVC)
  include_directories ("${PROJECT_SOURCE_DIR}/extra")
endif()

option(ENABLE_DECODER "Enable Decoder" ON)
option(ENABLE_ENCODER "Enable Encoder" OFF)

add_subdirectory (libde265)
if (ENABLE_DECODER)
  add_subdirectory (dec265)
endif()
if (ENABLE_ENCODER)
  add_subdirectory (enc265)
endif()
