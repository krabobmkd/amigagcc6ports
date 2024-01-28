# to be included
# gather the list of gamedrivers source to be included in project.
# also gather list of corresponding CPU sources.
# just change MGD_XXXX variables with cmake -D option to enable machines or not in the project. 

option(MGD_SEGASYSTEM16 "" ON )
option(MGD_NEOMAME "" OFF)

if(MGD_SEGASYSTEM16)
	 add_compile_definitions(LINK_SEGASYSTEM16=1)
	list(APPEND MAME_DRIVERS_SRC
		drivers/system16.cpp
	)
	list(APPEND MAME_SOUND_SRC
		sound/upd7759.cpp
		sound/dac.cpp
		sound/2151intf.cpp
		sound/upd7759.cpp
		sound/rf5c68.cpp
		sound/2612intf.cpp # HAS_YM2612, HAS_YM3438
		sound/2203intf.cpp # need ay8910
		sound/segapcm.cpp
		sound/3812intf.cpp

		# also needed
		sound/ym2413.cpp
		sound/fmopl.cpp
		sound/fm.cpp
		sound/ay8910.cpp
	)
	list(APPEND CPU_DEFS HAS_DAC=1 HAS_YM2151=1 HAS_UPD7759=1 HAS_RF5C68=1
					HAS_YM2612=1 HAS_YM2203=1 HAS_SEGAPCM=1 HAS_YM3438=1 HAS_YM2413=1
					HAS_YM3812=1 HAS_AY8910=1)

	list(APPEND MAME_CPU_SRC
		cpu/m68000/m68kmame.cpp
		cpu/z80/z80.cpp
		cpu/i8039/i8039.cpp # HAS_N7751=1
	)
	list(APPEND CPU_DEFS HAS_M68000=1 HAS_Z80=1 HAS_N7751=1)

endif()

list(REMOVE_DUPLICATES MAME_DRIVERS_SRC)
list(REMOVE_DUPLICATES MAME_SOUND_SRC)
list(REMOVE_DUPLICATES MAME_CPU_SRC)
list(REMOVE_DUPLICATES CPU_DEFS)

