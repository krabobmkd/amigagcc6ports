#  done in 2024 to adapt Mame4Allv0.37 to old triumph Amiga Mame.

cmake_minimum_required (VERSION 3.1)

set(CXX_STANDARD 11)
set(C_STANDARD 11)

project(MameLib LANGUAGES C CXX VERSION 1.0)

#if(AMIGA)
#
#	 add_compile_options(-m68060 -mhard-float -ffast-math -save-temps )
#endif()

include(gamedrivers.cmake)

set(ZLIBPATH "../../../../zlib")
add_subdirectory(${ZLIBPATH} zlib)

set(MAME_CORE_SRC
	version.cpp info.cpp audit.cpp datafile.cpp
    state.cpp png.cpp artwork.cpp unzip.cpp

	profiler.cpp cheat.cpp hiscore.cpp
	input.cpp inptport.cpp
    mame.cpp usrintrf.cpp ui_text.cpp
	tilemap.cpp sprite.cpp gfxobj.cpp
	drawgfx.cpp palette.cpp common.cpp
	cpuintrf.cpp memory.cpp timer.cpp
	scanline.cpp
	sndintrf.cpp

	sound/streams.cpp
	sound/mixer.cpp
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

set(MAME_INCLUDE_DIRS "src" "../../amiga" ".")

add_library( mamecore STATIC ${MAME_CORE_SRC}
							driver_modules.cpp ${MAME_DRIVERS_SRC} ${MAME_SOUND_SRC} ${MAME_CPU_SRC} )
set_property(TARGET mamecore PROPERTY C_STANDARD 11)
set_property(TARGET mamecore PROPERTY CXX_STANDARD 11)
target_include_directories(mamecore PUBLIC ${MAME_INCLUDE_DIRS} ${ZLIBPATH} )
target_compile_definitions(mamecore PUBLIC INLINE=inline ZIP_CACHE=1 ${CPU_DEFS} )
target_compile_options(mamecore PUBLIC -fpermissive -Wno-narrowing )

