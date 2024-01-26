#  done for old triumph amiga mame.

cmake_minimum_required (VERSION 3.1)

set(CXX_STANDARD 11)
set(C_STANDARD 11)

project(MameLib LANGUAGES C CXX VERSION 1.0)

if(AMIGA)
	 add_compile_options(-O2 -m68060 -mhard-float -ffast-math -save-temps -fno-rtti )
endif()

include(gamedrivers.cmake)

set(ZLIBPATH "../../../../zlib")
add_subdirectory(${ZLIBPATH} zlib)

set(MAME_CORE_SRC
	version.cpp info.cpp audit.cpp datafile.cpp
    state.cpp png.cpp artwork.cpp unzip.cpp

#original file:	driver.cpp
	#version more prone to be reconfigured
	driver_mv.cpp

	profiler.cpp cheat.cpp hiscore.cpp
	input.cpp inptport.cpp
    mame.cpp usrintrf.cpp ui_text.cpp
	tilemap.cpp sprite.cpp gfxobj.cpp
	drawgfx.cpp palette.cpp common.cpp
	cpuintrf.cpp memory.cpp timer.cpp
	scanline.cpp
#	$(sort $(CPUOBJS)) 
	sndintrf.cpp

	sound/streams.cpp
	sound/mixer.cpp
#	$(SOUNDOBJS)
	machine/z80fmly.cpp machine/6821pia.cpp
	machine/8255ppi.cpp
	vidhrdw/generic.cpp vidhrdw/vector.cpp
	vidhrdw/avgdvg.cpp machine/mathbox.cpp
	machine/ticket.cpp machine/eeprom.cpp
	sound/votrax.cpp

	artwork.h   drawgfx.h   input.h         network.h   sndintrf.h      ui_text.h
	audit.h     driver.h    legacy.h        osdepend.h  sprite.h        unzip.h
	cheat.h     gfxobj.h    mame.h          palette.h   state.h         usrintrf.h
	common.h    hiscore.h   memory.h        png.h       tilemap_draw.h
	cpuintrf.h  info.h      memory_read.h   profiler.h  tilemap.h
	datafile.h  inptport.h  memory_write.h  scanline.h  timer.h


	)

add_library( mamecore STATIC ${MAME_CORE_SRC} ${MAME_DRIVERS_SRC} ${MAME_CPU_SRC} ${MAME_SOUND_SRC} )
set_property(TARGET mamecore PROPERTY C_STANDARD 11)
set_property(TARGET mamecore PROPERTY CXX_STANDARD 11)
#no, odx is an arm machine. set(MAME_INCLUDE_DIRS "src" "odx" )
set(MAME_INCLUDE_DIRS "src" "../../amiga" ".")

target_include_directories(mamecore PUBLIC ${MAME_INCLUDE_DIRS} ${ZLIBPATH} )
target_compile_definitions(mamecore PUBLIC INLINE=inline )

## generate zconf.h
#configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/../zlib/zconf.h.cmakein
#		${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)
#include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

