#  done for old triumph amiga mame.

cmake_minimum_required (VERSION 3.1)

option(TINY_COMPILE "smaller compile including only one driver" OFF)
option(NEOFREE "do not compile neo" ON)
option(NEOMAME "compile neo" OFF)

#SET(ASM_OPTIONS "-x assembler-with-cpp")
#SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}" )

set(CXX_STANDARD 11)
set(C_STANDARD 11)
project(AmigaTriumphMame LANGUAGES C CXX ASM VERSION 1.0)

if(AMIGA)

	add_definitions(-O2 -m68060 -mhard-float -ffast-math -save-temps )
	
endif()

add_subdirectory(src/mame037b MameLib)


set(AMIGA_SRC
	src/amiga/amiga037b.cpp
	src/amiga/amiga037b_keyboard.cpp
	src/amiga/amiga037b_joysticks.cpp
	src/amiga/amiga037b_audiostream.cpp
	src/amiga/amiga037b_filesystem.cpp
	src/amiga/main.cpp
	src/amiga/gui_mui.cpp
	src/amiga/config.cpp
	src/amiga/file.cpp
	src/amiga/audio.cpp
	src/amiga/video.cpp
	src/amiga/amiga_inputs.cpp
	src/amiga/amiga_locale.cpp
	src/amiga/main.h
	src/amiga/config.h
	src/amiga/file.h
	src/amiga/audio.h
	src/amiga/video.h
	src/amiga/amiga_inputs.h
	src/amiga/amiga_locale.h
	src/amiga/gui.h
	src/amiga/mame_ver.h
	src/amiga/mame_msg.h

	src/amiga/c2p.s
	)

set(PROGNAME TriumphMame)
add_executable(${PROGNAME} ${AMIGA_SRC} )
set_property(TARGET ${PROGNAME} PROPERTY C_STANDARD 11)
target_compile_definitions(${PROGNAME} PUBLIC MAMEVERSION=37 )

target_link_directories(${PROGNAME} PUBLIC "lib")
# the static libs that are part of this repo: 
target_link_libraries(${PROGNAME} PUBLIC mamecore z)
# the extra original static libs furnished by triumph,"moo db" is a database used for config:
target_link_libraries(${PROGNAME} PUBLIC moo mood cgxhooks)

target_include_directories(${PROGNAME} PUBLIC "include" )
target_include_directories(${PROGNAME} PUBLIC "src/mame037b/src")
target_include_directories(${PROGNAME} PUBLIC "../extrasdk/MUI/include")

# include_directories(src/mame037b/src)
## generate zconf.h
#configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/../zlib/zconf.h.cmakein
#		${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)
#include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

