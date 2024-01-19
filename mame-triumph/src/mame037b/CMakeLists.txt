#  done for old triumph amiga mame.

cmake_minimum_required (VERSION 3.1)

set(CXX_STANDARD 11)
set(C_STANDARD 11)

project(MameLib LANGUAGES C CXX VERSION 1.0)

if(AMIGA)

	add_definitions(-O2 -m68060 -mhard-float -ffast-math -save-temps )
	add_definitions(-std=c11)
	
endif()

set(ZLIBPATH "../../../zlib")
add_subdirectory(${ZLIBPATH} zlib)


#    src/zlib/adler32.cpp src/zlib/compress.cpp src/zlib/crc32.cpp
#	src/zlib/gzio.cpp src/zlib/uncompr.cpp src/zlib/deflate.cpp
#	src/zlib/trees.cpp src/zlib/zutil.cpp src/zlib/inflate.cpp
#	src/zlib/infback.cpp src/zlib/inftrees.cpp src/zlib/inffast.cpp

set(MAME_CORE_SRC
	src/version.cpp src/info.cpp src/audit.cpp src/datafile.cpp 
    src/state.cpp src/png.cpp src/artwork.cpp src/unzip.cpp 

	src/profiler.cpp src/cheat.cpp src/hiscore.cpp 
	src/input.cpp src/inptport.cpp 
    src/mame.cpp src/usrintrf.cpp src/ui_text.cpp 
	src/tilemap.cpp src/sprite.cpp src/gfxobj.cpp 
	src/drawgfx.cpp src/palette.cpp src/common.cpp 
	src/cpuintrf.cpp src/memory.cpp src/timer.cpp 
	src/scanline.cpp 
#	$(sort $(CPUOBJS)) 
	src/sndintrf.cpp src/sound/streams.cpp src/sound/mixer.cpp 
#	$(SOUNDOBJS)
	src/machine/z80fmly.cpp src/machine/6821pia.cpp 
	src/machine/8255ppi.cpp 
	src/vidhrdw/generic.cpp src/vidhrdw/vector.cpp 
	src/vidhrdw/avgdvg.cpp src/machine/mathbox.cpp 
	src/machine/ticket.cpp src/machine/eeprom.cpp 
	src/sound/votrax.cpp
	)
set(MAME_CPU_SRC

	)
	
set(MAME_SOUND_SRC

	)	
add_library( mamecore STATIC ${MAME_CORE_SRC} ${MAME_CPU_SRC} ${MAME_SOUND_SRC} )
set_property(TARGET mamecore PROPERTY C_STANDARD 11)

set(MAME_INCLUDE_DIRS "src" "src/odx" )

target_include_directories(mamecore PUBLIC ${MAME_INCLUDE_DIRS} ${ZLIBPATH} )
target_compile_definitions(mamecore PUBLIC INLINE=inline )
## generate zconf.h
#configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/../zlib/zconf.h.cmakein
#		${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)
#include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

