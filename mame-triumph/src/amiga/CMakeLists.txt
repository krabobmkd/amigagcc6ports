# CMakelists.txt done to refresh old 1999 triumph amiga mame.

cmake_minimum_required (VERSION 3.1)

# VASM is an extra asm language configured in our cmake platform,
# to cross-assemble with vasm assembler installed by bebbo gcc
enable_language( VASM )
set(CXX_STANDARD 11)
set(C_STANDARD 11)
project(AmigaMame LANGUAGES C CXX VERSION 1.0)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(MAME_USE_HARDFLOAT "compile using direct 68882 FPU, will not work on economic 060" ON)
option(MAME_USE_68060 "else 040" ON)

#option(HOOKTEST "do" ON)

if(AMIGA)
#  -fno-rtti -fno-exceptions -fno-strict-aliasing
	# add your generic options here:
	add_compile_options( -save-temps )

	if(MAME_USE_68060)
		add_compile_options( -m68060 -mcpu=68060 )
		add_link_options( -m68060 -mcpu=68060 )
	else()
		add_compile_options( -m68040 -mcpu=68040 )
		add_link_options( -m68040 -mcpu=68040 )
	endif()
	if(MAME_USE_HARDFLOAT)
		add_compile_options( -mhard-float -ffast-math )
		add_link_options( -mhard-float -ffast-math)
	else()
		add_compile_options( -msoft-float )
		add_link_options( -msoft-float )
	endif()
endif()

add_subdirectory(../mame106 MameLib)

set(AMIGA_SRC
	main.cpp

	gui_mui.cpp
	gui_gadtools.cpp

	amiga106.cpp

	amiga106_inputs.h
	amiga106_inputs.cpp
	amiga106_audiostream.cpp
	amiga106_filesystem.cpp
	amiga106_timer.cpp
	amiga106_config.cpp
	amiga106_config.h
	amiga106_video.h
	amiga106_video.cpp
#	amiga106_video_p96.cpp
	amiga106_video_cgx.cpp

	amiga_locale.cpp
	main.h

	audio.h

	amiga_locale.h
	gui_mui.h
	gui_gadtools.h
	mame_ver.h
	mame_msg.h

	directdraw.s
#	c2p.s
#	gfx.s
#	../../lib/c2p.s.o ../../lib/gfx.s.o
#	cgxhooks.c
#	cgxhooks.h
#	cgxhooks_re.s
	)

set(PROGNAME Mame106)
add_executable(${PROGNAME} ${AMIGA_SRC} )
set_property(TARGET ${PROGNAME} PROPERTY C_STANDARD 11)
set_property(TARGET ${PROGNAME} PROPERTY CXX_STANDARD 11)
target_compile_definitions(${PROGNAME} PUBLIC MAMEVERSION=106 ZIP_CACHE=1 )
if(MAME_USE_HARDFLOAT)
	target_compile_definitions(${PROGNAME} PUBLIC MAME_USE_HARD_FLOAT=1 )
endif()
# the static libs that are part of this repo: 
target_link_libraries(${PROGNAME} PUBLIC mamecore z expat)

target_link_directories(${PROGNAME} PUBLIC "../../lib")
target_include_directories(${PROGNAME} PUBLIC "../../include" )

#add_library(${PROGNAME} PUBLIC OBJECT IMPORTED  ../../c2p.s.o ../../gfx.s.o)

# target_include_directories(${PROGNAME} PUBLIC "../mame037b/src")
target_include_directories(${PROGNAME} PUBLIC "../mame106")

# MUI is not provided by gccbebbo6.5 ndk by default, we have it there:
target_include_directories(${PROGNAME} PUBLIC "../../../extrasdk/MUI/include")
target_include_directories(${PROGNAME} PUBLIC "../../../extrasdk/P96/include")

# add_executable(cgxhooktest cgxhookstest.c cgxhooks.c cgxhooks.h )
# set_property(TARGET cgxhooktest PROPERTY C_STANDARD 11)

#  - - - - - - - - - - 
## debug/test phase, no release.
#set(RNAME rmame)
## amiga_locale.cpp config_v37.cpp config_v37.h
#add_executable(${RNAME} r2_main.cpp amiga_locale.cpp config_v37.cpp config_v37.h romscan.cpp romscan.h gui_mui.cpp gui_gadtools.cpp)
#set_property(TARGET ${RNAME} PROPERTY C_STANDARD 11)
#set_property(TARGET ${RNAME} PROPERTY CXX_STANDARD 11)
#target_compile_definitions(${RNAME} PUBLIC MAMEVERSION=37 static inline=inline )
## the static libs that are part of this repo:
#target_link_directories(${RNAME} PUBLIC "../../lib")
#target_include_directories(${RNAME} PUBLIC "../../include" )
#target_include_directories(${RNAME} PUBLIC "../mame037b/src")

## MUI is not provided by gccbebbo6.5 ndk by default, we have it there:
#target_include_directories(${RNAME} PUBLIC "../../../extrasdk/MUI/include")
