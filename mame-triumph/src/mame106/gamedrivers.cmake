# to be included
# gather the list of gamedrivers source to be included in project.
# also gather list of corresponding CPU sources.
# just change MGD_XXXX variables with cmake -D option to enable machines or not in the project. 

option(MGD_SEGASYSTEM16 "" ON )
option(MGD_NEOMAME "" OFF)
option(MGD_CAPCOMSYSTEM "" OFF)
# todo link
option(MGD_TAITO86 "" OFF)
#todo link
option(MGD_BEFORE_1983 "" OFF)

option(MGD_NINTENDO "" OFF)

option(MGD_1983_1985 "" OFF)

if(MGD_SEGASYSTEM16)
	 add_compile_definitions(LINK_SEGASYSTEM16=1)
	list(APPEND MAME_DRIVERS_SRC
		drivers/system16.c
		#todo drivers/deniam.c
	)
	list(APPEND MAME_SOUND_SRC
		sound/upd7759.c
		sound/dac.c
		sound/2151intf.c
		sound/upd7759.c
		sound/rf5c68.c
		sound/2612intf.c # HAS_YM2612, HAS_YM3438
		sound/2203intf.c # need ay8910
		sound/segapcm.c
		sound/3812intf.c

		# also needed
		sound/ym2413.c
		sound/fmopl.c
		sound/fm.c
		sound/ay8910.c
	)
	list(APPEND CPU_DEFS HAS_DAC=1 HAS_YM2151=1 HAS_UPD7759=1 HAS_RF5C68=1
					HAS_YM2612=1 HAS_YM2203=1 HAS_SEGAPCM=1 HAS_YM3438=1 HAS_YM2413=1
					HAS_YM3812=1 HAS_AY8910=1)

	list(APPEND MAME_CPU_SRC
		cpu/m68000/m68kmame.c
		cpu/z80/z80.c
		cpu/i8039/i8039.c # HAS_N7751=1
	)
	list(APPEND CPU_DEFS HAS_M68000=1 HAS_Z80=1 HAS_N7751=1)

#endif MGD_SEGASYSTEM16
endif()

list(REMOVE_DUPLICATES MAME_DRIVERS_SRC)
list(REMOVE_DUPLICATES MAME_SOUND_SRC)
list(REMOVE_DUPLICATES MAME_CPU_SRC)
list(REMOVE_DUPLICATES CPU_DEFS)

