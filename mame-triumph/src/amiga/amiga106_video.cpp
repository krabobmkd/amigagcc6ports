/**************************************************************************
 *
 * Copyright (C) 2024 Vic Krb Ferry
 *
 *************************************************************************/

// Amiga includes, proto manages __cplusplus__
//#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>
#include <proto/intuition.h>

#include <proto/utility.h>

extern "C" {
    #include <exec/types.h>
    #include <exec/memory.h>

    #include <graphics/gfxbase.h>
    #include <graphics/rastport.h>
    #include <graphics/modeid.h>

    #include <intuition/intuition.h>
    #include <intuition/screens.h>

    #include <cybergraphx/cybergraphics.h>

}
// from mame
extern "C" {
    #include "osdepend.h"
    #include "video.h"
    // for logerror
    #include "mame.h"
}
#include "amiga106_inputs.h"
#include "amiga106_video.h"
/** some abstact display management */

#include <stdio.h>

extern struct Library *CyberGfxBase;
extern struct Library *P96Base;

MameDisplay::MameDisplay() {
}
MameDisplay::~MameDisplay(){}

// - - - - from driver.h
/* is the video hardware raser or vector base? */
#define	VIDEO_TYPE_RASTER				0x0000
#define	VIDEO_TYPE_VECTOR				0x0001

/* should VIDEO_UPDATE by called at the start of VBLANK or at the end? */
#define	VIDEO_UPDATE_BEFORE_VBLANK		0x0000
#define	VIDEO_UPDATE_AFTER_VBLANK		0x0002

/* set this to use a direct RGB bitmap rather than a palettized bitmap */
#define VIDEO_RGB_DIRECT	 			0x0004

/*
  Create a display screen, or window, of the given dimensions (or larger). It is
  acceptable to create a smaller display if necessary, in that case the user must
  have a way to move the visibility window around.

  The params contains all the information the
  Attributes are the ones defined in driver.h, they can be used to perform
  optimizations, e.g. dirty rectangle handling if the game supports it, or faster
  blitting routines with fixed palette if the game doesn't change the palette at
  run time. The VIDEO_PIXEL_ASPECT_RATIO flags should be honored to produce a
  display of correct proportions.
  Orientation is the screen orientation (as defined in driver.h) which will be done
  by the core. This can be used to select thinner screen modes for vertical games
  (ORIENTATION_SWAP_XY set), or even to ask the user to rotate the monitor if it's
  a pivot model. Note that the OS dependent code must NOT perform any rotation,
  this is done entirely in the core.
  Depth can be 8 or 16 for palettized modes, meaning that the core will store in the
  bitmaps logical pens which will have to be remapped through a palette at blit time,
  and 15 or 32 for direct mapped modes, meaning that the bitmaps will contain RGB
  triplets (555 or 888). For direct mapped modes, the VIDEO_RGB_DIRECT flag is set
  in the attributes field.

  Returns 0 on success.
*/
static void waitsec(int s)
{
    for(int j=0;j<s;j++)
    for(int i=0;i<50;i++)
    {
        WaitTOF();
    }
}
MameDisplay *g_pMameDisplay=NULL;

int osd_create_display(const _osd_create_params *params, UINT32 *rgb_components)
{
    if(g_pMameDisplay) osd_close_display();
    if((params->video_attributes &VIDEO_TYPE_VECTOR)==0)
    {
        //try RTG  drivers first:
        if(CyberGfxBase)
        {
            g_pMameDisplay = new Display_CGX();
            g_pMameDisplay->open(params,0);
        }

    } // end if bitmap

//    if(P96Base)
//    {
//        g_pMameDisplay = new Display_P96(params->width, params->height);
//    }
    if(!g_pMameDisplay || !g_pMameDisplay->good())
    {
        //
        logerror("couldn't find a graphic mode.");
        return 1; // fail.
    }


    AllocInputs(); // input object depends of screen or window.

    return 0; // success
}
void osd_close_display(void)
{
    FreeInputs();
    if(g_pMameDisplay) {
        delete g_pMameDisplay;
        g_pMameDisplay = NULL;
    }
}


/*
  Update video and audio. game_bitmap contains the game display, while
  debug_bitmap an image of the debugger window (if the debugger is active; NULL
  otherwise). They can be shown one at a time, or in two separate windows,
  depending on the OS limitations. If only one is shown, the user must be able
  to toggle between the two by pressing IPT_UI_TOGGLE_DEBUG; moreover,
  osd_debugger_focus() will be used by the core to force the display of a
  specific bitmap, e.g. the debugger one when the debugger becomes active.

  leds_status is a bitmask of lit LEDs, usually player start lamps. They can be
  simulated using the keyboard LEDs, or in other ways e.g. by placing graphics
  on the window title bar.
*/
void osd_update_video_and_audio(struct _mame_display *display)
{
   // printf("osd_update_video_and_audio\n");
    if(!g_pMameDisplay) return;
    g_pMameDisplay->draw(display);
    MsgPort *userport = g_pMameDisplay->userPort();
    if(userport) UpdateInputs(userport);
}


/*
  osd_skip_this_frame() must return 0 if the current frame will be displayed.
  This can be used by drivers to skip cpu intensive processing for skipped
  frames, so the function must return a consistent result throughout the
  current frame. The function MUST NOT check timers and dynamically determine
  whether to display the frame: such calculations must be done in
  osd_update_video_and_audio(), and they must affect the FOLLOWING frames, not
  the current one. At the end of osd_update_video_and_audio(), the code must
  already know exactly whether the next frame will be skipped or not.
*/
int osd_skip_this_frame(void)
{
// TODO
//    if(FrameCounter >= NoFrameSkipCount)
//    {
//      if(FrameCounter < (NoFrameSkipCount + frameskip))
//        return(1);
//    }
//    static int i=0;
//    i++;
//    return (i&1);
    return(0);
}

/*
  Provides a hook to allow the OSD system to override processing of a
  snapshot.  This function will either return a new bitmap, for which the
  caller is responsible for freeing.
*/
mame_bitmap *osd_override_snapshot(mame_bitmap *bitmap, rectangle *bounds)
{
    return NULL;
}

/*
  Returns a pointer to the text to display when the FPS display is toggled.
  This normally includes information about the frameskip, FPS, and percentage
  of full game speed.
*/
const char *osd_get_fps_text(const performance_info *performance)
{
    return "osd_get_fps_text to implement";
}


