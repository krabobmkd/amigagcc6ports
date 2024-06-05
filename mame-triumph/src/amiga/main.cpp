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
// from mame
extern "C" {
    #include "osdepend.h"
    #include "driver.h"
    #include "unzip.h"

}
//#define CATCOMP_BLOCK
//#include "mame_msg.h" -> called by main.h

#define CATCOMP_NUMBERS
#include "messages.h"

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

extern "C" {
    // all C amiga stuffs should be included from C++ in extern "C" paragraph
    #include <cybergraphx/cybergraphics.h>
    #include <macros.h>

}


#include "version.h"
#include "video.h"

#include "amiga_locale.h"
#include "amiga106_config.h"
#include "amiga106_inputs.h"
#include "gui_mui.h"
#include "gui_gadtools.h"

#include "file.h"

#define MIN_STACK (10*1024)

#define ITEM_NEW    0
#define ITEM_SAVE_ILBM  1
#define ITEM_ABOUT    3
#define ITEM_QUIT   5
#define NUM_ITEMS   6

//extern "C" {
void     Main(int argc, char **argv);
extern void ASM RefreshHandler(struct Hook *hook REG(a0));
extern void ASM MenuHandler(struct Hook *hook REG(a0), APTR null REG(a2), ULONG *itemnum REG(a1));
extern void ASM IDCMPHandler(struct Hook *hook REG(a0), APTR null REG(a2), ULONG *imclass REG(a1));
void     SaveILBM(void);
void     ErrorRequest(LONG msg_id, ...);

//}


void     StartGame(void);  /* In amiga/amiga.c. */


#ifdef POWERUP
struct _game_driver **Drivers;
#endif

LONG        MenuSelect[NUM_ITEMS];
UBYTE       Channel[4];
static LONG SelectNextGame;

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
struct Library      *P96Base = NULL;

}

struct FileRequester  *FileRequester  = NULL;

static struct StackSwapStruct StackSwapStruct;

void initTimers();
void closeTimers();


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

    InitLowLevelLib();
    // optional:
    CyberGfxBase  = OpenLibrary("cybergraphics.library", 1);
    P96Base  = OpenLibrary("Picasso96API.library", 0);
    GadToolsBase  = OpenLibrary("gadtools.library", 1);
    // mui is done elsewhere.

    if(GadToolsBase) gui_gadtools_init();

    initTimers();

    return(0);
}

extern "C" {
void mameExitCleanCtrlC(void);
}
// exit code that is called from any exit() and ctrl-c breaks:
void main_close()
{
    printf("does main_close\n");

    mameExitCleanCtrlC(); // flush game allocs, ahcked from mame.c, in case stopped during game.
    osd_close_display(); // also useful when ctrl-C
    osd_stop_audio_stream();
    unzip_cache_clear();

    closeTimers();

    FreeGUI();
   // FreeConfig(); -> static destructor in _config.

    gui_gadtools_close();

    CloseLowLevelLib();
    if(GadToolsBase) CloseLibrary(GadToolsBase);
    if(P96Base) CloseLibrary(P96Base);
    if(CyberGfxBase) CloseLibrary(CyberGfxBase);

    if(KeymapBase) CloseLibrary(KeymapBase);
    if(UtilityBase) CloseLibrary(UtilityBase);
    if(IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    if(GfxBase) CloseLibrary((struct Library *)GfxBase);
// done in  by compiler runtime or not.
#ifdef USE_OWN_DOSBASE
   if(DOSBase) CloseLibrary((struct Library *)DOSBase);
#endif
}

const char *pVersion="$VER: 0.106 a0.1";

#if defined(MAME_USE_HARD_FLOAT)

int amiga_hasFPU()
{UWORD attnflags = SysBase->AttnFlags; return (int)((attnflags & (AFF_68881|AFF_68882|AFF_FPU40))!=0);
}
void amiga_cpucheck()
{
    int hasFPU = amiga_hasFPU();
    if(!hasFPU) {
        printf("This tool was compiled for machines with a FPU.\n");
        exit(1);
    }
}

// make it happen before main() with a global constructor, less intrusive to original code.
struct beforeMainInit
{
    beforeMainInit() {
      amiga_cpucheck();
    }
};
beforeMainInit _ginit;

#endif


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

    getMainConfig().init(argc,argv);
    getMainConfig().load();

    AllocGUI();

    // go into interface loop, select first game.
    if(MainGUI()!=0) exit(0);

    ULONG quit=FALSE;

    // loop per emulation launched
    while(!quit)
    {
        SelectNextGame = 1;

        StartGame();
        // here game is closed.
        osd_stop_audio_stream();

        // relaunch GUI
        if(SelectNextGame > 0)
          quit = MainGUI();
        else if(!SelectNextGame)
          quit = TRUE;
        } // end loop by emulation launch

    // end of main, will automatically reach main_close(), like any exit() call does.
    return(0);
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
