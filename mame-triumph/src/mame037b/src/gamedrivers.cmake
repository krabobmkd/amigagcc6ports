# to be included
# gather the list of gamedrivers source to be included in project.
# also gather list of corresponding CPU sources.
# just change MGD_XXXX variables with cmake -D option to enable machines or not in the project. 

#option(MGD_TINY_COMPILE = ON)


#option(MGD_ALL ON)
option(MGD_SEGASYSTEM16 "" ON )
option(MGD_NEOMAME "" OFF)

if(MGD_SEGASYSTEM16)
	 add_compile_definitions(LINK_SEGASYSTEM16=1)
	list(APPEND MAME_DRIVERS_SRC drivers/system16.cpp)
endif()

#set(MAME_DRIVERS_SRC
#	)
#set(MAME_CPU_SRC
#	)
#set(MAME_SOUND_SRC
#	)
