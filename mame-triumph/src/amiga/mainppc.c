/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id: mainppc.c,v 1.1 1999/04/28 18:54:28 meh Exp meh $
 *
 * $Log: mainppc.c,v $
 * Revision 1.1  1999/04/28 18:54:28  meh
 * Initial revision
 *
 *
 *************************************************************************/

#include <stdio.h>

#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <utility/tagitem.h>
#include <powerup/ppclib/interface.h>
#include <powerup/ppclib/message.h>
#include <powerup/ppclib/tasks.h>
#include <powerup/gcclib/powerup_protos.h>

#include "main.h"
#include "version.h"
#include "config.h"
#include "file.h"
#include "audio.h"
#include "osdepend.h"
#include "driver.h"

void  *M68kPort;
void  *PPCPort;
void  *MyMsg;
LONG  *MsgData;

LONG  Config[CFG_ITEMS];

extern struct osd_bitmap *BitMap;
extern LONG              ClearBitMap;
extern int               frameskip;
extern int               throttle;
extern char              *DirtyLines;

LONG FPS;
LONG FrameSkip;

LONG                  Width;
LONG                  Height;
struct Audio          *Audio;
struct Video          *Video;
struct AChannelArray  *ChannelArray[2];
struct VPixelArray    *PixelArray[2];
struct VDirectArray   *DirectArray;
LONG                  ChannelArraySize;
LONG                  CurrentArray;
BYTE                  *Keys;
struct IPort          *Port1;
struct IPort          *Port2;

void StartGame(void);

int main(void)
{
  void          *m68k_msg;
  LONG          *cfg;
  LONG          i;
  struct MsgStartupData *msg_startup_data;

  if(__initstdio())
  {
  if(__initstdfio())
  {

  msg_startup_data = (struct MsgStartupData *) PPCGetTaskAttr(PPCTASKTAG_STARTUP_MSGDATA);

  if((msg_startup_data->Version == VERNUM) && (msg_startup_data->Revision == REVNUM))
  {
    PPCPort = (void *) PPCGetTaskAttr(PPCTASKTAG_MSGPORT);
  
    if(PPCPort)
    {
      M68kPort = msg_startup_data->M68kPort;

      MyMsg = PPCCreateMessage(PPCPort, PPC_MSGSIZE_MAX);
      
      if(MyMsg)
      {
        MsgData = PPCAllocVec(PPC_MSGSIZE_MAX, MEMF_ANY);
        
        if(MsgData)
        {
          PPCCacheFlushAll();

          PPCSendMessage(M68kPort, MyMsg, &drivers, 0, PPC_MSG_READY);
          PPCWaitPort(PPCPort);
          PPCGetMessage(PPCPort);

          while(1)
          {
            PPCWaitPort(PPCPort);
            m68k_msg = PPCGetMessage(PPCPort);

            cfg = (LONG *) PPCGetMessageAttr(m68k_msg, PPCMSGTAG_DATA);

            if(cfg)
            {
              PPCCacheFlushAll();

              for(i = 0; i < CFG_ITEMS; i++)
                Config[i] = cfg[M68k_MSGDATA_CONFIG + i];

              ChannelArray[0]   = (struct AChannelArray *) cfg[M68k_MSGDATA_CHANNELARRAY1];
              ChannelArray[1]   = (struct AChannelArray *) cfg[M68k_MSGDATA_CHANNELARRAY2];
              ChannelArraySize  = cfg[M68k_MSGDATA_CHANNELARRAYSIZE];

              PPCReplyMessage(m68k_msg);

              if(ChannelArray[0] && ChannelArray[1])
              {
                PPCCacheInvalid(ChannelArray[0], ChannelArraySize);
                PPCCacheInvalid(ChannelArray[1], ChannelArraySize);
              }
            }
            else
            {           
              PPCReplyMessage(m68k_msg);
              break;
            }
            
            Keys = NULL;

            StartGame();
            
            PPCSendMessage(M68kPort, MyMsg, 0, 0, PPC_MSG_QUIT);
            PPCWaitPort(PPCPort);
            PPCGetMessage(PPCPort);
          }

          PPCFreeVec(MsgData);
        }
        PPCDeleteMessage(MyMsg);
      }
    }
  }
  }
  __exitstdio();
  }
  __exitmalloc();

  return(0);
}

LONG VideoOpen(LONG width, LONG height, LONG left, LONG top, LONG right, LONG bottom, LONG dirty)
{
  void  *m68k_msg;
  LONG  *m68k_msg_data;

  MsgData[PPC_MSGDATA_WIDTH]  = width;
  MsgData[PPC_MSGDATA_HEIGHT] = height;
  MsgData[PPC_MSGDATA_LEFT]   = left;
  MsgData[PPC_MSGDATA_TOP]    = top;
  MsgData[PPC_MSGDATA_RIGHT]  = right;
  MsgData[PPC_MSGDATA_BOTTOM] = bottom;
  MsgData[PPC_MSGDATA_DIRTY]  = dirty;

  PPCSendMessage(M68kPort, MyMsg, MsgData, PPC_MSGSIZE_VIDEOOPEN, PPC_MSG_VIDEOOPEN);
  PPCWaitPort(PPCPort);
  PPCGetMessage(PPCPort);

  PPCWaitPort(PPCPort);
  m68k_msg = PPCGetMessage(PPCPort);
  
  m68k_msg_data = (LONG *) PPCGetMessageAttr(m68k_msg, PPCMSGTAG_DATA);

  if(m68k_msg_data[M68k_MSGDATA_RESULT])
  {
    Width  = m68k_msg_data[M68k_MSGDATA_WIDTH];
    Height = m68k_msg_data[M68k_MSGDATA_HEIGHT];

    PixelArray[0] = (struct VPixelArray *)  m68k_msg_data[M68k_MSGDATA_PIXELARRAY1];
    PixelArray[1] = (struct VPixelArray *)  m68k_msg_data[M68k_MSGDATA_PIXELARRAY2];
    DirectArray   = (struct VDirectArray *) m68k_msg_data[M68k_MSGDATA_DIRECTARRAY];

    Keys  = (UBYTE *)        m68k_msg_data[M68k_MSGDATA_KEYS];
    Port1 = (struct IPort *) m68k_msg_data[M68k_MSGDATA_PORT1];
    Port2 = (struct IPort *) m68k_msg_data[M68k_MSGDATA_PORT2];

    CurrentArray = 0;

    PPCReplyMessage(m68k_msg);

    PPCCacheFlushAll();

    return(1);
  }

  PPCReplyMessage(m68k_msg);
  
  return(0);
}

void VideoClose(void)
{
  PPCSendMessage(M68kPort, MyMsg, 0, 0, PPC_MSG_VIDEOCLOSE);
  PPCWaitPort(PPCPort);
  PPCGetMessage(PPCPort);
}

void InputUpdate(LONG wait)
{
  if(wait)
  {
    PPCSendMessage(M68kPort, MyMsg, 0, 0, PPC_MSG_INPUTUPDATE);
    PPCWaitPort(PPCPort);
    PPCGetMessage(PPCPort);
  }
}

struct File *OpenFileType(const char *dir_name, const char *file_name, int mode, int type)
{
  struct File *file;
  void    *m68k_msg;
  LONG    i;

  i = 0;
  if(dir_name)
  {
    for(; dir_name[i]; i++)
      ((UBYTE *) &MsgData[PPC_MSGDATA_DIRNAME])[i] = dir_name[i];
  }
  ((UBYTE *) &MsgData[PPC_MSGDATA_DIRNAME])[i] = 0;

  i = 0;
  if(file_name)
  {
    for(i = 0; file_name[i]; i++)
      ((UBYTE *) &MsgData[PPC_MSGDATA_FILENAME])[i] = file_name[i];
  }
  ((UBYTE *) &MsgData[PPC_MSGDATA_FILENAME])[i] = 0;

  MsgData[PPC_MSGDATA_MODE] = mode;
  MsgData[PPC_MSGDATA_TYPE] = type;

  PPCSendMessage(M68kPort, MyMsg, MsgData, PPC_MSGSIZE_OPENFILETYPE, PPC_MSG_OPENFILETYPE);
  PPCWaitPort(PPCPort);
  PPCGetMessage(PPCPort);

  PPCWaitPort(PPCPort);
  m68k_msg = PPCGetMessage(PPCPort);
  
  file = (struct File *) PPCGetMessageAttr(m68k_msg, PPCMSGTAG_DATA);

  PPCReplyMessage(m68k_msg);

  return(file);
}

void CloseFile(struct File *file)
{
  PPCSendMessage(M68kPort, MyMsg, file, 0, PPC_MSG_CLOSEFILE);
  PPCWaitPort(PPCPort);
  PPCGetMessage(PPCPort);
}

void VSetPixelFrame(struct VPixelArray *pixelarray)
{
  void *m68k_msg;
  LONG i;
  
  VFlushPixelArray(PixelArray[CurrentArray]);

  if(ChannelArray[0] && ChannelArray[1])
    PPCCacheFlush(ChannelArray[CurrentArray], ChannelArraySize);

  PPCSendMessage(M68kPort, MyMsg, (void *) (CurrentArray|(frameskip<<4)|(throttle)<<8), 0, PPC_MSG_SETPIXELFRAME);
  PPCWaitPort(PPCPort);
  PPCGetMessage(PPCPort);

  PPCWaitPort(PPCPort);
  m68k_msg  = PPCGetMessage(PPCPort); 
  FrameSkip = (LONG) PPCGetMessageAttr(m68k_msg, PPCMSGTAG_DATA);
  FPS       = FrameSkip & 0xffff;
  FrameSkip = FrameSkip >> 16;
  PPCReplyMessage(m68k_msg);

  if(Config[CFG_ASYNCPPC])
  {
    CurrentArray = (CurrentArray + 1) & 1;

    if(CurrentArray)
      BitMap->line= (unsigned char **) ((ULONG) &BitMap[1] + BitMap->height * sizeof(unsigned char *));
    else
      BitMap->line= (unsigned char **) &BitMap[1];

    if(ClearBitMap)
      osd_clearbitmap(BitMap);
  }

  for(i = 0; i < 256; i++)
    PixelArray[CurrentArray]->Palette[i][0] = 0;

  if(ChannelArray[0] && ChannelArray[1])
  {
    for(i = 0; i < AUDIO_CHANNELS; i++)
      ChannelArray[CurrentArray]->Channels[i].Flags = 0;

    ChannelArray[CurrentArray]->Flags = 0;
  }
}

void VSetDirectFrame(struct VDirectArray *directarray)
{
  void  *m68k_msg;
  LONG  i;
  
  if(ChannelArray[0] && ChannelArray[1])
    PPCCacheFlush(ChannelArray[CurrentArray], ChannelArraySize);
  PPCSendMessage(M68kPort, MyMsg, (void *) (CurrentArray|(frameskip<<4)|(throttle)<<8), 0, PPC_MSG_SETDIRECTFRAME);
  PPCWaitPort(PPCPort);
  PPCGetMessage(PPCPort);

  PPCWaitPort(PPCPort);
  m68k_msg  = PPCGetMessage(PPCPort); 
  FrameSkip = (LONG) PPCGetMessageAttr(m68k_msg, PPCMSGTAG_DATA);
  FPS     = FrameSkip & 0xffff;
  FrameSkip = FrameSkip >> 16;
  PPCReplyMessage(m68k_msg);

  CurrentArray = (CurrentArray + 1) & 1;

  if(ChannelArray[0] && ChannelArray[1])
  {
    for(i = 0; i < AUDIO_CHANNELS; i++)
      ChannelArray[CurrentArray]->Channels[i].Flags = 0;

    ChannelArray[CurrentArray]->Flags = 0;
  }
}

void ASetChannelFrame(struct AChannelArray *channelarray)
{
}

struct ASound *AReadSound(struct Audio *audio, BPTR file)
{
  struct ASound *sound;
  void      *m68k_msg;

  PPCSendMessage(M68kPort, MyMsg, (void *) file, 0, PPC_MSG_READSOUND);
  PPCWaitPort(PPCPort);
  PPCGetMessage(PPCPort);

  PPCWaitPort(PPCPort);
  m68k_msg = PPCGetMessage(PPCPort);

  sound = (struct ASound *) PPCGetMessageAttr(m68k_msg, PPCMSGTAG_DATA);

  PPCReplyMessage(m68k_msg);

  if(sound)
    PPCCacheInvalid(sound, sizeof(struct ASound));

  return(sound);
}


struct ASound *ALoadSound(struct Audio *audio, UBYTE *sample, LONG res, LONG len, LONG freq, LONG vol)
{
  struct ASound *sound;
  void      *m68k_msg;

  PPCCacheFlush(sample, len);

  MsgData[PPC_MSGDATA_SAMPLE]   = (LONG) sample;
  MsgData[PPC_MSGDATA_RESOLUTION] = res;
  MsgData[PPC_MSGDATA_LENGTH]   = len;
  MsgData[PPC_MSGDATA_FREQUENCY]  = freq;
  MsgData[PPC_MSGDATA_VOLUME]   = vol;

  PPCSendMessage(M68kPort, MyMsg, MsgData, PPC_MSGSIZE_LOADSOUND, PPC_MSG_LOADSOUND);
  PPCWaitPort(PPCPort);
  PPCGetMessage(PPCPort);

  PPCWaitPort(PPCPort);
  m68k_msg = PPCGetMessage(PPCPort);

  sound = (struct ASound *) PPCGetMessageAttr(m68k_msg, PPCMSGTAG_DATA);

  PPCReplyMessage(m68k_msg);

  if(sound)
    PPCCacheInvalid(sound, sizeof(struct ASound));

  return(sound);
}
