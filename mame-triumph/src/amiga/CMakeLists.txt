# CMakelists.txt done to refresh old 1999 triumph amiga mame.

cmake_minimum_required (VERSION 3.1)

# VASM is an extra asm language configured in our cmake platform,
# to cross-assemble with vasm assembler installed by bebbo gcc
enable_language( VASM )
set(CXX_STANDARD 11)
set(C_STANDARD 11)
project(AmigaTriumphMame LANGUAGES C CXX VERSION 1.0)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(AMIGA)
#  -fno-rtti -fno-exceptions -fno-strict-aliasing
	add_definitions( -m68060 -mhard-float -ffast-math -save-temps )
	
endif()

#add_subdirectory(../mame037b/src MameLib)
add_subdirectory(../mame106 MameLib)

set(AMIGA_SRC
	amiga106.cpp
	amiga106_inputs.cpp
	amiga106_audiostream.cpp
	amiga106_filesystem.cpp
	main.cpp
#	romscan.cpp
	gui_mui.cpp
	gui_gadtools.cpp
	config_moo.cpp
#	cgxhooks_re.c
#	file.cpp
	audio.cpp
	video.cpp
	amiga_inputs.cpp
	amiga_locale.cpp
	main.h
	config_moo.h
#	file.h
	audio.h
	video.h
	amiga_inputs.h
	amiga_locale.h
	gui_mui.h
	gui_gadtools.h
	mame_ver.h
	mame_msg.h

	c2p.s
	gfx.s
#	../../lib/c2p.s.o ../../lib/gfx.s.o
	cgxhooks_re.s
	)

set(PROGNAME TriumphMame)
add_executable(${PROGNAME} ${AMIGA_SRC} )
set_property(TARGET ${PROGNAME} PROPERTY C_STANDARD 11)
set_property(TARGET ${PROGNAME} PROPERTY CXX_STANDARD 11)
target_compile_definitions(${PROGNAME} PUBLIC MAMEVERSION=106 ZIP_CACHE=1 )

# the static libs that are part of this repo: 
target_link_libraries(${PROGNAME} PUBLIC mamecore z)

# the extra original static libs furnished by triumph,"moo db" is a database used for config:
target_link_directories(${PROGNAME} PUBLIC "../../lib")
target_include_directories(${PROGNAME} PUBLIC "../../include" )
target_link_libraries(${PROGNAME} PUBLIC moo mood )
#add_library(${PROGNAME} PUBLIC OBJECT IMPORTED  ../../c2p.s.o ../../gfx.s.o)

# target_include_directories(${PROGNAME} PUBLIC "../mame037b/src")
target_include_directories(${PROGNAME} PUBLIC "../mame106")

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
