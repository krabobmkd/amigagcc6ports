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

/*
 krb note:
 mame's input.h and amiga intuition.h collides because of KEYCODE_XXX defines.
 let's make a point not including both in the same files.
*/

//#define CATCOMP_BLOCK
//#include "mame_msg.h" -> called by main.h

#define CATCOMP_NUMBERS
#include "messages.h"

#include "main.h"

#include <stdio.h>
#include <strings.h>

#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/timer.h>

#include <proto/asl.h>
#include <proto/utility.h>
#include <proto/keymap.h>
//#include <proto/cybergraphics.h>

extern "C" {
// all C amiga stuffs should be included from C++ in extern "C" paragraph
#include <cybergraphx/cybergraphics.h>
#include <macros.h>

// end extern "C"
}


#include "version.h"
#include "audio.h"
#include "video.h"
#include "amiga_inputs.h"
#include "amiga_locale.h"
#include "config_moo.h"
#include "gui_mui.h"
#include "gui_gadtools.h"

#include "osdepend.h"
#include "driver.h"
#include "file.h"




typedef ULONG (*RE_HOOKFUNC)();

#define MIN_STACK (10*1024)

#define ITEM_NEW    0
#define ITEM_SAVE_ILBM  1
#define ITEM_ABOUT    3
#define ITEM_QUIT   5
#define NUM_ITEMS   6

extern "C" {
void     Main(int argc, char **argv);
void ASM RefreshHandler(struct Hook *hook REG(a0));
void ASM MenuHandler(struct Hook *hook REG(a0), APTR null REG(a2), ULONG *itemnum REG(a1));
void ASM IDCMPHandler(struct Hook *hook REG(a0), APTR null REG(a2), ULONG *imclass REG(a1));
void     SaveILBM(void);
void     ErrorRequest(LONG msg_id, ...);

}
void     StartGame(void);  /* In amiga/amiga.c. */

#ifdef POWERUP
struct GameDriver **Drivers;
#endif

LONG        MenuSelect[NUM_ITEMS];
UBYTE       Channel[4];
static LONG NewGame;

UBYTE       ChannelBuffer[8];

extern "C" {
#ifdef USE_OWN_DOSBASE
struct DosLibrary   *DOSBase    = NULL;
#endif
struct GfxBase      *GfxBase    = NULL;
struct IntuitionBase  *IntuitionBase  = NULL;
struct Library      *GadToolsBase = NULL;
struct Library      *AslBase    = NULL;
struct Library      *KeymapBase   = NULL;
struct Library      *UtilityBase  = NULL;
struct Library      *CyberGfxBase = NULL;

struct Device      *TimerBase    = NULL;
}

struct FileRequester  *FileRequester  = NULL;

LONG          Width;
LONG          Height;
struct Audio      *Audio=NULL;
struct Video      *Video=NULL;
struct Inputs     *Inputs=NULL;
struct AChannelArray  *ChannelArray[2];
struct VPixelArray    *PixelArray[2];
struct VDirectArray   *DirectArray;
LONG          CurrentArray  = 0;
BYTE          *Keys=NULL;
struct IPort      *Port1=NULL;
struct IPort      *Port2=NULL;
UBYTE         *DirectPixels;
ULONG         DirectBytesPerRow;

struct timerequest    *TimerIO;
struct MsgPort      TimerMP;

static struct Hook    RefreshHook;
static struct Hook    MenuHook;
static struct Hook    IDCMPHook;

static struct StackSwapStruct StackSwapStruct;

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


int libs_init()
{
#ifdef USE_OWN_DOSBASE
    if(!(DOSBase = (struct DosLibrary *) OpenLibrary("dos.library", 36))) return(1);
#endif
    if(!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 39)))
    {
        printf("need at least OS3.0\n");
        return(1);
    }
    if(!(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 39)))
    {
        return(1);
    }
    if(!(UtilityBase = OpenLibrary("utility.library",0))) return(1);
    if(!(KeymapBase = OpenLibrary("keymap.library", 36))) return(1);
    if(!(AslBase = OpenLibrary("asl.library", 36))) return(1);

    // optional:
    CyberGfxBase  = OpenLibrary("cybergraphics.library", 1);
    GadToolsBase  = OpenLibrary("gadtools.library", 1);

    if(GadToolsBase) gui_gadtools_init();

    // - - - - - - - timer init. Most likely to work
    {
        TimerMP.mp_Node.ln_Type   = NT_MSGPORT;
        TimerMP.mp_Flags      = PA_IGNORE;
        NewList(&TimerMP.mp_MsgList);

        TimerIO = (struct timerequest*)CreateIORequest(&TimerMP, sizeof(struct timerequest));
        if(TimerIO)
        {
        if(!OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *) TimerIO, 0))
          TimerBase = TimerIO->tr_node.io_Device;
        }
    }

    return(0);
}

void unzip_cache_clear();

// exit code that is executed in all cases:
// - after main()
// - when anything call exit()
// - SIGTERM signal (->to be managed)
void main_close()
{
    unzip_cache_clear();
    if(Audio)
    {
      FreeAudio(Audio);
      Audio = NULL;
    }
    printf("does main_close\n");
    if(TimerIO)
    {
        if(TimerBase)
          CloseDevice((struct IORequest *) TimerIO);
        DeleteIORequest((struct IORequest *) TimerIO);
    }
    FreeGUI();
    FreeConfig();

    gui_gadtools_close();

    if(GadToolsBase) CloseLibrary(GadToolsBase);
    if(CyberGfxBase) CloseLibrary(CyberGfxBase);

#ifdef POWERUP
    if(PPCLibBase) CloseLibrary(PPCLibBase);
#endif
    if(KeymapBase) CloseLibrary(KeymapBase);
    if(UtilityBase) CloseLibrary(UtilityBase);
    if(IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    if(GfxBase) CloseLibrary((struct Library *)GfxBase);
// done in  by compiler runtime or not.
#ifdef USE_OWN_DOSBASE
   if(DOSBase) CloseLibrary((struct Library *)DOSBase);
#endif
}



int main(int argc, char **argv)
{

/* krb: looks messy to me, original stack should be restored and alloc freed , in an atexit().
  task  = FindTask(NULL);
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
    // any exit case will lead to close().
    atexit(&main_close);
    if(libs_init()!=0) exit(1);

    if(GadToolsBase) gui_gadtools_init();

    if(AllocConfig(argc, argv) !=0 ) exit(1);

    AllocGUI();
    LoadConfig(argc, argv);

    if(Config[CFG_DRIVER] < 0)
    {
        GetConfig(0, Config);
        // go into interface loop
        if(MainGUI()!=0) exit(0);
    }

    ULONG quit=FALSE;

    // loop per emulation launched
    while(!quit)
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


    StartGame();

    if(Audio)
    {
      FreeAudio(Audio);
      Audio = NULL;
    }
    if(NewGame > 0)
      quit = MainGUI();
    else if(!NewGame)
      quit = TRUE;
    } // end loop by emulation launch


    // end of main
    return(0);
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
                     VA_Menu,          g_gtMenu,
                     VA_FPS,           Drivers[Config[CFG_DRIVER]]->drv->frames_per_second,
                     VA_MaxColors,     // we only manage 8b or 16b
                                        // yet VIDEO_NEEDS_6BITS_PER_GUN would mean better in 24b
                                        (Drivers[Config[CFG_DRIVER]]->drv->total_colors <= 256)
                                        ?(Drivers[Config[CFG_DRIVER]]->drv->total_colors):(1<<16)

                                        /*(Drivers[Config[CFG_DRIVER]]->drv->total_colors <= 256)
                                       ? Drivers[Config[CFG_DRIVER]]->drv->total_colors
                                       : ((Drivers[Config[CFG_DRIVER]]->drv->video_attributes & VIDEO_NEEDS_6BITS_PER_GUN)
                                         && Config[CFG_ALLOW16BIT])
                                         ? (1<<24)
                                         : (1<<16)

                                        */,
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
      /*old0.35: if((Drivers[Config[CFG_DRIVER]]->drv->video_attributes & VIDEO_SUPPORTS_16BIT)
         && Config[CFG_ALLOW16BIT])*/
      if(Drivers[Config[CFG_DRIVER]]->drv->total_colors >256)
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
      RefreshHook.h_Entry = (RE_HOOKFUNC) RefreshHandler;
      MenuHook.h_Entry    = (RE_HOOKFUNC) MenuHandler;
      IDCMPHook.h_Entry   = (RE_HOOKFUNC) IDCMPHandler;

      Inputs = AllocInputs(IA_Port1,          (Config[CFG_JOY1TYPE] == CFGJ1_MOUSE1)
                                              ? IPT_MOUSE
                                              : Config[CFG_JOY2TYPE],
                           IA_Port2,          (Config[CFG_JOY1TYPE] == CFGJ1_MOUSE1)
                                              ? IPT_NONE
                                              : Config[CFG_JOY1TYPE],
                      //TODO: to be replaced     IA_KeyMap,         (ULONG) KeyMap,
//                           IA_P1AutoFireRate, (Config[CFG_JOY1TYPE] == CFGJ1_MOUSE1)
//                                              ? Config[CFG_JOY1AUTOFIRERATE]
//                                              : Config[CFG_JOY2AUTOFIRERATE],
//                           IA_P1BlueEmuTime,  (Config[CFG_JOY1TYPE] == CFGJ1_MOUSE1)
//                                              ? Config[CFG_JOY1BUTTONBTIME]
//                                              : Config[CFG_JOY2BUTTONBTIME],
//                           IA_P2AutoFireRate, Config[CFG_JOY1AUTOFIRERATE],
//                           IA_P2BlueEmuTime,  Config[CFG_JOY1BUTTONBTIME],
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
   //re   Keys[OSD_KEY_ESC]    = 1;
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
    //old  Keys[OSD_KEY_ESC] = 1;
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
  //old  Keys[OSD_KEY_ESC] = 1;
  }
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
    FileRequester = (struct FileRequester  *)AllocAslRequest(ASL_FileRequest, NULL);
  
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
  // intuition requester
  struct EasyStruct es;
  
  es.es_StructSize   = sizeof(struct EasyStruct);
  es.es_Flags        = 0;
  es.es_Title        =(CONST_STRPTR)(APPNAME);
  es.es_TextFormat   = (CONST_STRPTR)(GetMessage(msg_id));
  es.es_GadgetFormat = (CONST_STRPTR)(GetMessage(MSG_OK));

  EasyRequestArgs(NULL, &es, NULL, &((&msg_id)[1]));
}
