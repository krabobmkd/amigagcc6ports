/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id: main.c,v 1.1 1999/04/28 18:54:28 meh Exp meh $
 *
 * $Log: main.c,v $
 * Revision 1.1  1999/04/28 18:54:28  meh
 * Initial revision
 *
 *
 *************************************************************************/

#include <stdio.h>
#include <strings.h>

#include <clib/alib_protos.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <workbench/workbench.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <cybergraphx/cybergraphics.h>

#include <inline/exec.h>
#include <inline/dos.h>
#include <inline/graphics.h>
#include <inline/intuition.h>
#include <inline/asl.h>
#include <inline/gadtools.h>
#include <inline/locale.h>
#include <inline/timer.h>

#ifdef POWERUP
#include <powerup/ppclib/interface.h>
#include <powerup/ppclib/message.h>
#include <powerup/ppclib/tasks.h>
#include <powerup/ppclib/memory.h>
#include <inline/ppc.h>
#endif

#define CATCOMP_BLOCK
#define CATCOMP_CODE

#include "main.h"
#include "version.h"
#include "audio.h"
#include "video.h"
#include "amiga_inputs.h"
#include "config.h"
#include "gui.h"

#include "osdepend.h"
#include "driver.h"
#include "file.h"

#define MIN_STACK (10*1024)

#define ITEM_NEW    0
#define ITEM_SAVE_ILBM  1
#define ITEM_ABOUT    3
#define ITEM_QUIT   5
#define NUM_ITEMS   6

void     Main(int argc, char **argv);
void ASM RefreshHandler(struct Hook *hook REG(a0));
void ASM MenuHandler(struct Hook *hook REG(a0), APTR null REG(a2), ULONG *itemnum REG(a1));
void ASM IDCMPHandler(struct Hook *hook REG(a0), APTR null REG(a2), ULONG *imclass REG(a1));
void     SaveILBM(void);
void     ErrorRequest(LONG msg_id, ...);

void     StartGame(void);  /* In amiga/amiga.c. */

#ifdef POWERUP
struct GameDriver **Drivers;
#endif

LONG        MenuSelect[NUM_ITEMS];
UBYTE       Channel[4];
static LONG NewGame;

UBYTE       ChannelBuffer[8];

struct DosLibrary   *DOSBase    = NULL;
struct Library      *GfxBase    = NULL;
struct Library      *CyberGfxBase = NULL;
struct IntuitionBase  *IntuitionBase  = NULL;
struct Library      *GadToolsBase = NULL;
struct Library      *AslBase    = NULL;
struct Library      *KeymapBase   = NULL;
struct Library      *UtilityBase  = NULL;
struct Library      *TimerBase    = NULL;

#ifdef POWERUP
struct Library      *PPCLibBase   = NULL;
#endif

struct FileRequester  *FileRequester  = NULL;

LONG          Width;
LONG          Height;
struct Audio      *Audio;
struct Video      *Video;
struct Inputs     *Inputs;
struct AChannelArray  *ChannelArray[2];
struct VPixelArray    *PixelArray[2];
struct VDirectArray   *DirectArray;
LONG          CurrentArray  = 0;
BYTE          *Keys;
struct IPort      *Port1;
struct IPort      *Port2;
UBYTE         *DirectPixels;
ULONG         DirectBytesPerRow;

struct timerequest    *TimerIO;
struct MsgPort      TimerMP;

struct LocaleInfo   LocaleInfo;

static struct Hook    RefreshHook;
static struct Hook    MenuHook;
static struct Hook    IDCMPHook;

static struct StackSwapStruct StackSwapStruct;

static struct NewMenu NewMenu[] =
{
  { NM_TITLE, (STRPTR) MSG_MENU_GAME,      NULL, 0,  0,  NULL  },
  { NM_ITEM,  (STRPTR) MSG_MENU_NEW,       "N",  0,  0,  NULL  },
  { NM_ITEM,  (STRPTR) MSG_MENU_SAVE_ILBM, "S",  0,  0,  NULL  },
  { NM_ITEM,  (STRPTR) NM_BARLABEL,        NULL, 0,  0,  NULL  },
  { NM_ITEM,  (STRPTR) MSG_MENU_ABOUT,     "?",  0,  0,  NULL  },
  { NM_ITEM,  (STRPTR) NM_BARLABEL,        NULL, 0,  0,  NULL  },
  { NM_ITEM,  (STRPTR) MSG_MENU_QUIT,      "Q",  0,  0,  NULL  },
  { NM_END,   NULL,                        NULL, 0,  0,  NULL  },
};

static struct Menu *Menu;

/*0.35 keycode system is obsolete
struct IKeyMap KeyMap[] =
{
  { OSD_KEY_ESC,        IKEY_ESC       },  { OSD_KEY_1,      '1'         },
  { OSD_KEY_2,          '2'            },  { OSD_KEY_3,      '3'         },
  { OSD_KEY_4,          '4'            },  { OSD_KEY_5,      '5'         },
  { OSD_KEY_6,          '6'            },  { OSD_KEY_7,      '7'         },
  { OSD_KEY_8,          '8'            },  { OSD_KEY_9,      '9'         },
  { OSD_KEY_0,          '0'            },  { OSD_KEY_MINUS,      '-'         },
  { OSD_KEY_EQUALS,     '='            },  { OSD_KEY_BACKSPACE, IKEY_BACKSPACE    },
  { OSD_KEY_TAB,        IKEY_TAB       },  { OSD_KEY_Q,      'q'         },
  { OSD_KEY_W,          'w'            },  { OSD_KEY_E,      'e'         },
  { OSD_KEY_R,          'r'            },  { OSD_KEY_T,      't'         },
  { OSD_KEY_Y,          'y'            },  { OSD_KEY_U,      'u'         },
  { OSD_KEY_I,          'i'            },  { OSD_KEY_O,      'o'         },
  { OSD_KEY_P,          'p'            },  { OSD_KEY_OPENBRACE,  '('         },
  { OSD_KEY_CLOSEBRACE, ')'            },  { OSD_KEY_ENTER,    IKEY_ENTER      },
  { OSD_KEY_LCONTROL,   IKEY_CONTROL   },  { OSD_KEY_A,      'a'         },
  { OSD_KEY_S,          's'            },  { OSD_KEY_D,      'd'         },
  { OSD_KEY_F,          'f'            },  { OSD_KEY_G,      'g'         },
  { OSD_KEY_H,          'h'            },  { OSD_KEY_J,      'j'         },
  { OSD_KEY_K,          'k'            },  { OSD_KEY_L,      'l'         },
  { OSD_KEY_COLON,      ':'            },  { OSD_KEY_QUOTE,    '"'         },
  { OSD_KEY_TILDE,      '`'            },  { OSD_KEY_LSHIFT,   IKEY_LSHIFT     },
  { OSD_KEY_Z,          'z'            },  { OSD_KEY_X,      'x'         },
  { OSD_KEY_C,          'c'            },  { OSD_KEY_V,      'v'         },
  { OSD_KEY_B,          'b'            },  { OSD_KEY_N,      'n'         },
  { OSD_KEY_M,          'm'            },  { OSD_KEY_COMMA,    ','         },
  { OSD_KEY_STOP,       '.'            },  { OSD_KEY_SLASH,    '/'         },
  { OSD_KEY_RSHIFT,     IKEY_RSHIFT,   },  { OSD_KEY_ASTERISK, '*'         },
  { OSD_KEY_ALT,        IKEY_RALT      },  { OSD_KEY_ALT,    IKEY_LALT     },
  { OSD_KEY_SPACE,      ' '            },  { OSD_KEY_CAPSLOCK, IKEY_CAPSLOCK,    },
  { OSD_KEY_F1,         IKEY_F1        },  { OSD_KEY_F2,     IKEY_F2       },
  { OSD_KEY_F3,         IKEY_F3        },  { OSD_KEY_F4,     IKEY_F4       },
  { OSD_KEY_F5,         IKEY_F5        },  { OSD_KEY_F6,     IKEY_F6       },
  { OSD_KEY_F8,         IKEY_F8        },  { OSD_KEY_F9,     IKEY_F9       },
  { OSD_KEY_F10,        IKEY_F10       },  { OSD_KEY_NUMLOCK,  IKEY_NONE     },
  { OSD_KEY_SCRLOCK,    IKEY_NONE      },  { OSD_KEY_HOME,   IKEY_NONE     },
  { OSD_KEY_UP,         IKEY_UP        },  { OSD_KEY_PGUP,   IKEY_NONE     },
  { OSD_KEY_MINUS_PAD,  IKEY_MINUS_PAD },  { OSD_KEY_LEFT,   IKEY_LEFT     },
  { OSD_KEY_5_PAD,      IKEY_5_PAD     },  { OSD_KEY_RIGHT,    IKEY_RIGHT      },
  { OSD_KEY_PLUS_PAD,   IKEY_PLUS_PAD  },  { OSD_KEY_END,    IKEY_NONE     },
  { OSD_KEY_DOWN,       IKEY_DOWN      },  { OSD_KEY_PGDN,   IKEY_NONE     },
  { OSD_KEY_INSERT,     IKEY_NONE      },  { OSD_KEY_DEL,    IKEY_DEL      },
  { OSD_KEY_F11,        IKEY_NONE      },  { OSD_KEY_F12,    IKEY_NONE     },
  { OSD_KEY_F7,         IKEY_F7        },  { OSD_MAX_KEY,    IKEY_NONE     },
  { 0,                  0              }
};
*/

int main(int argc, char **argv)
{
  struct Task *task;
  
  task  = FindTask(NULL);
/* krb: looks messy to me, original stack should be restored and alloc freed , in an atexit().
  if((task->tc_SPReg - task->tc_SPLower) < (MIN_STACK - 1024))
  {
    StackSwapStruct.stk_Lower = AllocVec(MIN_STACK, MEMF_PUBLIC);
    
    if(StackSwapStruct.stk_Lower)
    {
      StackSwapStruct.stk_Upper = (ULONG) StackSwapStruct.stk_Lower + MIN_STACK;
      StackSwapStruct.stk_Pointer = (APTR) StackSwapStruct.stk_Upper;

      StackSwap(&StackSwapStruct);
      
      Main(argc, argv);

      StackSwap(&StackSwapStruct);
    }
    //krb: where is FreeVec(StackSwapStruct.stk_Lower) ???
  }
  else
  */
   Main(argc, argv);

  return(0);
}

void Main(int argc, char **argv)
{
  ULONG i;
  ULONG quit;
#ifdef POWERUP
  struct ASound     *sound;
  void          *ppc_object;
  void          *ppc_task;
  void          *m68k_ports[2];
  void          *ppc_port;
  void          *ppc_msg;
  void          *msg_startup;
  void          *msg;
  void          *port_list;
  struct MsgStartupData *msg_startup_data;
  LONG          *msg_data;
  LONG          *ppc_msg_data;
  LONG          ppc_msg_id;
  LONG          reply;
#endif
  if((DOSBase = (struct DosLibrary *) OpenLibrary("dos.library", 39)))
  {
    if((GfxBase = OpenLibrary("graphics.library", 39)))
    {
      if((IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 39)))
      {
        if((UtilityBase = OpenLibrary("utility.library",0)))
        {
          if((KeymapBase = OpenLibrary("keymap.library", 36)))
          {
            if((AslBase = OpenLibrary("asl.library", 36)))
            {
#ifdef POWERUP
              if((PPCLibBase = OpenLibrary("ppc.library", 46)))
              {
#endif

              CyberGfxBase  = OpenLibrary("cybergraphics.library", 0);

              GadToolsBase  = OpenLibrary("gadtools.library", 0);

              if(GadToolsBase)
              {
                for(i = 0; NewMenu[i].nm_Type != NM_END; i++)
                {
                  if(((NewMenu[i].nm_Type == NM_TITLE) || (NewMenu[i].nm_Type == NM_ITEM))
                  && (NewMenu[i].nm_Label != NM_BARLABEL))
                    NewMenu[i].nm_Label = GetMessage((LONG) NewMenu[i].nm_Label);
                }

                Menu  = CreateMenus(NewMenu, GTMN_FullMenu, TRUE, TAG_END);
              }
              else
                Menu  = NULL;

              TimerBase   = NULL;

              TimerMP.mp_Node.ln_Type   = NT_MSGPORT;
              TimerMP.mp_Flags      = PA_IGNORE;
              NewList(&TimerMP.mp_MsgList);
              
              TimerIO = CreateIORequest(&TimerMP, sizeof(struct timerequest));
              if(TimerIO)
              {
                if(!OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *) TimerIO, 0))
                  TimerBase = (struct Library *) TimerIO->tr_node.io_Device;
              }

              LocaleInfo.li_LocaleBase = NULL;    /* Locale support is not
                                  ** implemented yet. */

#ifdef POWERUP
              if(TimerBase)
              {
#ifndef MESS
                ppc_object = PPCLoadObject("mameppc.elf");
#else
                ppc_object = PPCLoadObject("messppc.elf");
#endif                  
                if(ppc_object)
                {
                  m68k_ports[0] = PPCCreatePort(NULL);
                  m68k_ports[1] = NULL;
                  
                  if(m68k_ports[0])
                  {
                    msg_startup = PPCCreateMessage(m68k_ports[0], sizeof(struct MsgStartupData));
                    
                    if(msg_startup)
                    {
                      msg = PPCCreateMessage(m68k_ports[0], M68k_MSGSIZE_MAX);

                      if(msg)                   
                      {
                        msg_startup_data = PPCAllocVec(sizeof(struct MsgStartupData), MEMF_ANY);

                        if(msg_startup_data)
                        {
                          msg_data = PPCAllocVec(M68k_MSGSIZE_MAX, MEMF_ANY);

                          if(msg_data)
                          {
                            msg_startup_data->M68kPort  = m68k_ports[0];
                            msg_startup_data->Version = VERNUM;
                            msg_startup_data->Revision  = REVNUM;
                            
                            ppc_task = PPCCreateTaskTags( ppc_object,
                                            PPCTASKTAG_MSGPORT,       TRUE,
                                            PPCTASKTAG_STARTUP_MSG,     (ULONG) msg_startup,
                                            PPCTASKTAG_STARTUP_MSGDATA,   (ULONG) msg_startup_data,
                                            PPCTASKTAG_STARTUP_MSGLENGTH, 0,
                                            PPCTASKTAG_STARTUP_MSGID,   M68k_MSG_STARTUP,
                                            PPCTASKTAG_STACKSIZE,     204800,
                                            TAG_DONE);

                            if(ppc_task)
                            {
                              ppc_port = (void *) PPCGetTaskAttrsTags(ppc_task, PPCTASKINFOTAG_MSGPORT, 0, TAG_DONE);
                              
                              PPCWaitPort(m68k_ports[0]);
                              ppc_msg = PPCGetMessage(m68k_ports[0]);
                                                    
                              if(ppc_msg != msg_startup)
                              {
                                Drivers = (struct GameDriver **) PPCGetMessageAttr(ppc_msg, PPCMSGTAG_DATA);
                                
                                PPCReplyMessage(ppc_msg);
                                
                                CacheClearU();
                            
                                /* Now it should be safe to access Drivers. */
#endif
                quit = AllocConfig(argc, argv);
                
                if(!quit)
                {
                  AllocGUI();
                  LoadConfig(argc, argv);

                  if(Config[CFG_DRIVER] < 0)
                  {
                    GetConfig(0, Config);
                    quit = MainGUI();
                  }
                  else
                    quit = FALSE;
#ifndef POWERUP
                  while(!quit)
#else
                  while(1)
#endif                    
                  {
#ifdef MESS
                    GetConfig(Config[CFG_DRIVER], Config);
#else
                    GetConfig(Config[CFG_DRIVER]+2, Config);

                    if(Config[CFG_USEDEFAULTS])
                    {   
                      if(Drivers[Config[CFG_DRIVER]]->drv->video_attributes & VIDEO_TYPE_VECTOR)
                        GetConfig(1, Config);
                      else
                        GetConfig(0, Config);
                    }
#endif
                    
                    NewGame     = 1;
                    Inputs      = NULL;
                    Keys      = NULL;
                    Audio     = NULL;
                    ChannelArray[0] = NULL;
                    ChannelArray[1] = NULL;

                    if(!quit && (Config[CFG_SOUND] != CFGS_NO))
                    {
                      Audio = AllocAudio( AA_UseAHI,    (Config[CFG_SOUND] == CFGS_AHI) ? TRUE : FALSE,
                                AA_Channels,  AUDIO_CHANNELS,
                                AA_MaxSounds, 255,
                                AA_MinFreeChip, Config[CFG_MINFREECHIP]*1024,
                                TAG_END);
                      
                      if(Audio)
                      {
                        ChannelArray[0] = AAllocChannelArray(Audio, AUDIO_BUFFER_LENGTH);
#ifdef POWERUP
                        ChannelArray[1] = AAllocChannelArray(Audio, AUDIO_BUFFER_LENGTH);
#endif
                      }
                    }
#ifdef POWERUP
                    msg_data[M68k_MSGDATA_CHANNELARRAY1]  = (LONG) ChannelArray[0];
                    msg_data[M68k_MSGDATA_CHANNELARRAY2]  = (LONG) ChannelArray[1];

                    if(ChannelArray[0])
                      msg_data[M68k_MSGDATA_CHANNELARRAYSIZE] = ChannelArray[0]->Size;

                    for(i = 0; i < CFG_ITEMS; i++)
                      msg_data[M68k_MSGDATA_CONFIG + i] = Config[i];

                    if(quit)
                    {
                      PPCSendMessage(ppc_port, msg, 0, 0, M68k_MSG_CONFIG);
                      PPCWaitPort(m68k_ports[0]);
                      PPCGetMessage(m68k_ports[0]);
                    }
                    else
                    {                   
                      CacheClearU();
                      
                      PPCSendMessage(ppc_port, msg, msg_data, M68k_MSGSIZE_CONFIG, M68k_MSG_CONFIG);
                      PPCWaitPort(m68k_ports[0]);
                      PPCGetMessage(m68k_ports[0]);
                    }

                    port_list = NULL;

                    do
                    {
                      if(port_list)
                      {
                        ppc_msg = PPCWaitPortList(port_list);
                        
                        i = PPCGetPortListAttr(port_list, PPCPORTLISTTAG_RECEIVEDSIGNALS);

                        if(i & Inputs->SignalMask)
                          InputUpdate(FALSE);
                      }
                      else
                        ppc_msg = PPCWaitPort(m68k_ports[0]);
                      
                      if(ppc_msg)
                        ppc_msg = PPCGetMessage(m68k_ports[0]);

                      if(ppc_msg != msg_startup)
                      {
                        if(ppc_msg)
                        {
                          ppc_msg_id    = PPCGetMessageAttr(ppc_msg, PPCMSGTAG_MSGID);
                          ppc_msg_data  = (LONG *) PPCGetMessageAttr(ppc_msg, PPCMSGTAG_DATA);
                        
                          switch(ppc_msg_id)
                          {
                            case PPC_MSG_VIDEOOPEN:
                              msg_data[M68k_MSGDATA_RESULT] = VideoOpen(ppc_msg_data[PPC_MSGDATA_WIDTH],
                                                                        ppc_msg_data[PPC_MSGDATA_HEIGHT],
                                                                        ppc_msg_data[PPC_MSGDATA_LEFT],
                                                                        ppc_msg_data[PPC_MSGDATA_TOP],
                                                                        ppc_msg_data[PPC_MSGDATA_RIGHT],
                                                                        ppc_msg_data[PPC_MSGDATA_BOTTOM],
                                                                        ppc_msg_data[PPC_MSGDATA_DIRTY]);

                              PPCReplyMessage(ppc_msg);

                              if(msg_data[M68k_MSGDATA_RESULT])
                              {
                                msg_data[M68k_MSGDATA_WIDTH]  = Width;
                                msg_data[M68k_MSGDATA_HEIGHT] = Height;

                                msg_data[M68k_MSGDATA_PIXELARRAY1]      = (LONG) PixelArray[0];
                                msg_data[M68k_MSGDATA_PIXELARRAY2]      = (LONG) PixelArray[1];

                                msg_data[M68k_MSGDATA_DIRECTARRAY]      = (LONG) DirectArray;

                                msg_data[M68k_MSGDATA_KEYS]   = (LONG) Keys;
                                msg_data[M68k_MSGDATA_PORT1]  = (LONG) Port1;
                                msg_data[M68k_MSGDATA_PORT2]  = (LONG) Port2;
                                
                                port_list = PPCCreatePortList(m68k_ports, Inputs->SignalMask);
                              }

                              PPCSendMessage(ppc_port, msg, msg_data, M68k_MSGSIZE_VIDEODATA, M68k_MSG_VIDEODATA);
                              PPCWaitPort(m68k_ports[0]);
                              PPCGetMessage(m68k_ports[0]);
                              
                              break;

                            case PPC_MSG_VIDEOCLOSE:
                              PPCReplyMessage(ppc_msg);
                              VideoClose();

                              if(port_list)
                                PPCDeletePortList(port_list);
                              
                              port_list = NULL;

                              break;

                            case PPC_MSG_INPUTUPDATE:
                              InputUpdate(TRUE);
                              PPCReplyMessage(ppc_msg);
                              break;

                            case PPC_MSG_SETPIXELFRAME:
                              if(Config[CFG_ASYNCPPC])
                              {
                                PPCReplyMessage(ppc_msg);
                              
                                PPCSendMessage(ppc_port, msg, (void *) ((VGetFrameSkip(Video)<<16)|VGetFPS(Video)), 0, M68k_MSG_FRAMEDATA);
                                PPCWaitPort(m68k_ports[0]);
                                PPCGetMessage(m68k_ports[0]);
                              }

                              i = ((ULONG) ppc_msg_data) & 0xf;

                              VInvalidPixelArray(PixelArray[i]);

                              VSetFrameSkip(Video, (((LONG) ppc_msg_data) >> 4) & 0xf);
                              VSetLimitSpeed(Video, (((LONG) ppc_msg_data) >> 8) & 0xf);

                              InputUpdate(FALSE);
                              
                              VSetPixelFrame(PixelArray[i]);
                                
                              if(ChannelArray[0] && ChannelArray[1])
                              {
                                PPCCacheInvalidE(ChannelArray[i], ChannelArray[i]->Size, CACRF_ClearD);
                                ASetChannelFrame(ChannelArray[i]);
                              }

                              if(!Config[CFG_ASYNCPPC])
                              {
                                PPCReplyMessage(ppc_msg);
                              
                                PPCSendMessage(ppc_port, msg, (void *) ((VGetFrameSkip(Video)<<16)|VGetFPS(Video)), 0, M68k_MSG_FRAMEDATA);
                                PPCWaitPort(m68k_ports[0]);
                                PPCGetMessage(m68k_ports[0]);
                              }
                              break;

                            case PPC_MSG_SETDIRECTFRAME:
                              PPCReplyMessage(ppc_msg);

                              if(ChannelArray[0] && ChannelArray[1])
                              {
                                PPCCacheInvalidE(ChannelArray[i], ChannelArray[i]->Size, CACRF_ClearD);
                                ASetChannelFrame(ChannelArray[i]);
                              }

                              i = ((ULONG) ppc_msg_data) & 0xf;
                              
                              VSetFrameSkip(Video, (((LONG) ppc_msg_data) >> 4) & 0xf);
                              VSetLimitSpeed(Video, (((LONG) ppc_msg_data) >> 8) & 0xf);

                              InputUpdate(FALSE);
                              
                              VSetDirectFrame(DirectArray);
                                                              
                              PPCSendMessage(ppc_port, msg, (void *) ((VGetFrameSkip(Video)<<16)|VGetFPS(Video)), 0, M68k_MSG_FRAMEDATA);
                              PPCWaitPort(m68k_ports[0]);
                              PPCGetMessage(m68k_ports[0]);

                              break;

                            case PPC_MSG_OPENFILETYPE:
                              reply = (LONG) OpenFileType(  (const char *) &ppc_msg_data[PPC_MSGDATA_DIRNAME],
                                              (const char *) &ppc_msg_data[PPC_MSGDATA_FILENAME],
                                              ppc_msg_data[PPC_MSGDATA_MODE],
                                              ppc_msg_data[PPC_MSGDATA_TYPE]);
                              PPCReplyMessage(ppc_msg);

                              PPCSendMessage(ppc_port, msg, (void *) reply, 0, M68k_MSG_FILE);
                              PPCWaitPort(m68k_ports[0]);
                              PPCGetMessage(m68k_ports[0]);
                              break;
                            
                            case PPC_MSG_CLOSEFILE:
                              PPCReplyMessage(ppc_msg);
                              CloseFile((struct File *) ppc_msg_data);
                              break;
                            
                            case PPC_MSG_READSOUND:
                              PPCReplyMessage(ppc_msg);
                              PPCSendMessage(ppc_port, msg, AReadSound(Audio, (BPTR) ppc_msg_data), 0, M68k_MSG_FILE);
                              PPCWaitPort(m68k_ports[0]);
                              PPCGetMessage(m68k_ports[0]);
                              break;

                            case PPC_MSG_LOADSOUND:
                              PPCCacheInvalidE((APTR) ppc_msg_data[PPC_MSGDATA_SAMPLE], ppc_msg_data[PPC_MSGDATA_LENGTH], CACRF_ClearD);
                            
                              sound = ALoadSound(Audio, (APTR) ppc_msg_data[PPC_MSGDATA_SAMPLE],
                                            ppc_msg_data[PPC_MSGDATA_RESOLUTION],
                                            ppc_msg_data[PPC_MSGDATA_LENGTH],
                                            ppc_msg_data[PPC_MSGDATA_FREQUENCY],
                                            ppc_msg_data[PPC_MSGDATA_VOLUME]);
                              PPCReplyMessage(ppc_msg);

                              PPCSendMessage(ppc_port, msg, sound, 0, M68k_MSG_FILE);
                              PPCWaitPort(m68k_ports[0]);
                              PPCGetMessage(m68k_ports[0]);
                              break;

                            case PPC_MSG_QUIT:
                              PPCReplyMessage(ppc_msg);
                              quit = TRUE;
                              break;
                          }
                        }
                      }
                    } while((ppc_msg != msg_startup) && !quit);
#else
                    StartGame();
#endif
                    if(Audio)
                      FreeAudio(Audio);
#ifdef POWERUP
                    if(ppc_msg == msg_startup)
                      break;
#endif                    
                    if(NewGame > 0)
                      quit = MainGUI();
                    else if(!NewGame)
                      quit = TRUE;
                  }

                  FreeGUI();                  
                }

                FreeConfig();
#ifdef POWERUP
                              }
                            }
                            else
                              ErrorRequest(MSG_FAILED_TO_CREATE_PPC_TASK);

                            PPCFreeVec(msg_data);
                          }

                          PPCFreeVec(msg_startup_data);
                        }
                        
                        PPCDeleteMessage(msg);
                      }

                      PPCDeleteMessage(msg_startup);
                    }
                    
                    PPCDeletePort(m68k_ports[0]);
                  }
                  
                  PPCUnLoadObject(ppc_object);
                }
                else
                  ErrorRequest(MSG_FAILED_TO_LOAD_ELF);
              }
#endif
              if(TimerIO)
              {
                if(TimerBase)
                  CloseDevice((struct IORequest *) TimerIO);
                DeleteIORequest((struct IORequest *) TimerIO);
              }

              if(CyberGfxBase)
                CloseLibrary(CyberGfxBase);
              if(GadToolsBase)
                CloseLibrary(GadToolsBase);
              if(FileRequester) 
                FreeAslRequest(FileRequester);
              CloseLibrary(AslBase);

#ifdef POWERUP
                CloseLibrary(PPCLibBase);
              }
              else
                ErrorRequest(MSG_REQUIRES_LIB, "ppc.library", 46);
#endif
            }
            CloseLibrary(KeymapBase);
          }
          CloseLibrary(UtilityBase);
        }
        CloseLibrary((struct Library *) IntuitionBase);
      }
      CloseLibrary(GfxBase);
    }
    CloseLibrary((struct Library *) DOSBase);
  }
}

LONG VideoOpen(LONG width, LONG height, LONG left, LONG top, LONG right, LONG bottom, LONG dirty)
{

// #define	GAME_REQUIRES_16BIT			0x0100	/* cannot fit in 256 colors */
  static ULONG pixel_formats[] =
  {
    PIXFMT_RGB16,
    PIXFMT_RGB15,
    PIXFMT_BGR15,
    PIXFMT_RGB15PC,
    PIXFMT_BGR15PC,
    PIXFMT_BGR16,
    PIXFMT_RGB16PC,
    PIXFMT_BGR16PC,
    PIXFMT_LUT8,
    ~0
  };

  int  fail;
  LONG visible_width;
  LONG visible_height;


  if(Config[CFG_DIRECTMODE] == CFGDM_DRAW)
  {
    visible_width  = width + 16;
    visible_height = height + 16;
  }
  else
  {
    visible_width  = right - left + 1;
    visible_height = bottom - top + 1;
  }

  /* Disable dirty line support if requested by user. */

  if(!Config[CFG_DIRTYLINES])
    dirty = 0;

  Video = AllocVideo(VA_UseScreen,     (Config[CFG_SCREENTYPE] != CFGST_WB),
                     VA_UseScreenReq,  (Config[CFG_SCREENTYPE] == CFGST_USERSELECT),
                     VA_Width,         (Config[CFG_WIDTH] > visible_width)
                                       ? Config[CFG_WIDTH]
                                       : visible_width,
                     VA_Height,        (Config[CFG_HEIGHT] > visible_height)
                                       ? Config[CFG_HEIGHT]
                                       : visible_height,
                     VA_ModeID,        (Config[CFG_SCREENTYPE] == CFGST_CUSTOM)
                                       ? (Config[CFG_SCREENMODE])
                                       : INVALID_ID,
                     VA_Depth,         (Config[CFG_SCREENTYPE] == CFGST_CUSTOM)
                                       ? (Config[CFG_DEPTH])
                                       : 0,
                     VA_Buffers,       Config[CFG_BUFFERING]+1,
                     VA_Title,         APPNAME,
                     VA_Menu,          Menu,
                     VA_FPS,           Drivers[Config[CFG_DRIVER]]->drv->frames_per_second,
                     VA_MaxColors,     (Drivers[Config[CFG_DRIVER]]->drv->total_colors < 256)
                                       ? Drivers[Config[CFG_DRIVER]]->drv->total_colors
                                       : ((Drivers[Config[CFG_DRIVER]]->drv->video_attributes & VIDEO_SUPPORTS_16BIT)
                                         && Config[CFG_ALLOW16BIT])
                                         ? (1<<16)
                                         : 256,
                     VA_AutoFrameSkip, Config[CFG_AUTOFRAMESKIP],
                     VA_MaxFrameSkip,  4,
                     TAG_END);

  if(Video)
  {
    DirectArray = NULL;
    Inputs      = NULL;
  
    if(Config[CFG_DIRECTMODE])
    {
      DirectArray = VAllocDirectArray(Video, visible_width, visible_height);
      
      if(DirectArray)
      {
        VSetDirectFrame(DirectArray);
        
        if(!DirectArray->Pixels)
        {
          VFreeDirectArray(DirectArray);
          DirectArray = NULL;
        }
      }
    }

    fail = 0;

    if(!DirectArray)
    {
      if((Drivers[Config[CFG_DRIVER]]->drv->video_attributes & VIDEO_SUPPORTS_16BIT)
         && Config[CFG_ALLOW16BIT])
      {
        PixelArray[0] = VAllocPixelArray(Video, width+16, height+16, dirty, pixel_formats);
      }
      else
        PixelArray[0] = VAllocPixelArray(Video, width+16, height+16, dirty, NULL);
      
      if(PixelArray[0])
      {
        VSetPixelArrayBox(PixelArray[0], 8 + left, 8 + top, 8 + right, 8 + bottom);
      }
      else
        fail = 1;
    }
    else
      PixelArray[0] = NULL;

#ifdef POWERUP
    if(!DirectArray)
    {
      if((Drivers[Config[CFG_DRIVER]]->drv->video_attributes & VIDEO_SUPPORTS_16BIT)
         && Config[CFG_ALLOW16BIT])
      {
        PixelArray[1] = VAllocPixelArray(Video, width+16, height+16, dirty, pixel_formats);
      }
      else
        PixelArray[1] = VAllocPixelArray(Video, width+16, height+16, dirty, NULL);

      if(PixelArray[1])
      {
        VSetPixelArrayBox(PixelArray[1], 8 + left, 8 + top, 8 + right, 8 + bottom);
      }
      else
        fail = 1;
    }
    else
      PixelArray[1] = NULL;
#endif

    if(!fail)
    {
      RefreshHook.h_Entry = (HOOKFUNC) RefreshHandler;
      MenuHook.h_Entry    = (HOOKFUNC) MenuHandler;
      IDCMPHook.h_Entry   = (HOOKFUNC) IDCMPHandler;

      Inputs = AllocInputs(IA_Port1,          (Config[CFG_JOY1TYPE] == CFGJ1_MOUSE1)
                                              ? IPT_MOUSE
                                              : Config[CFG_JOY2TYPE],
                           IA_Port2,          (Config[CFG_JOY1TYPE] == CFGJ1_MOUSE1)
                                              ? IPT_NONE
                                              : Config[CFG_JOY1TYPE],
                           IA_KeyMap,         (ULONG) KeyMap,
                           IA_P1AutoFireRate, (Config[CFG_JOY1TYPE] == CFGJ1_MOUSE1)
                                              ? Config[CFG_JOY1AUTOFIRERATE]
                                              : Config[CFG_JOY2AUTOFIRERATE],
                           IA_P1BlueEmuTime,  (Config[CFG_JOY1TYPE] == CFGJ1_MOUSE1)
                                              ? Config[CFG_JOY1BUTTONBTIME]
                                              : Config[CFG_JOY2BUTTONBTIME],
                           IA_P2AutoFireRate, Config[CFG_JOY1AUTOFIRERATE],
                           IA_P2BlueEmuTime,  Config[CFG_JOY1BUTTONBTIME],
                           IA_Window,         Video->Window,
                           IA_RefreshHook,    (ULONG) &RefreshHook,
                           IA_MenuHook,       (ULONG) &MenuHook,
                           IA_IDCMPHook,      (ULONG) &IDCMPHook,
#ifdef POWERUP
                           IA_UseTicks,       TRUE,
#endif
                           TAG_END);


      if(Inputs)
      {
        Width  = Video->Width;
        Height = Video->Height;
        Keys   = Inputs->Keys;
        Port1  = Inputs->Ports[1];
        Port2  = Inputs->Ports[0];
        return(1);
      }
      else
        ErrorRequest(MSG_FAILED_TO_ALLOCATE_INPUTS);
    }
    else
      ErrorRequest(MSG_NOT_ENOUGH_MEMORY);

    if(DirectArray)
      VFreeDirectArray(DirectArray);
    if(PixelArray[0])
      VFreePixelArray(PixelArray[0]);
#ifdef POWERUP
    if(PixelArray[1])
      VFreePixelArray(PixelArray[1]);
#endif

    FreeVideo(Video);
  }
  else
  {
    switch(VError)
    {
      case None:
        break;
        
      case OutOfMemory:
        ErrorRequest(MSG_NOT_ENOUGH_MEMORY);
        break;

      case OpenScreenFailed:
        ErrorRequest(MSG_FAILED_TO_OPEN_SCREEN);
        break;

      case OpenWindowFailed:
        ErrorRequest(MSG_FAILED_TO_OPEN_WINDOW);
        break;
    }
  }

  return(0);
}

void VideoClose(void)
{
  if(Video)
  {
    if(DirectArray)
      VFreeDirectArray(DirectArray);
    if(PixelArray[0])
      VFreePixelArray(PixelArray[0]);
#ifdef POWERUP
    if(PixelArray[1])
      VFreePixelArray(PixelArray[1]);
#endif

    if(Inputs)
      FreeInputs(Inputs);

    FreeVideo(Video);
    Video = NULL;
  }
}

void InputUpdate(LONG wait)
{
  if(Inputs)
  {
    if(wait)
      Wait(Inputs->SignalMask);

    IUpdate(Inputs);

    if(MenuSelect[ITEM_NEW])
    {
      MenuSelect[ITEM_NEW] = 0;

      NewGame              = 1;
      Keys[OSD_KEY_ESC]    = 1;
    }

    if(MenuSelect[ITEM_SAVE_ILBM])
    {
      MenuSelect[ITEM_SAVE_ILBM] = 0;
      IDisable(Inputs);
      SaveILBM();
      ScreenToFront(Video->Window->WScreen);
      ActivateWindow(Video->Window);
      IEnable(Inputs);
    }

    if(MenuSelect[ITEM_ABOUT])
    {
      MenuSelect[ITEM_ABOUT]  = 0;
      IDisable(Inputs);
      AboutGUI();
      ScreenToFront(Video->Window->WScreen);
      ActivateWindow(Video->Window);
      IEnable(Inputs);
    }

    if(MenuSelect[ITEM_QUIT])
    {
      MenuSelect[ITEM_QUIT] = 0;

      NewGame           = 0;
      Keys[OSD_KEY_ESC] = 1;
    }
  }
}

void ASM RefreshHandler(struct Hook *hook REG(a0))
{
  VRefresh(Video);
}

void ASM MenuHandler(struct Hook *hook REG(a0), APTR null REG(a2), ULONG *itemnum REG(a1))
{
  MenuSelect[*itemnum]  = 1;
}

void ASM IDCMPHandler(struct Hook *hook REG(a0), APTR null REG(a2), ULONG *imclass REG(a1))
{
  if(*imclass == IDCMP_CLOSEWINDOW)
  {
    NewGame           = 1;
    Keys[OSD_KEY_ESC] = 1;
  }
}

STRPTR GetMessage(LONG id)
{
  return(GetString(&LocaleInfo, id));
}

/*
uclock_t uclock(void)
{
  static uclock_t old_uclock    = 0;
  uclock_t    new_uclock;
  struct timeval  tv;

  if(TimerBase)
  {
    GetSysTime(&tv);
    new_uclock  = (tv.tv_secs * UCLOCKS_PER_SEC) + (tv.tv_micro / (1000000 / UCLOCKS_PER_SEC));
    if(new_uclock == old_uclock)
      new_uclock++;
  }
  else
    new_uclock  = old_uclock + UCLOCKS_PER_SEC;

  old_uclock  = new_uclock;

  return(new_uclock);
}
*/

void SaveILBM(void)
{
  BPTR new_dir;
  BPTR old_dir;

  if(!FileRequester)
    FileRequester = AllocAslRequest(ASL_FileRequest, NULL);
  
  if(FileRequester)
  {
    if(AslRequestTags(FileRequester,
                      ASLFR_DoSaveMode,  TRUE,
                      ASLFR_SleepWindow, TRUE,
                      ASLFR_TitleText,   (ULONG) GetMessage(MSG_MENU_SAVE_ILBM),
                      TAG_END))
    {
      new_dir = Lock(FileRequester->fr_Drawer, ACCESS_READ);
      
      if(new_dir)
      {
        old_dir = CurrentDir(new_dir);
        
        VSaveILBM(Video, FileRequester->fr_File);
        
        CurrentDir(old_dir);
        
        UnLock(new_dir);
      }
    }
  }
}

void ErrorRequest(LONG msg_id, ...)
{
  struct EasyStruct es;
  
  es.es_StructSize   = sizeof(struct EasyStruct);
  es.es_Flags        = 0;
  es.es_Title        = APPNAME;
  es.es_TextFormat   = GetMessage(msg_id);
  es.es_GadgetFormat = GetMessage(MSG_OK);
  
  EasyRequestArgs(NULL, &es, NULL, &((&msg_id)[1]));
}
