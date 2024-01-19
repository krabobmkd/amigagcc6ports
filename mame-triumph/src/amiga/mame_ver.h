/* $Revision Header built automatically *************** (do not edit) ************
**
** © Copyright by Mats Eirik Hansen
**
** File             : mysrc:emu/mame/src/amiga/amiga.c
** Created on       : Tirsdag, 10.06.97 14:34:45
** Created by       : Mats Eirik Hansen
** Current revision : V35.12
**
**
** Purpose
** -------
**     Amiga port of M.A.M.E.
**
** Revision V35.12
** --------------
** created on Onsdag, 12.05.99 19:54:28  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Fredag, 14.05.99 10:37:01  by  --- Unknown ---.   LogMessage :
**   - Changed: If a game was found or not is now stored in
**     mame.cfg. If games are added then a rescan must be done
**     manually.
**  -*-  changed on Fredag, 14.05.99 10:34:16  by  --- Unknown ---.   LogMessage :
**   - Fixed: The rom and sample path for default bitmap and vector
**     games are searched for games with UseDefaults=No.
**  -*-  changed on Onsdag, 12.05.99 20:01:34  by  Mats Eirik Hansen.   LogMessage :
**   - Added: If the option AsyncPPC is set to no then the ppc task
**     will wait for the m68k task and then reuse the display
**     buffer. It can be used to avoid artifacts and flickering
**     display in some games like Space Invaders.
**  -*-  changed on Onsdag, 12.05.99 19:57:44  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed several bugs in the library used for parsing of
**     config files and command line. The command line parsing was
**     almost totally broken.
**  -*-  changed on Onsdag, 12.05.99 19:56:25  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: The on screen volume control is now toggled with the
**     ` key (the key above tab).
**  -*-  changed on Onsdag, 12.05.99 19:55:17  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: The sound volume is now set correctly.
**  -*-  created on Onsdag, 12.05.99 19:54:28  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on 0.35 beta 12.
**
** Revision V35.11
** --------------
** created on Fredag, 23.04.99 23:53:39  by  --- Unknown ---.   LogMessage :
**  -*-  changed on Mandag, 26.04.99 08:22:31  by  --- Unknown ---.   LogMessage :
**   - Added: Supports games with 16 bit colors in non direct video
**     modes.
**  -*-  created on Fredag, 23.04.99 23:53:39  by  --- Unknown ---.   LogMessage :
**   - Changed: Now based on 0.35 beta 11.
**
** Revision V35.10
** ---------------
** created on Onsdag, 21.04.99 17:18:55  by  --- Unknown ---.   LogMessage :
**   - Changed: Now based on 0.35 beta 10.
**
** Revision V35.8
** --------------
** created on Mandag, 29.03.99 19:45:29  by  --- Unknown ---.   LogMessage :
**   - Changed: Now based on 0.35 beta 8.
**
** Revision V34.9
** --------------
** created on Mandag, 18.01.99 18:33:57  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on 0.34 final.
**
** Revision V34.5
** --------------
** created on Søndag, 25.10.98 20:33:42  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on 0.34 beta 5.
**
** Revision V34.3
** --------------
** created on Fredag, 18.09.98 12:24:04  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on 0.34 beta 3.
**
** Revision V34.2
** --------------
** created on Tirsdag, 08.09.98 14:21:01  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Søndag, 13.09.98 19:59:16  by  Mats Eirik Hansen.   LogMessage :
**   - Added: When the driver gui object is active you can type
**     part of the game name to search towards it.
**  -*-  changed on Tirsdag, 08.09.98 14:22:02  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added automatic frameskip setting.
**  -*-  created on Tirsdag, 08.09.98 14:21:01  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on 0.34 beta 2.
**
** Revision V34.1
** --------------
** created on Onsdag, 19.08.98 16:55:43  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Torsdag, 20.08.98 23:14:53  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Some small changed in the GUI.
**  -*-  changed on Torsdag, 20.08.98 23:14:25  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Hopefully fixed some crashes in vector drivers.
**  -*-  created on Onsdag, 19.08.98 16:55:43  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on 0.34 beta 1.
**
** Revision V33.8
** --------------
** created on Mandag, 03.08.98 05:22:17  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Fredag, 07.08.98 22:29:12  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: All buttons on the CD32 joypad is now supported.
**  -*-  changed on Fredag, 07.08.98 16:47:35  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: The configuration system has been completely
**     rewritten. This has lead to many changes in the GUI and the
**     CLI interface.
**  -*-  changed on Fredag, 07.08.98 16:45:26  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: On AGA/ECS a custom c2p routine is used for all
**     depths and there's no need for a screen with more colors
**     than actually used.
**  -*-  changed on Onsdag, 05.08.98 20:20:14  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Now it's possible to snapshot the display and save it
**     to an file as an IFF ILBM.
**  -*-  changed on Onsdag, 05.08.98 20:19:00  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Change the speed limiting code so it can be used by
**     the m68k version as well.
**  -*-  changed on Mandag, 03.08.98 05:24:30  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added support for samples in the paula sound mode.
**  -*-  changed on Mandag, 03.08.98 05:23:41  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added two methods for direct rendering into graphic
**     card memory.
**  -*-  created on Mandag, 03.08.98 05:22:17  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on MAME 0.33 rc 1.
**
** Revision V33.7
** --------------
** created on Onsdag, 22.07.98 19:27:41  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Torsdag, 30.07.98 18:21:49  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added support for dirty line buffers.
**  -*-  changed on Torsdag, 23.07.98 03:52:11  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a couple of bugs that would show up if you had
**     a zipped ROM or sample archive that were missing one or more
**     of the needed files.
**  -*-  created on Onsdag, 22.07.98 19:27:41  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on 0.33 beta 7.
**
** Revision V33.6
** --------------
** created on Fredag, 19.06.98 09:47:14  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on 0.33 beta 6.
**
** Revision V33.5
** --------------
** created on Lørdag, 13.06.98 17:06:35  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Lørdag, 13.06.98 17:10:26  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: An initialization bug on PPC that made MAME crash
**     if it was started with frameskip > 0. This bug may have
**     caused other problems as well.
**  -*-  changed on Lørdag, 13.06.98 17:08:14  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a bug in the PPC C lib that prevented the cheat
**     feature to work on PPC.
**  -*-  created on Lørdag, 13.06.98 17:06:35  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on 0.33 beta 5.
**
** Revision V33.4
** --------------
** created on Torsdag, 11.06.98 16:05:36  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on 0.33 beta 4.
**
** Revision V33.3
** --------------
** created on Tirsdag, 02.06.98 16:48:37  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on 0.33 beta 3.
**
** Revision V33.2
** --------------
** created on Lørdag, 23.05.98 18:48:17  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now based on 0.33 beta 2.
**
** Revision V30.4
** --------------
** created on Tirsdag, 03.03.98 18:17:45  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Tirsdag, 03.03.98 18:18:38  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: The setup screen (TAB) wasn't visible in vector
**     games.
**  -*-  created on Tirsdag, 03.03.98 18:17:45  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Samples were looped in some games.
**
** Revision V30.3
** --------------
** created on Søndag, 15.02.98 00:10:20  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Søndag, 22.02.98 22:51:04  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Rewrote the audio support code to use AHI and be
**     more suited for the the PPC version.
**  -*-  created on Søndag, 15.02.98 00:10:20  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: The joystick didn't work correctly in games that
**     supported both joystick and mouse.
**
** Revision V30.2
** --------------
** created on Tirsdag, 03.02.98 20:59:37  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Tirsdag, 03.02.98 21:12:31  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added MAKEDIRS option that can be used to make the
**     directories for all the supported games.
**  -*-  changed on Tirsdag, 03.02.98 21:07:54  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a bug in the tooltype handling code that could
**     cause all sorts of strange behaviour.
**  -*-  changed on Tirsdag, 03.02.98 21:06:54  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a bug in the Rastan sound code that caused 
**     crashes on exit.
**  -*-  changed on Tirsdag, 03.02.98 21:05:12  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Rewrote some memory access function in asm.
**  -*-  changed on Tirsdag, 03.02.98 21:01:07  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a "bug" that caused the audio CPU to be
**     emulated even if the sound was turned off.
**  -*-  created on Tirsdag, 03.02.98 20:59:37  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Rewrote the game scanning routine so it should be
**     faster and recognice archives.
**
** Revision V30.1
** --------------
** created on Søndag, 25.01.98 18:21:02  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Søndag, 25.01.98 18:28:33  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added better support for <256 color screens.
**  -*-  changed on Søndag, 25.01.98 18:22:09  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added support triple buffering.
**  -*-  changed on Søndag, 25.01.98 18:21:38  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added support for files packed with zip, lha or lzx.
**  -*-  created on Søndag, 25.01.98 18:21:02  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a small "bug" causing the joystick fire button
**     to not work.
**
** Revision V30.0
** --------------
** created on Lørdag, 10.01.98 20:30:04  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Mandag, 12.01.98 02:41:38  by  Mats Eirik Hansen.   LogMessage :
**   - Added: A larger stack will be allocated if needed.
**  -*-  created on Lørdag, 10.01.98 20:30:04  by  Mats Eirik Hansen.   LogMessage :
**   - Upgraded: Upgraded to M.A.M.E. 0.30.
**
** Revision V29.1
** --------------
** created on Lørdag, 25.10.97 14:08:46  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Lørdag, 10.01.98 20:28:51  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added support for a second joystick. New options
**     JOY2, J2BUTTONBTIME and J2AUTOFIRE. FIRE2TIME changed to
**     BUTTONBTIME.
**  -*-  changed on Lørdag, 10.01.98 20:24:03  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a bug in the asm drawgfx() that caused black
**     boxes around sprites in som games.
**  -*-  created on Lørdag, 25.10.97 14:08:46  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a bug that caused parts of window border to
**     be overwritten at startup if MAME was run in a window on
**     WB.
**
** Revision V29.0
** --------------
** created on Onsdag, 22.10.97 13:41:56  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Fredag, 24.10.97 03:23:44  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a bug in the asm drawgfx() that caused
**     invisible spaceships in Galaga.
**  -*-  changed on Fredag, 24.10.97 00:21:55  by  Mats Eirik Hansen.   LogMessage :
**   - Removed: Removed the REMOVELINES option. It wasn't really
**     useful.
**  -*-  changed on Fredag, 24.10.97 00:21:09  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Games using dynamic palettes were really slow on
**     AGA.
**  -*-  created on Onsdag, 22.10.97 13:41:56  by  Mats Eirik Hansen.   LogMessage :
**   - Upgraded: Upgraded to M.A.M.E. 0.29.
**
** Revision V28.2
** --------------
** created on Torsdag, 09.10.97 23:28:04  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: The saving of options to icons didn't filter out the
**     existing tooltypes.
**
** Revision V28.1
** --------------
** created on Lørdag, 04.10.97 20:51:13  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Onsdag, 08.10.97 00:05:56  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a bug that cause Rastan to crash on exit.
**  -*-  changed on Onsdag, 08.10.97 00:05:34  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a major typing error in the OpenScreenTags()
**     call.
**  -*-  changed on Søndag, 05.10.97 01:31:02  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added the SMR=SCREENMODEREQ option which behaves
**     like the old SCREEN option.
**  -*-  changed on Søndag, 05.10.97 01:30:20  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: If the SCREEN option is used without DISPLAYID
**     M.A.M.E. will no longer put up a screen mode requester
**     but instead use the best matching screen mode.
**  -*-  changed on Lørdag, 04.10.97 20:52:26  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: The way the asm drawgfx() handled transparency wasn't
**     compatible with the original version.
**  -*-  created on Lørdag, 04.10.97 20:51:13  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: drawgfx() now doesn't create enforcer hits when
**     given an illegal clip rectangle. This cause crashes with
**     amon others Rush'n'Attack.
**
** Revision V28.0
** --------------
** created on Tirsdag, 16.09.97 22:56:39  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Torsdag, 02.10.97 14:23:01  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added the option MINCHIP to prevent M.A.M.E. from
**     filling up chip ram with samples so the display couldn't be
**     opened.
**  -*-  changed on Torsdag, 02.10.97 14:21:30  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added the options FLIPX and FLIPY.
**  -*-  changed on Torsdag, 02.10.97 14:20:56  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a bug in the sound code that caused noise in
**     channels that should be silent.
**  -*-  changed on Torsdag, 02.10.97 14:19:54  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a bug in the vector routines that caused
**     unwanted lines to be drawn.
**  -*-  changed on Torsdag, 02.10.97 14:18:59  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added support for saving options into icons from the
**     menu.
**  -*-  changed on Torsdag, 02.10.97 14:18:07  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Better palette handling on 8 bit custom screens and
**     hi/true-color screens.
**  -*-  changed on Torsdag, 02.10.97 14:16:35  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now uses the CGXHooks linker lib by Trond Werner
**     Hansen for drawing to CyberGraphX screens.
**  -*-  changed on Torsdag, 02.10.97 14:15:11  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Rewrote the main gfx draw routine in assembler.
**  -*-  changed on Torsdag, 02.10.97 14:14:06  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added a chunky to planar routine by Mikael Kalms.
**  -*-  created on Tirsdag, 16.09.97 22:56:39  by  Mats Eirik Hansen.   LogMessage :
**   - Upgraded: Upgraded to M.A.M.E. 0.28.
**
** Revision V27.7
** --------------
** created on Tirsdag, 19.08.97 07:06:34  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Tirsdag, 19.08.97 13:09:34  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added MOUSE option that turns on mouse support.
**  -*-  changed on Tirsdag, 19.08.97 07:13:50  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added LESSFLICKER, DOUBLEBUFFER and REMOVELINES
**     options that is used to select between six different ways to
**     do the drawing of vector games.
**  -*-  changed on Tirsdag, 19.08.97 07:12:27  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added WIDTH and HEIGHT options. Vector games will be
**     scaled to fit the window.
**  -*-  changed on Tirsdag, 19.08.97 07:09:59  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Changed the CHANNELS option so it accepts channel
**     numbers from 0 to F.
**  -*-  changed on Tirsdag, 19.08.97 07:08:35  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Changed the DISPLAYID option so it accepts hex numbers
**     with $ or 0x as prefix.
**  -*-  changed on Tirsdag, 19.08.97 07:07:46  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added NOSCAN option that disables the scanning of the
**     installed games in the GUI.
**  -*-  changed on Tirsdag, 19.08.97 07:07:04  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added NOJOY option to turn off joystick support.
**  -*-  created on Tirsdag, 19.08.97 07:06:34  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added support for multiple joystick buttons and the
**     CD32 controller if lowlevel.library is available.
**
** Revision V27.6
** --------------
** created on Mandag, 11.08.97 21:01:43  by  Mats Eirik Hansen.   LogMessage :
**   - Upgraded: Upgraded to M.A.M.E. 0.27.
**
** Revision V26.6
** --------------
** created on Lørdag, 26.07.97 02:48:46  by  Mats Eirik Hansen.   LogMessage :
**   - Upgraded: Upgraded to M.A.M.E. 0.26.1.
**
** Revision V26.5
** --------------
** created on Onsdag, 16.07.97 03:41:29  by  Mats Eirik Hansen.   LogMessage :
**   - Upgraded: Upgraded to M.A.M.E. 0.26.
**
** Revision V25.5
** --------------
** created on Tirsdag, 15.07.97 14:40:37  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Tirsdag, 15.07.97 14:43:42  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added a menu with "New",  "Open", "About" and "Quit".
**  -*-  changed on Tirsdag, 15.07.97 14:42:15  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added new options DISPLAYID and DEPTH.
**  -*-  created on Tirsdag, 15.07.97 14:40:37  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added Workbench support. Now it's possible to make
**     project icons with the options as tooltypes.
**
** Revision V25.4
** --------------
** created on Søndag, 06.07.97 23:58:57  by  Mats Eirik Hansen.   LogMessage :
**   - Upgraded: Upgraded to M.A.M.E. 0.25. This took longer than
**     expected due to many more or less serious bugs in the new
**     drivers.
**
** Revision V24.4
** --------------
** created on Lørdag, 28.06.97 21:28:57  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Søndag, 29.06.97 22:22:39  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed som GUI bugs.
**  -*-  changed on Søndag, 29.06.97 19:57:23  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Added support for real time changing of the palette.
**     This also works on true color screens and it's used by the
**     Williams games.
**  -*-  changed on Lørdag, 28.06.97 21:37:24  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed a nasty bug that caused Kung Fu Master to crash
**     on exit. Kung Fu Master generated a sample set that was
**     allocated with malloc() but freesamples() expects the memory
**     to be allocated with AllocVec().
**  -*-  changed on Lørdag, 28.06.97 21:30:43  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Fixed two bugs in the audio code that caused some games
**     (Pacman, Kung Fu Master and some more) to crash if the NOSOUND
**     option was used.
**  -*-  created on Lørdag, 28.06.97 21:28:57  by  Mats Eirik Hansen.   LogMessage :
**   - Updated: Updated the game list in the gui to match the 0.24
**     games.
**
** Revision V24.3
** --------------
** created on Fredag, 13.06.97 19:21:19  by  Mats Eirik Hansen.   LogMessage :
**   - Upgraded: Upgraded to M.A.M.E. 0.24.
**
** Revision V23.3
** --------------
** created on Tirsdag, 10.06.97 14:40:43  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Fredag, 13.06.97 19:20:28  by  Mats Eirik Hansen.   LogMessage :
**   - Added: FPS option that displays frames per second in the
**     titlebar together with a status bar that tells you which
**     virtual channels that have been accessed. This can be
**     used to find the wanted CHANNELS argument.
**  -*-  changed on Fredag, 13.06.97 14:42:48  by  Mats Eirik Hansen.   LogMessage :
**   - Added: A CHANNELS option where the argument should be a 4
**     byte string with characters from '0' to '7'. This tells the
**     audio rotuines which virtual channel should go to which real
**     channel.
**  -*-  changed on Fredag, 13.06.97 14:40:32  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Rewrote the audio routines so that they can use the
**     samples sets and play the music in games like Galaga and
**     Pacman.
**  -*-  changed on Fredag, 13.06.97 14:39:10  by  Mats Eirik Hansen.   LogMessage :
**   - Changed: Now it uses ReadArgs() to parse the arguments so no
**     more - in front of the options.
**  -*-  changed on Fredag, 13.06.97 14:35:50  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Auto fire by holding down the fir button. The number
**     of shots per second can be set with the AF=AUTOFIRE
**     option to between 0 (off) and 10. It defaults to off and
**     it's only when it's off that Fire2 emulation can be on.
**  -*-  changed on Fredag, 13.06.97 14:33:07  by  Mats Eirik Hansen.   LogMessage :
**   - Added: Fire2 emulation by holding down the button for more
**     than a set time. The time can be set with the new
**     FT=FIRE2TIME option to value between 0 (off) and 9 (*0.1s).
**     It defaults to 5 (*0.1s).
**  -*-  created on Tirsdag, 10.06.97 14:40:43  by  Mats Eirik Hansen.   LogMessage :
**   - Added: MUI GUI.
**
** Revision V23.2
** --------------
** created on Tirsdag, 10.06.97 14:37:39  by  Mats Eirik Hansen.   LogMessage :
**  -*-  changed on Tirsdag, 07.06.97 14:39:08  by
**   - Added: Custom screen support.
**  -*-  changed on Tirsdag, 07.06.97 14:38:42  by  Mats Eirik Hansen.   LogMessage :
**   - Added: 4 channel sound support.
**  -*-  changed on Tirsdag, 07.06.97 14:38:20  by  Mats Eirik Hansen.   LogMessage :
**   - Fixed: Wrote 4 bytes too much into the bitmap.
**  -*-  created on Tirsdag, 07.06.97 14:37:39  by  Mats Eirik Hansen.   LogMessage :
**   - Rewrote keyboardboard support. Using TAB to enter the
**     settings seems to work.
**
** Revision V23.1
** --------------
** created on Onsdag, 04.06.97 14:34:45  by  Mats Eirik Hansen.   LogMessage :
**     --- Initial release ---
**
*********************************************************************************/
#define REVISION "35.12"
#define REVDATE  "14.05.99"
#define REVTIME  "10:37:01"
#define AUTHOR   "--- Unknown ---"
#define VERNUM   35
#define REVNUM   12
