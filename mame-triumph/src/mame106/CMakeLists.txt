#  done in 2024 to adapt Mame4Allv0.37 to old triumph Amiga Mame.

cmake_minimum_required (VERSION 3.1)

set(C_STANDARD 11)
set(CXX_STANDARD 11)
project(MameLib LANGUAGES C CXX VERSION 1.0)
option(MAME_USE_HARDFLOAT "compile using direct 68882 FPU, will not work on economic 060" ON)
option(MAME_USE_68060 "else 040" ON)
if(AMIGA)
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

include(gamedrivers.cmake)

set(ZLIBPATH "../../../zlib")
set(EXPATPATH "../../../expat")
add_subdirectory(${ZLIBPATH} zlib)
add_subdirectory(${EXPATPATH} expat)

set(MAME_CORE_SRC
# chdman.c is an external tool !
# romcmp.c  is an external tool !

 fileio.c
 artwork.c
   cpuint.c    inptport.c  palette.c   streams.c   video.c
	cpuintrf.c  input.c     png.c       tilemap.c
	cdrom.c    drawgfx.c   jedparse.c  timer.c     xml2info.c
	chd.c      driver.c    jedutil.c   xmlfile.c
	chdcd.c    mame.c      romload.c   ui_text.c
	  harddisk.c  mamecore.c  sha1.c      unzip.c
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


	# minimal common driver needs:
	vidhrdw/generic.c vidhrdw/vector.c

	machine/generic.c
	machine/generic.h
	machine/eeprom.c
	machine/eeprom.h

	sound/flt_vol.c
	sound/flt_vol.h
	sound/flt_rc.c
	sound/flt_rc.h
	sound/wavwrite.c
	sound/wavwrite.h

	sndhrdw/generic.c
	sndhrdw/generic.h

	)

# ${MAME_CORE_SRC} ${MAME_DRIVERS_SRC} ${MAME_SOUND_SRC} ${MAME_CPU_SRC}

set(MAME_INCLUDE_DIRS "src" "../amiga" "." "./includes" ${EXPATPATH})

add_library( mamecore STATIC ${MAME_CORE_SRC} ${MAME_DRIVERS_SRC} ${MAME_SOUND_SRC}
							  )
set_property(TARGET mamecore PROPERTY C_STANDARD 11)
set_property(TARGET mamecore PROPERTY CXX_STANDARD 11)
target_include_directories(mamecore PUBLIC ${MAME_INCLUDE_DIRS} ${ZLIBPATH} )
set(MAME_INLINE "static inline")
target_compile_definitions(mamecore PUBLIC INLINE=${MAME_INLINE} ZIP_CACHE=1 CRLF=2 ${CPU_DEFS} )
if(MAME_USE_HARDFLOAT)
	target_compile_definitions(mamecore PUBLIC MAME_USE_HARD_FLOAT=1 )
endif()
# -fpermissive
#target_compile_options(mamecore PUBLIC -O2 -Wno-narrowing )


# - - - - - - --  - -same for O2

add_library( mamecoreO2 STATIC ${MAME_CPU_SRC} )
set_property(TARGET mamecoreO2 PROPERTY C_STANDARD 11)
set_property(TARGET mamecoreO2 PROPERTY CXX_STANDARD 11)
target_include_directories(mamecoreO2 PUBLIC ${MAME_INCLUDE_DIRS} ${ZLIBPATH} )
set(MAME_INLINE "static inline")
target_compile_definitions(mamecoreO2 PUBLIC INLINE=${MAME_INLINE} ZIP_CACHE=1 CRLF=2 ${CPU_DEFS} )
if(MAME_USE_HARDFLOAT)
	target_compile_definitions(mamecoreO2 PUBLIC MAME_USE_HARD_FLOAT=1 )
endif()
# -fpermissive -Wno-narrowing
#target_compile_options(mamecoreO2 PUBLIC -O2  -Wno-narrowing )

# - - - - - - - - - - same for O3

#add_library( mamecoreO3 STATIC  ${MAME_DRIVERS_SRC} ${MAME_SOUND_SRC} ${MAME_CPU_SRC} )
#set_property(TARGET mamecoreO3 PROPERTY C_STANDARD 11)
#set_property(TARGET mamecoreO3 PROPERTY CXX_STANDARD 11)
#target_include_directories(mamecoreO3 PUBLIC ${MAME_INCLUDE_DIRS} ${ZLIBPATH} )
#set(MAME_INLINE "static inline")
#target_compile_definitions(mamecoreO3 PUBLIC INLINE=${MAME_INLINE} ZIP_CACHE=1 CRLF=2 ${CPU_DEFS} )
#if(MAME_USE_HARDFLOAT)
#	target_compile_definitions(mamecoreO3 PUBLIC MAME_USE_HARD_FLOAT=1 )
#endif()
## -fpermissive  -Wno-narrowing
#target_compile_options(mamecoreO3 PUBLIC -O3 )
