#  done in 2024 to adapt Mame4Allv0.37 to old triumph Amiga Mame.

cmake_minimum_required (VERSION 3.1)

set(C_STANDARD 11)

project(MameLib LANGUAGES C CXX VERSION 1.0)

#if(AMIGA)
#
#	 add_compile_options(-m68060 -mhard-float -ffast-math -save-temps )
#endif()

#re include(gamedrivers.cmake)

set(ZLIBPATH "../../../zlib")
set(EXPATPATH "../../../expat")
add_subdirectory(${ZLIBPATH} zlib)
add_subdirectory(${EXPATPATH} expat)

set(MAME_CORE_SRC
# artwork.c x86drc.c profiler.c tiny.c
# fileio.c finally not, implemented on amiga side.
   cpuint.c    inptport.c  palette.c   streams.c   video.c
	audit.c    cpuintrf.c  input.c     png.c       tilemap.c
	cdrom.c    drawgfx.c   jedparse.c  timer.c     xml2info.c
	chd.c      driver.c    jedutil.c   romcmp.c    xmlfile.c
	chdcd.c    fileio.c    mame.c      romload.c   ui_text.c
	chdman.c   harddisk.c  mamecore.c  sha1.c      unzip.c
	cheat.c    hash.c      mamedriv.c  sndintrf.c  usrintrf.c
	config.c   hiscore.c   md5.c       sound.c     validity.c
	cpuexec.c  info.c      memory.c    state.c     version.c
#  artwork.h
 debugger.h   jedparse.h            png.h        ui_text.h
 audit.h      drawgfx.h    mame.h                profiler.h   unzip.h
 cdrom.h      driver.h     mamecore.h            romload.h    usrintrf.h
 chd.h        fileio.h     md5.h                 sha1.h       video.h
 chdcd.h      harddisk.h   memconv.h             sndintrf.h   x86drc.h
 cheat.h      hash.h       memory.h              sound.h      xmlfile.h
 config.h     hiscore.h    osd_tool.h            state.h
 cpuexec.h    info.h       streams.h
 cpuint.h     inptport.h   osdepend.h            tilemap.h
 cpuintrf.h   input.h      palette.h             timer.h


#	version.cpp info.cpp audit.cpp datafile.cpp
#    state.cpp png.cpp artwork.cpp unzip.cpp

#	profiler.cpp cheat.cpp hiscore.cpp
#	input.cpp inptport.cpp
#    mame.cpp usrintrf.cpp ui_text.cpp
#	tilemap.cpp sprite.cpp gfxobj.cpp
#	drawgfx.cpp palette.cpp common.cpp
#	cpuintrf.cpp memory.cpp timer.cpp
#	scanline.cpp
#	sndintrf.cpp

#	sound/streams.cpp
#	sound/mixer.cpp
#	machine/z80fmly.cpp machine/6821pia.cpp
#	machine/8255ppi.cpp
#	vidhrdw/generic.cpp vidhrdw/vector.cpp
#	vidhrdw/avgdvg.cpp machine/mathbox.cpp
#	machine/ticket.cpp machine/eeprom.cpp
#	sound/votrax.cpp

#	artwork.h   drawgfx.h   input.h         network.h   sndintrf.h      ui_text.h
#	audit.h     driver.h    legacy.h        osdepend.h  sprite.h        unzip.h
#	cheat.h     gfxobj.h    mame.h          palette.h   state.h         usrintrf.h
#	common.h    hiscore.h   memory.h        png.h       tilemap_draw.h
#	cpuintrf.h  info.h      memory_read.h   profiler.h  tilemap.h
#	datafile.h  inptport.h  memory_write.h  scanline.h  timer.h


	)

set(MAME_INCLUDE_DIRS "src" "../amiga" "." ${EXPATPATH})

add_library( mamecore STATIC ${MAME_CORE_SRC}
							 ${MAME_DRIVERS_SRC} ${MAME_SOUND_SRC} ${MAME_CPU_SRC} )
set_property(TARGET mamecore PROPERTY C_STANDARD 11)
set_property(TARGET mamecore PROPERTY CXX_STANDARD 11)
target_include_directories(mamecore PUBLIC ${MAME_INCLUDE_DIRS} ${ZLIBPATH} )
target_compile_definitions(mamecore PUBLIC INLINE=inline ZIP_CACHE=1 CRLF=2 ${CPU_DEFS} )
# -fpermissive
target_compile_options(mamecore PUBLIC  -Wno-narrowing )

