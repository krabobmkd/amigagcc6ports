# CMakelists.txt done to refresh old 1999 triumph amiga mame.

cmake_minimum_required (VERSION 3.1)

set(CXX_STANDARD 11)
set(C_STANDARD 11)
project(AmigaTriumphMame LANGUAGES C CXX VERSION 1.0)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(AMIGA)
#  -fno-rtti -fno-exceptions -fno-strict-aliasing
	add_definitions( -m68060 -mhard-float -ffast-math -save-temps )
	
endif()

add_subdirectory(../mame037b/src MameLib)


set(AMIGA_SRC
	amiga037b.cpp
	amiga037b_keyboard.cpp
	amiga037b_joysticks.cpp
	amiga037b_audiostream.cpp
	amiga037b_filesystem.cpp
	main.cpp
#	romscan.cpp
	gui_mui.cpp
	gui_gadtools.cpp
	config_moo.cpp
	file.cpp
	audio.cpp
	video.cpp
	amiga_inputs.cpp
	amiga_locale.cpp
	main.h
	config_moo.h
	file.h
	audio.h
	video.h
	amiga_inputs.h
	amiga_locale.h
	gui_mui.h
	gui_gadtools.h
	mame_ver.h
	mame_msg.h

	../../lib/c2p.s.o ../../lib/gfx.s.o
#	c2p.s
	)

set(PROGNAME TriumphMame)
add_executable(${PROGNAME} ${AMIGA_SRC} )
set_property(TARGET ${PROGNAME} PROPERTY C_STANDARD 11)
set_property(TARGET ${PROGNAME} PROPERTY CXX_STANDARD 11)
target_compile_definitions(${PROGNAME} PUBLIC MAMEVERSION=37 )

# the static libs that are part of this repo: 
target_link_libraries(${PROGNAME} PUBLIC mamecore z)

# the extra original static libs furnished by triumph,"moo db" is a database used for config:
target_link_directories(${PROGNAME} PUBLIC "../../lib")
target_include_directories(${PROGNAME} PUBLIC "../../include" )
target_link_libraries(${PROGNAME} PUBLIC moo mood cgxhooks )
#add_library(${PROGNAME} PUBLIC OBJECT IMPORTED  ../../c2p.s.o ../../gfx.s.o)

target_include_directories(${PROGNAME} PUBLIC "../mame037b/src")

# MUI is not provided by gccbebbo6.5 ndk by default, we have it there:
target_include_directories(${PROGNAME} PUBLIC "../../../extrasdk/MUI/include")

#  - - - - - - - - - - 
## debug/test phase, no release.
#set(RNAME rmame)
## amiga_locale.cpp config_v37.cpp config_v37.h
#add_executable(${RNAME} r2_main.cpp amiga_locale.cpp config_v37.cpp config_v37.h romscan.cpp romscan.h gui_mui.cpp gui_gadtools.cpp)
#set_property(TARGET ${RNAME} PROPERTY C_STANDARD 11)
#set_property(TARGET ${RNAME} PROPERTY CXX_STANDARD 11)
#target_compile_definitions(${RNAME} PUBLIC MAMEVERSION=37 INLINE=inline )
## the static libs that are part of this repo:
#target_link_directories(${RNAME} PUBLIC "../../lib")
#target_include_directories(${RNAME} PUBLIC "../../include" )
#target_include_directories(${RNAME} PUBLIC "../mame037b/src")

## MUI is not provided by gccbebbo6.5 ndk by default, we have it there:
#target_include_directories(${RNAME} PUBLIC "../../../extrasdk/MUI/include")
