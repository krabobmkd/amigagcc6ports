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

//#include "main.h"


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




extern "C" {
// all C amiga stuffs should be included from C++ in extern "C" paragraph
#include <stdio.h>
#include <stdlib.h>
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
#include <intuition/intuition.h>
#include "intuiuncollide.h"
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
// end extern "C"
#include <macros.h>
}


#include "version.h"
#include "audio.h"
#include "video.h"
#include "amiga_inputs.h"
#include "amiga_locale.h"
#include "config.h"
#include "gui.h"

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

struct timerequest    *TimerIO=NULL;
struct MsgPort      TimerMP;

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

static struct Menu *g_gtMenu=NULL;


//krbtest, fake drivers list
const struct GameDriver *drivers[] =
{
	0	/* end of array */
};


//
int libs_init()
{
    // mandatory:
  printf("DOSBase:%08x\n",(int)DOSBase);

    if(!(DOSBase = (struct DosLibrary *) OpenLibrary("dos.library", 39))) return(1);
  printf("after: DOSBase:%08x\n",(int)DOSBase);
  printf("IntuitionBase:%08x\n",(int)IntuitionBase);
    if(!(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 39)))
    {
        printf("need at least OS3.0\n");
        return(1);
    }
    if(!(UtilityBase = OpenLibrary("utility.library",0))) return(1);
    if(!(KeymapBase = OpenLibrary("keymap.library", 36))) return(1);
    if(!(AslBase = OpenLibrary("asl.library", 36))) return(1);
#ifdef POWERUP
    if(!(PPCLibBase = OpenLibrary("ppc.library", 46))) return(1);
#endif
    // optional:
    CyberGfxBase  = OpenLibrary("cybergraphics.library", 0);
    GadToolsBase  = OpenLibrary("gadtools.library", 0);

    // - - - - - - -
    if(GadToolsBase)
    {
        for(int i = 0; NewMenu[i].nm_Type != NM_END; i++)
        {
          if(((NewMenu[i].nm_Type == NM_TITLE) || (NewMenu[i].nm_Type == NM_ITEM))
          && (NewMenu[i].nm_Label != NM_BARLABEL))
            NewMenu[i].nm_Label = GetMessage((LONG) NewMenu[i].nm_Label);
        }
        g_gtMenu  = CreateMenus(NewMenu, GTMN_FullMenu, TRUE, TAG_END);
    }
    else
        g_gtMenu  = NULL;
    // - - - - - - - timer init. Most likely to work
    {
        TimerMP.mp_Node.ln_Type   = NT_MSGPORT;
        TimerMP.mp_Flags      = PA_IGNORE;
        NewList(&TimerMP.mp_MsgList);

        TimerIO = (struct timerequest*)CreateIORequest(&TimerMP, sizeof(struct timerequest));
        if(TimerIO)
        {
        if(!OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *) TimerIO, 0))
          TimerBase = (struct Library *) TimerIO->tr_node.io_Device;
        }
    }
    return(0);
}

// exit code that is executed in all cases:
// - after main()
// - when anything call exit()
// - SIGTERM signal (->to be managed)
void main_close()
{

    if(TimerIO)
    {
        if(TimerBase)
          CloseDevice((struct IORequest *) TimerIO);
        DeleteIORequest((struct IORequest *) TimerIO);
    }

    if(g_gtMenu) FreeMenus(g_gtMenu);

    if(GadToolsBase) CloseLibrary(GadToolsBase);
    if(CyberGfxBase) CloseLibrary(CyberGfxBase);

#ifdef POWERUP
    if(PPCLibBase) CloseLibrary(PPCLibBase);
#endif
    if(KeymapBase) CloseLibrary(KeymapBase);
    if(UtilityBase) CloseLibrary(UtilityBase);
    if(IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    if(GfxBase) CloseLibrary(GfxBase);
    if(DOSBase) CloseLibrary((struct Library *)DOSBase);
}


int main(int argc, char **argv)
{
    atexit(&main_close);

    if(libs_init()!=0) exit(1);
 printf("after libs_init()\n");

    if(AllocConfig(argc, argv)!=0) exit(1);

 printf("after AllocConfig()\n");
   // AllocGUI();
   // LoadConfig(argc, argv);

//  struct Task *task;
  
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

//   Main(argc, argv);

  return(0);
}

//LONG VideoOpen(LONG width, LONG height, LONG left, LONG top, LONG right, LONG bottom, LONG dirty)
//{

//// #define	GAME_REQUIRES_16BIT			0x0100	/* cannot fit in 256 colors */
//  static ULONG pixel_formats[] =
//  {
//    PIXFMT_RGB16,
//    PIXFMT_RGB15,
//    PIXFMT_BGR15,
//    PIXFMT_RGB15PC,
//    PIXFMT_BGR15PC,
//    PIXFMT_BGR16,
//    PIXFMT_RGB16PC,
//    PIXFMT_BGR16PC,
//    PIXFMT_LUT8,
//    ~0
//  };

//  int  fail;
//  LONG visible_width;
//  LONG visible_height;


//  if(Config[CFG_DIRECTMODE] == CFGDM_DRAW)
//  {
//    visible_width  = width + 16;
//    visible_height = height + 16;
//  }
//  else
//  {
//    visible_width  = right - left + 1;
//    visible_height = bottom - top + 1;
//  }

//  /* Disable dirty line support if requested by user. */

//  if(!Config[CFG_DIRTYLINES])
//    dirty = 0;

//  Video = AllocVideo(VA_UseScreen,     (Config[CFG_SCREENTYPE] != CFGST_WB),
//                     VA_UseScreenReq,  (Config[CFG_SCREENTYPE] == CFGST_USERSELECT),
//                     VA_Width,         (Config[CFG_WIDTH] > visible_width)
//                                       ? Config[CFG_WIDTH]
//                                       : visible_width,
//                     VA_Height,        (Config[CFG_HEIGHT] > visible_height)
//                                       ? Config[CFG_HEIGHT]
//                                       : visible_height,
//                     VA_ModeID,        (Config[CFG_SCREENTYPE] == CFGST_CUSTOM)
//                                       ? (Config[CFG_SCREENMODE])
//                                       : INVALID_ID,
//                     VA_Depth,         (Config[CFG_SCREENTYPE] == CFGST_CUSTOM)
//                                       ? (Config[CFG_DEPTH])
//                                       : 0,
//                     VA_Buffers,       Config[CFG_BUFFERING]+1,
//                     VA_Title,         APPNAME,
//                     VA_Menu,          Menu,
//                     VA_FPS,           Drivers[Config[CFG_DRIVER]]->drv->frames_per_second,
//                     VA_MaxColors,     // we only manage 8b or 16b
//                                        // yet VIDEO_NEEDS_6BITS_PER_GUN would mean better in 24b
//                                        (Drivers[Config[CFG_DRIVER]]->drv->total_colors <= 256)
//                                        ?(Drivers[Config[CFG_DRIVER]]->drv->total_colors):(1<<16)

//                                        /*(Drivers[Config[CFG_DRIVER]]->drv->total_colors <= 256)
//                                       ? Drivers[Config[CFG_DRIVER]]->drv->total_colors
//                                       : ((Drivers[Config[CFG_DRIVER]]->drv->video_attributes & VIDEO_NEEDS_6BITS_PER_GUN)
//                                         && Config[CFG_ALLOW16BIT])
//                                         ? (1<<24)
//                                         : (1<<16)

//                                        */,
//                     VA_AutoFrameSkip, Config[CFG_AUTOFRAMESKIP],
//                     VA_MaxFrameSkip,  4,
//                     TAG_END);

//  if(Video)
//  {
//    DirectArray = NULL;
//    Inputs      = NULL;
  
//    if(Config[CFG_DIRECTMODE])
//    {
//      DirectArray = VAllocDirectArray(Video, visible_width, visible_height);
      
//      if(DirectArray)
//      {
//        VSetDirectFrame(DirectArray);
        
//        if(!DirectArray->Pixels)
//        {
//          VFreeDirectArray(DirectArray);
//          DirectArray = NULL;
//        }
//      }
//    }

//    fail = 0;

//    if(!DirectArray)
//    {
//      /*old0.35: if((Drivers[Config[CFG_DRIVER]]->drv->video_attributes & VIDEO_SUPPORTS_16BIT)
//         && Config[CFG_ALLOW16BIT])*/
//      if(Drivers[Config[CFG_DRIVER]]->drv->total_colors >256)
//      {
//        PixelArray[0] = VAllocPixelArray(Video, width+16, height+16, dirty, pixel_formats);
//      }
//      else
//        PixelArray[0] = VAllocPixelArray(Video, width+16, height+16, dirty, NULL);
      
//      if(PixelArray[0])
//      {
//        VSetPixelArrayBox(PixelArray[0], 8 + left, 8 + top, 8 + right, 8 + bottom);
//      }
//      else
//        fail = 1;
//    }
//    else
//      PixelArray[0] = NULL;

//#ifdef POWERUP
//    if(!DirectArray)
//    {
//      if((Drivers[Config[CFG_DRIVER]]->drv->video_attributes & VIDEO_SUPPORTS_16BIT)
//         && Config[CFG_ALLOW16BIT])
//      {
//        PixelArray[1] = VAllocPixelArray(Video, width+16, height+16, dirty, pixel_formats);
//      }
//      else
//        PixelArray[1] = VAllocPixelArray(Video, width+16, height+16, dirty, NULL);

//      if(PixelArray[1])
//      {
//        VSetPixelArrayBox(PixelArray[1], 8 + left, 8 + top, 8 + right, 8 + bottom);
//      }
//      else
//        fail = 1;
//    }
//    else
//      PixelArray[1] = NULL;
//#endif

//    if(!fail)
//    {
//      RefreshHook.h_Entry = (RE_HOOKFUNC) RefreshHandler;
//      MenuHook.h_Entry    = (RE_HOOKFUNC) MenuHandler;
//      IDCMPHook.h_Entry   = (RE_HOOKFUNC) IDCMPHandler;

//      Inputs = AllocInputs(IA_Port1,          (Config[CFG_JOY1TYPE] == CFGJ1_MOUSE1)
//                                              ? IPT_MOUSE
//                                              : Config[CFG_JOY2TYPE],
//                           IA_Port2,          (Config[CFG_JOY1TYPE] == CFGJ1_MOUSE1)
//                                              ? IPT_NONE
//                                              : Config[CFG_JOY1TYPE],
//                      //TODO: to be replaced     IA_KeyMap,         (ULONG) KeyMap,
//                           IA_P1AutoFireRate, (Config[CFG_JOY1TYPE] == CFGJ1_MOUSE1)
//                                              ? Config[CFG_JOY1AUTOFIRERATE]
//                                              : Config[CFG_JOY2AUTOFIRERATE],
//                           IA_P1BlueEmuTime,  (Config[CFG_JOY1TYPE] == CFGJ1_MOUSE1)
//                                              ? Config[CFG_JOY1BUTTONBTIME]
//                                              : Config[CFG_JOY2BUTTONBTIME],
//                           IA_P2AutoFireRate, Config[CFG_JOY1AUTOFIRERATE],
//                           IA_P2BlueEmuTime,  Config[CFG_JOY1BUTTONBTIME],
//                           IA_Window,         Video->Window,
//                           IA_RefreshHook,    (ULONG) &RefreshHook,
//                           IA_MenuHook,       (ULONG) &MenuHook,
//                           IA_IDCMPHook,      (ULONG) &IDCMPHook,
//#ifdef POWERUP
//                           IA_UseTicks,       TRUE,
//#endif
//                           TAG_END);


//      if(Inputs)
//      {
//        Width  = Video->Width;
//        Height = Video->Height;
//        Keys   = Inputs->Keys;
//        Port1  = Inputs->Ports[1];
//        Port2  = Inputs->Ports[0];
//        return(1);
//      }
//      else
//        ErrorRequest(MSG_FAILED_TO_ALLOCATE_INPUTS);
//    }
//    else
//      ErrorRequest(MSG_NOT_ENOUGH_MEMORY);

//    if(DirectArray)
//      VFreeDirectArray(DirectArray);
//    if(PixelArray[0])
//      VFreePixelArray(PixelArray[0]);
//#ifdef POWERUP
//    if(PixelArray[1])
//      VFreePixelArray(PixelArray[1]);
//#endif

//    FreeVideo(Video);
//  }
//  else
//  {
//    switch(VError)
//    {
//      case None:
//        break;
        
//      case OutOfMemory:
//        ErrorRequest(MSG_NOT_ENOUGH_MEMORY);
//        break;

//      case OpenScreenFailed:
//        ErrorRequest(MSG_FAILED_TO_OPEN_SCREEN);
//        break;

//      case OpenWindowFailed:
//        ErrorRequest(MSG_FAILED_TO_OPEN_WINDOW);
//        break;
//    }
//  }

//  return(0);
//}

//void VideoClose(void)
//{
//  if(Video)
//  {
//    if(DirectArray)
//      VFreeDirectArray(DirectArray);
//    if(PixelArray[0])
//      VFreePixelArray(PixelArray[0]);
//#ifdef POWERUP
//    if(PixelArray[1])
//      VFreePixelArray(PixelArray[1]);
//#endif

//    if(Inputs)
//      FreeInputs(Inputs);

//    FreeVideo(Video);
//    Video = NULL;
//  }
//}

//void InputUpdate(LONG wait)
//{
//  if(Inputs)
//  {
//    if(wait)
//      Wait(Inputs->SignalMask);

//    IUpdate(Inputs);

//    if(MenuSelect[ITEM_NEW])
//    {
//      MenuSelect[ITEM_NEW] = 0;

//      NewGame              = 1;
//   //re   Keys[OSD_KEY_ESC]    = 1;
//    }

//    if(MenuSelect[ITEM_SAVE_ILBM])
//    {
//      MenuSelect[ITEM_SAVE_ILBM] = 0;
//      IDisable(Inputs);
//      SaveILBM();
//      ScreenToFront(Video->Window->WScreen);
//      ActivateWindow(Video->Window);
//      IEnable(Inputs);
//    }

//    if(MenuSelect[ITEM_ABOUT])
//    {
//      MenuSelect[ITEM_ABOUT]  = 0;
//      IDisable(Inputs);
//      AboutGUI();
//      ScreenToFront(Video->Window->WScreen);
//      ActivateWindow(Video->Window);
//      IEnable(Inputs);
//    }

//    if(MenuSelect[ITEM_QUIT])
//    {
//      MenuSelect[ITEM_QUIT] = 0;

//      NewGame           = 0;
//    //old  Keys[OSD_KEY_ESC] = 1;
//    }
//  }
//}

//void ASM RefreshHandler(struct Hook *hook REG(a0))
//{
//  VRefresh(Video);
//}

//void ASM MenuHandler(struct Hook *hook REG(a0), APTR null REG(a2), ULONG *itemnum REG(a1))
//{
//  MenuSelect[*itemnum]  = 1;
//}

//void ASM IDCMPHandler(struct Hook *hook REG(a0), APTR null REG(a2), ULONG *imclass REG(a1))
//{
//  if(*imclass == IDCMP_CLOSEWINDOW)
//  {
//    NewGame           = 1;
//  //old  Keys[OSD_KEY_ESC] = 1;
//  }
//}


///*
//uclock_t uclock(void)
//{
//  static uclock_t old_uclock    = 0;
//  uclock_t    new_uclock;
//  struct timeval  tv;

//  if(TimerBase)
//  {
//    GetSysTime(&tv);
//    new_uclock  = (tv.tv_secs * UCLOCKS_PER_SEC) + (tv.tv_micro / (1000000 / UCLOCKS_PER_SEC));
//    if(new_uclock == old_uclock)
//      new_uclock++;
//  }
//  else
//    new_uclock  = old_uclock + UCLOCKS_PER_SEC;

//  old_uclock  = new_uclock;

//  return(new_uclock);
//}
//*/

//void SaveILBM(void)
//{
//  BPTR new_dir;
//  BPTR old_dir;

//  if(!FileRequester)
//    FileRequester = (struct FileRequester  *)AllocAslRequest(ASL_FileRequest, NULL);
  
//  if(FileRequester)
//  {
//    if(AslRequestTags(FileRequester,
//                      ASLFR_DoSaveMode,  TRUE,
//                      ASLFR_SleepWindow, TRUE,
//                      ASLFR_TitleText,   (ULONG) GetMessage(MSG_MENU_SAVE_ILBM),
//                      TAG_END))
//    {
//      new_dir = Lock(FileRequester->fr_Drawer, ACCESS_READ);
      
//      if(new_dir)
//      {
//        old_dir = CurrentDir(new_dir);
        
//        VSaveILBM(Video, FileRequester->fr_File);
        
//        CurrentDir(old_dir);
        
//        UnLock(new_dir);
//      }
//    }
//  }
//}

//void ErrorRequest(LONG msg_id, ...)
//{
//  // intuition requester
//  struct EasyStruct es;
  
//  es.es_StructSize   = sizeof(struct EasyStruct);
//  es.es_Flags        = 0;
//  es.es_Title        =(CONST_STRPTR)(APPNAME);
//  es.es_TextFormat   = (CONST_STRPTR)(GetMessage(msg_id));
//  es.es_GadgetFormat = (CONST_STRPTR)(GetMessage(MSG_OK));

//  EasyRequestArgs(NULL, &es, NULL, &((&msg_id)[1]));
//}
