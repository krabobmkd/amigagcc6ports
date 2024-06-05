

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

// proto/xxx.h don't need extern "C" {}
#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/timer.h>

#include <proto/asl.h>
#include <proto/utility.h>
#include <proto/keymap.h>
#include <proto/cybergraphics.h>


extern "C" {
#include <inline/alib.h>
//#include <inline/cybergraphics.h>
// all C amiga stuffs should be included from C++ in extern "C" paragraph

//#include <exec/types.h>
//#include <exec/memory.h>
//#include <exec/tasks.h>
//#include <dos/dos.h>
//#include <workbench/workbench.h>
//#include <libraries/asl.h>
//#include <libraries/gadtools.h>
////re #include <cybergraphx/cybergraphics.h>
//#include <intuition/intuition.h>
//#include "intuiuncollide.h"




//#include <inline/graphics.h>
//#include <inline/intuition.h>
//#include <inline/asl.h>
//#include <inline/gadtools.h>
//#include <inline/locale.h>
//#include <inline/timer.h>

//struct Library      *GadToolsBase = NULL;
//struct Library      *AslBase    = NULL;
//struct Library      *KeymapBase   = NULL;
//struct Library      *UtilityBase  = NULL;
//struct Library      *TimerBase    = NULL;
// end extern "C"
//re #include <macros.h>
}

#define CATCOMP_NUMBERS
#include "messages.h"

#include "config_v37.h"
#include "gui_mui.h"
#include "gui_gadtools.h"
/*re
#include "version.h"
#include "audio.h"
#include "video.h"
#include "amiga_inputs.h"
#include "amiga_locale.h"
#include "config_v37.h"
#include "gui.h"

#include "osdepend.h"
#include "driver.h"
#include "file.h"
*/


//extern struct ExecBase      *SysBase;
//struct DosLibrary   *DOSBase=NULL;
struct GfxBase      *GfxBase=NULL;
struct IntuitionBase  *IntuitionBase=NULL;


struct Library      *CyberGfxBase=NULL;

struct Library      *GadToolsBase=NULL;
struct Library      *AslBase=NULL;
struct Library      *KeymapBase=NULL;
struct Library      *UtilityBase=NULL;

//struct Library      *TimerBasePrivate=NULL;
//struct timerequest    *TimerIO=NULL;
//struct MsgPort      TimerMP;

int libs_init();
void main_close();


int main(int argc, char **argv)
{

    atexit(&main_close);
    if(libs_init()!=0) exit(1);
    printf("after libs_init()\n");

    printf("init: DOSBase:%08x\n",(int)DOSBase);
    printf("init: GfxBase:%08x\n",(int)GfxBase);
    printf("init: IntuitionBase:%08x\n",(int)IntuitionBase);
    printf("init: UtilityBase:%08x\n",(int)UtilityBase);
    printf("init: KeymapBase:%08x\n",(int)KeymapBase);
    printf("init: AslBase:%08x\n",(int)AslBase);
  // if enabled, will exit before entering main because CyberGfxBase is linked.
    printf("init: CyberGfxBase:%08x\n",(int)CyberGfxBase);
    printf("init: GadToolsBase:%08x\n",(int)GadToolsBase);

    printf("c:%d\n",(int)Config().audio.sound);


  return(0);
}


int libs_init()
{
//    if(!(DOSBase = (struct DosLibrary *) OpenLibrary("dos.library", 36))) return(1);
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
#ifdef POWERUP
    if(!(PPCLibBase = OpenLibrary("ppc.library", 46))) return(1);
#endif

    // optional:
    CyberGfxBase  = OpenLibrary("cybergraphics.library", 1);
    GadToolsBase  = OpenLibrary("gadtools.library", 1);

    if(GadToolsBase) gui_gadtools_init();

//    // - - - - - - - timer init. Most likely to work
//    {
//        TimerMP.mp_Node.ln_Type   = NT_MSGPORT;
//        TimerMP.mp_Flags      = PA_IGNORE;
//        NewList(&TimerMP.mp_MsgList);

//        TimerIO = (struct timerequest*)CreateIORequest(&TimerMP, sizeof(struct timerequest));
//        if(TimerIO)
//        {
//        if(!OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *) TimerIO, 0))
//          TimerBasePrivate = (struct Library *) TimerIO->tr_node.io_Device;
//        }
//    }
    return(0);
}

// exit code that is executed in all cases:
// - after main()
// - when anything call exit()
// - SIGTERM signal (->to be managed)
void main_close()
{
    printf("does main_close\n");
//    if(TimerIO)
//    {
//        if(TimerBasePrivate)
//          CloseDevice((struct IORequest *) TimerIO);
//        DeleteIORequest((struct IORequest *) TimerIO);
//    }


    if(GadToolsBase) CloseLibrary(GadToolsBase);
    if(CyberGfxBase) CloseLibrary(CyberGfxBase);

#ifdef POWERUP
    if(PPCLibBase) CloseLibrary(PPCLibBase);
#endif
    if(KeymapBase) CloseLibrary(KeymapBase);
    if(UtilityBase) CloseLibrary(UtilityBase);
    if(IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    if(GfxBase) CloseLibrary((struct Library *)GfxBase);
// done in theory by gcc startup
//    if(DOSBase) CloseLibrary((struct Library *)DOSBase);
}


