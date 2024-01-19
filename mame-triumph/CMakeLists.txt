#  done for old triumph amiga mame.

cmake_minimum_required (VERSION 3.1)

option(TINY_COMPILE "smaller compile including only one driver" OFF)
option(NEOFREE "do not compile neo" ON)
option(NEOMAME "compile neo" OFF)


set(CXX_STANDARD 11)
set(C_STANDARD 11)
project(AmigaTriumphMame LANGUAGES C CXX VERSION 1.0)

if(AMIGA)

	add_definitions(-O2 -m68060 -mhard-float -ffast-math -save-temps )
	add_definitions(-std=c11)
	
endif()

add_subdirectory(src/mame037b MameLib)


set(AMIGA_SRC
	src/amiga/amiga037b.c
	src/amiga/amiga037b_joysticks.c
	src/amiga/main.c
#re	src/amiga/gui_mui.c
	src/amiga/config.c
	src/amiga/file.c
	src/amiga/audio.c
	src/amiga/video.c
	src/amiga/amiga_inputs.c

	src/amiga/main.h
	src/amiga/config.h
	src/amiga/file.h
	src/amiga/audio.h
	src/amiga/video.h
	src/amiga/amiga_inputs.h
	src/amiga/gui.h
	src/amiga/mame_ver.h
	src/amiga/mame_msg.h
	)

set(PROGNAME TriumphMame)
add_executable(${PROGNAME} ${AMIGA_SRC} )
set_property(TARGET ${PROGNAME} PROPERTY C_STANDARD 11)
target_compile_definitions(${PROGNAME} PUBLIC MAMEVERSION=37 )
target_link_libraries(${PROGNAME} PUBLIC mamecore)
target_include_directories(${PROGNAME} PUBLIC "include" "src/mame037b/src")

# include_directories(src/mame037b/src)
## generate zconf.h
#configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/../zlib/zconf.h.cmakein
#		${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)
#include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

