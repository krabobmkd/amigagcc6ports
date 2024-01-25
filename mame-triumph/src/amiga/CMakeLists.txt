# CMakelists.txt done to refresh old 1999 triumph amiga mame.

cmake_minimum_required (VERSION 3.1)

option(TINY_COMPILE "smaller compile including only one driver" OFF)
option(NEOFREE "do not compile neo" ON)
option(NEOMAME "compile neo" OFF)

#SET(ASM_OPTIONS "-x assembler-with-cpp")
#SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}" )

set(CXX_STANDARD 11)
set(C_STANDARD 11)
project(AmigaTriumphMame LANGUAGES C CXX VERSION 1.0)

if(AMIGA)

	add_definitions(-O2 -m68060 -mhard-float -ffast-math -save-temps -fno-rtti )
	
endif()

add_subdirectory(../mame037b MameLib)


set(AMIGA_SRC
	amiga037b.cpp
	amiga037b_keyboard.cpp
	amiga037b_joysticks.cpp
	amiga037b_audiostream.cpp
	amiga037b_filesystem.cpp
	main.cpp
	gui_mui.cpp
	config.cpp
	file.cpp
	audio.cpp
	video.cpp
	amiga_inputs.cpp
	amiga_locale.cpp
	main.h
	config.h
	file.h
	audio.h
	video.h
	amiga_inputs.h
	amiga_locale.h
	gui.h
	mame_ver.h
	mame_msg.h

#	c2p.s
	)

set(PROGNAME TriumphMame)
add_executable(${PROGNAME} ${AMIGA_SRC} )
set_property(TARGET ${PROGNAME} PROPERTY C_STANDARD 11)
target_compile_definitions(${PROGNAME} PUBLIC MAMEVERSION=37 )

# the static libs that are part of this repo: 
target_link_libraries(${PROGNAME} PUBLIC mamecore z)

# the extra original static libs furnished by triumph,"moo db" is a database used for config:
target_link_directories(${PROGNAME} PUBLIC "../../lib")
target_include_directories(${PROGNAME} PUBLIC "../../include" )
target_link_libraries(${PROGNAME} PUBLIC moo mood cgxhooks)

target_include_directories(${PROGNAME} PUBLIC "../mame037b/src")

# MUI is not provided by gccbebbo6.5 ndk by default, we have it there:
target_include_directories(${PROGNAME} PUBLIC "../../../extrasdk/MUI/include")

# include_directories(src/mame037b/src)
## generate zconf.h
#configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/../zlib/zconf.h.cmakein
#		${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)
#include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

