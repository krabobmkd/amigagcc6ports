#ifndef MAIN_H
#define MAIN_H

#ifndef __stdargs
// shut up clangd on qtcreator....
#define __stdargs
#endif


/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id: main.h,v 1.1 1999/04/28 18:54:28 meh Exp $
 *
 * $Log: main.h,v $
 * Revision 1.1  1999/04/28 18:54:28  meh
 * Initial revision
 *
 *
 *************************************************************************/

// included by everyone, urge to have less dependencies.

#include <exec/types.h>
//#include <dos/dos.h>
//#include <intuition/intuitionbase.h>
#include "driver.h"
#include "osdepend.h"

#include "config.h"
#include "video.h"
#include "amiga_inputs.h"

#define AUDIO_CHANNELS      16
#define AUDIO_BUFFER_LENGTH 2048

#if TRACE
#define TRACE_ENTER(f) puts("Entering "##f"()");
#define TRACE_LEAVE(f) puts("Leaving "##f"()");
#else
#define TRACE_ENTER(f)
#define TRACE_LEAVE(f)
#endif

struct Library;
struct ExecBase;
struct DosLibrary;
struct IntuitionBase;

extern struct ExecBase      *SysBase;
extern struct DosLibrary    *DOSBase;
extern struct Library       *GfxBase;
extern struct Library       *CyberGfxBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Library       *GadToolsBase;
extern struct Library       *AslBase;
extern struct Library       *KeymapBase;
extern struct Library       *TimerBase;
#ifdef POWERUP
extern struct Library       *PPCLibBase;
#endif

extern LONG                 Width;
extern LONG                 Height;
extern struct Audio         *Audio;
extern struct Video         *Video;
extern struct Inputs        *Inputs;
extern struct AChannelArray *ChannelArray[2];
extern struct VPixelArray   *PixelArray[2];
extern struct VDirectArray  *DirectArray;
extern LONG                 CurrentArray;
extern BYTE                 *Keys;
extern struct IPort         *Port1;
extern struct IPort         *Port2;

LONG  VideoOpen(LONG width, LONG height, LONG left, LONG top, LONG right, LONG bottom, LONG dirty);
void  VideoClose(void);
void  InputUpdate(LONG wait);

#ifdef POWERUP
extern struct GameDriver **Drivers;

#define M68k_MSG_STARTUP   0
#define M68k_MSG_CONFIG    1
#define M68k_MSG_VIDEODATA 2
#define M68k_MSG_FILE      3
#define M68k_MSG_FRAMEDATA 4
#define M68k_MSG_QUIT      5
#define M68k_MSG_SOUND     6

struct MsgStartupData
{
  void  *M68kPort;
  LONG  Version;
  LONG  Revision;
};

#define M68k_MSGDATA_CHANNELARRAY1    0
#define M68k_MSGDATA_CHANNELARRAY2    1
#define M68k_MSGDATA_CHANNELARRAYSIZE 2
#define M68k_MSGDATA_CONFIG           3

#define M68k_MSGDATA_RESULT         0
#define M68k_MSGDATA_WIDTH          1
#define M68k_MSGDATA_HEIGHT         2
#define M68k_MSGDATA_KEYS           3
#define M68k_MSGDATA_PORT1          4
#define M68k_MSGDATA_PORT2          5
#define M68k_MSGDATA_PIXELARRAY1    6
#define M68k_MSGDATA_PIXELARRAY2    7
#define M68k_MSGDATA_DIRECTARRAY    8

#define M68k_MSGSIZE_VIDEODATA  (9*sizeof(LONG))
#define M68k_MSGSIZE_CONFIG     ((3+CFG_ITEMS)*sizeof(LONG))
#define M68k_MSGSIZE_MAX        M68k_MSGSIZE_CONFIG

#define PPC_MSG_READY          0
#define PPC_MSG_VIDEOOPEN      1
#define PPC_MSG_VIDEOCLOSE     2
#define PPC_MSG_INPUTUPDATE    3
#define PPC_MSG_SETPIXELFRAME  4
#define PPC_MSG_SETDIRECTFRAME 5
#define PPC_MSG_OPENFILETYPE   6
#define PPC_MSG_CLOSEFILE      7
#define PPC_MSG_UCLOCK         8
#define PPC_MSG_QUIT           9
#define PPC_MSG_READSOUND      10
#define PPC_MSG_LOADSOUND      11

#define PPC_MSGDATA_DRIVERS 0

#define PPC_MSGDATA_WIDTH  0
#define PPC_MSGDATA_HEIGHT 1
#define PPC_MSGDATA_LEFT   2
#define PPC_MSGDATA_TOP    3
#define PPC_MSGDATA_RIGHT  4
#define PPC_MSGDATA_BOTTOM 5
#define PPC_MSGDATA_DIRTY  6

#define PPC_MSGDATA_DIRNAME  0
#define PPC_MSGDATA_FILENAME 4
#define PPC_MSGDATA_MODE     8
#define PPC_MSGDATA_TYPE     9

#define PPC_MSGDATA_SAMPLE     0
#define PPC_MSGDATA_RESOLUTION 1
#define PPC_MSGDATA_LENGTH     2
#define PPC_MSGDATA_FREQUENCY  3
#define PPC_MSGDATA_VOLUME     4

#define PPC_MSGSIZE_VIDEOOPEN    (7*sizeof(LONG))
#define PPC_MSGSIZE_OPENFILETYPE (10*sizeof(LONG))
#define PPC_MSGSIZE_LOADSOUND    (5*sizeof(LONG))
#define PPC_MSGSIZE_MAX          PPC_MSGSIZE_OPENFILETYPE

#else
#define Drivers (drivers)
#endif

#endif
