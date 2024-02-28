# completely redone by krb for Amiga port.
# still the zconf.h is processed.
cmake_minimum_required (VERSION 3.1)

set(C_STANDARD 11)
project(expat LANGUAGES C VERSION 1.17.6)

if(AMIGA)

	add_definitions(-O2 -m68020 -mhard-float -ffast-math -save-temps )
	add_definitions(-std=c11)
	
	set(BUILD_SHARED_LIBS OFF)
	set(SKIP_INSTALL_LIBRARIES ON)

#	set(HAVE_SYS_TYPES_H ON)
#	set(HAVE_STDINT_H ON)
#	set(HAVE_STDDEF_H ON)

endif()

#set(ZLIB_PUBLIC_HDRS
#    ${CMAKE_CURRENT_BINARY_DIR}/zconf.h
#    zlib.h
#)
set(EXPAT_HDRS
    ascii.h     expat_external.h  internal.h   utf8tab.h  xmltok_impl.h
    asciitab.h  iasciitab.h       latin1tab.h  xmlrole.h
    expat.h     intconfig.h       nametab.h    xmltok.h
)
set(EXPAT_SRCS
    xmlparse.c  xmlrole.c  xmltok.c  xmltok_impl.c  xmltok_ns.c
)

add_library(expat STATIC  ${EXPAT_SRCS} ${EXPAT_HDRS} )
set_property(TARGET expat PROPERTY C_STANDARD 11)

## generate zconf.h
#configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/../zlib/zconf.h.cmakein
#		${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

