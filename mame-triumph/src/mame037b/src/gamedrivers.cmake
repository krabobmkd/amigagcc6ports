# to be included
# gather the list of gamedrivers source to be included in project.
# also gather list of corresponding CPU sources.
# just change MGD_XXXX variables with cmake -D option to enable machines or not in the project. 

option(MGD_SEGASYSTEM16 "" ON )
option(MGD_NEOMAME "" ON)
option(MGD_CAPCOMSYSTEM "" ON)
# todo link
option(MGD_TAITO86 "" OFF)
#todo link
option(MGD_BEFORE_1983 "" OFF)

option(MGD_NINTENDO "" OFF)

option(MGD_1983_1985 "" OFF)

if(MGD_SEGASYSTEM16)
	 add_compile_definitions(LINK_SEGASYSTEM16=1)
	list(APPEND MAME_DRIVERS_SRC
		drivers/system16.cpp		
		#todo drivers/deniam.cpp
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

#endif MGD_SEGASYSTEM16
endif()

if(MGD_NEOMAME)
	 add_compile_definitions(LINK_NEOMAME=1)
	list(APPEND MAME_DRIVERS_SRC
		drivers/neogeo.cpp
	)
	list(APPEND MAME_SOUND_SRC
		sound/ay8910.cpp	# YM2610
		sound/2610intf.cpp
		#
		sound/fm.cpp
		sound/ymdeltat.cpp
	)
	list(APPEND CPU_DEFS HAS_YM2610=1 HAS_AY8910=1 HAS_YM2610=1)

	list(APPEND MAME_CPU_SRC
		cpu/m68000/m68kmame.cpp
		cpu/z80/z80.cpp
	)
	list(APPEND CPU_DEFS HAS_M68000=1 HAS_Z80=1)

# endif MGD_NEOMAME
endif()


if(MGD_CAPCOMSYSTEM)
	 add_compile_definitions(LINK_CAPCOMSYSTEM=1)
	list(APPEND MAME_DRIVERS_SRC
		drivers/cps1.cpp # capcom system1 games
		drivers/cps2.cpp # capcom system2 games
	)
	list(APPEND MAME_SOUND_SRC
		sound/2151intf.cpp
		sound/adpcm.cpp
		sound/qsound.cpp
		# also
		sound/fm.cpp # needed by 2151intf.cpp
	)

	list(APPEND CPU_DEFS HAS_QSOUND=1 HAS_YM2151=1 HAS_OKIM6295=1 )

	list(APPEND MAME_CPU_SRC
		cpu/m68000/m68kmame.cpp
		cpu/z80/z80.cpp

		machine/kabuki.cpp
	)
	list(APPEND CPU_DEFS HAS_M68000=1 HAS_M68010=1 HAS_M68EC020=1 HAS_Z80=1 )

# endif MGD_CAPCOMSYSTEM
endif()


if(MGD_TAITO86)
	 add_compile_definitions(LINK_TAITO86=1)
	list(APPEND MAME_DRIVERS_SRC
		drivers/bublbobl.cpp
		drivers/arkanoid.cpp # just need M68705.
	)
	list(APPEND MAME_SOUND_SRC
		sound/2203intf.cpp
		sound/3812intf.cpp
		sound/fmopl.cpp
	)
	list(APPEND CPU_DEFS HAS_YM2203=1 HAS_YM3526=1 HAS_YM3812=1 )

	list(APPEND MAME_CPU_SRC
		cpu/m6805/m6805.cpp
		cpu/z80/z80.cpp
	)
	list(APPEND CPU_DEFS HAS_M68705=1 HAS_Z80=1 )

# endif MGD_TAITO86
endif()

if(MGD_NINTENDO)
	add_compile_definitions(LINK_NINTENDO=1)
	list(APPEND MAME_DRIVERS_SRC
		drivers/dkong.cpp # nintendo 1981   - dkong, dkongjr.
		drivers/punchout.cpp # nintendo 1983
		drivers/mario.cpp
	)
	list(APPEND MAME_SOUND_SRC
#		sound/msm5205.cpp
#		sound/namco.cpp
		sound/samples.cpp
		sound/dac.cpp
#		sound/ay8910.cpp
#		sound/2151intf.cpp
#		sound/fm.cpp
		sound/nes_apu.cpp # dkong, punchout
		sound/vlm5030.cpp # punchout
#		sound/sn76496.cpp #pacman
	)
	# sounds CPU:
	list(APPEND CPU_DEFS
#		HAS_MSM5205=1 # mpatrol
#		HAS_NAMCO=1
		HAS_SAMPLES=1 # space invaders need.
#		HAS_CUSTOM=1 # galaxian
		HAS_DAC=1 # galaxian
#		HAS_AY8910=1 #mpatrol
#		HAS_YM2151=1 # qbert
		HAS_I8035=1 # dkong
		HAS_NES=1 # dkong  punchout
		HAS_VLM5030=1 # punchout
#		HAS_SN76496=1 #pacman
	 )
	# cpu:
	list(APPEND MAME_CPU_SRC
#		cpu/m6800/m6800.cpp
		cpu/s2650/s2650.cpp
		cpu/z80/z80.cpp
		cpu/m6502/m6502.cpp
#		cpu/i86/i86.cpp
		cpu/i8039/i8039.cpp
		)
	list(APPEND CPU_DEFS
#		HAS_M6803=1
#		HAS_M6808=1
		HAS_S2650=1 # CPU for scramble.
		HAS_Z80=1
		HAS_M6502=1 # qbert
#		HAS_I86=1 # qbert
		HAS_N2A03=1 # nintendo
		HAS_M65C02=1
	)


endif()

if(MGD_BEFORE_1983)
	 add_compile_definitions(LINK_BEFORE83=1)
	list(APPEND MAME_DRIVERS_SRC
		drivers/mpatrol.cpp # Irem 1982
		drivers/pengo.cpp	# coreland/sega 1982
		drivers/zaxxon.cpp	# sega 1982
		drivers/scramble.cpp # konami/stern 1981
		drivers/z80bw.cpp # space invaders: Taito/midway 1978
		drivers/galaxian.cpp #Namco 1979
		drivers/pacman.cpp #Namco 1980
		drivers/galaga.cpp	#Namco 1981
		drivers/gottlieb.cpp # qbert 1982
		drivers/dkong.cpp # nintendo 1981   - dkong, dkongjr.
		drivers/punchout.cpp # nintendo 1983
	)
	list(APPEND MAME_SOUND_SRC
		sound/msm5205.cpp
		sound/namco.cpp
		sound/samples.cpp
		sound/dac.cpp
		sound/ay8910.cpp
		sound/2151intf.cpp
		sound/fm.cpp
		sound/nes_apu.cpp # dkong, punchout
		sound/vlm5030.cpp # punchout
		sound/sn76496.cpp #pacman
	)
	# sounds CPU:
	list(APPEND CPU_DEFS
		HAS_MSM5205=1 # mpatrol
		HAS_NAMCO=1
		HAS_SAMPLES=1 # space invaders need.
		HAS_CUSTOM=1 # galaxian
		HAS_DAC=1 # galaxian
		HAS_AY8910=1 #mpatrol
		HAS_YM2151=1 # qbert
		HAS_I8035=1 # dkong
		HAS_NES=1 # dkong  punchout
		HAS_VLM5030=1 # punchout
		HAS_SN76496=1 #pacman
	 )
	# cpu:
	list(APPEND MAME_CPU_SRC
		cpu/m6800/m6800.cpp
		cpu/s2650/s2650.cpp
		cpu/z80/z80.cpp
		cpu/m6502/m6502.cpp
		cpu/i86/i86.cpp
		cpu/i8039/i8039.cpp
		)
	list(APPEND CPU_DEFS
		HAS_M6803=1
		HAS_M6808=1
		HAS_S2650=1 # CPU for scramble.
		HAS_Z80=1
		HAS_M6502=1 # qbert
		HAS_I86=1 # qbert
		HAS_N2A03=1 # nintendo
		HAS_M65C02=1
	)


# endif MGD_BEFORE_1983
endif()
list(REMOVE_DUPLICATES MAME_DRIVERS_SRC)
list(REMOVE_DUPLICATES MAME_SOUND_SRC)
list(REMOVE_DUPLICATES MAME_CPU_SRC)
list(REMOVE_DUPLICATES CPU_DEFS)

