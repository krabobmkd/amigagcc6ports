/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id: video.c,v 1.2 1999/04/28 18:55:01 meh Exp $
 *
 * $Log: video.c,v $
 * Revision 1.2  1999/04/28 18:55:01  meh
 * *** empty log message ***
 *
 * Revision 1.1  1999/04/20 18:52:45  meh
 * Initial revision
 *
 *************************************************************************/

#include <stdio.h>

#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/asl.h>
#include <proto/utility.h>

extern "C" {
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>
#include <graphics/rastport.h>
#include <intuition/intuition.h>
#include "intuiuncollide.h"
#include <intuition/screens.h>
#include <libraries/iffparse.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <cybergraphx/cybergraphics.h>
//#include "cgxhooks_re.h"e
//#define TIMER_BASE_NAME video->TimerBase
//#include <inline/timer.h>

#include <macros.h>
}

//#include "video.h"


#include <stdio.h>

#define CYBRBIDTG_TB	(TAG_USER+0x50000)

//FilterTags

#define CYBRBIDTG_Depth		(CYBRBIDTG_TB+0)
#define CYBRBIDTG_NominalWidth		(CYBRBIDTG_TB+1)
#define CYBRBIDTG_NominalHeight		(CYBRBIDTG_TB+2)
#define CYBRBIDTG_MonitorID		(CYBRBIDTG_TB+3)
#define CYBRBIDTG_BoardName		(CYBRBIDTG_TB+5)

#define CYBRIDATTR_PIXFMT		(0x80000001)	// the pixel format is returned
#define CYBRIDATTR_WIDTH		(0x80000002)	// returns visible width in pixels
#define CYBRIDATTR_HEIGHT		(0x80000003)	// returns visible height in lines
#define CYBRIDATTR_DEPTH		(0x80000004)	// returns bits per pixel
#define CYBRIDATTR_BPPIX		(0x80000005)	// BytesPerPixel shall be returned

#include <graphics/modeid.h>
#include <intuition/screens.h>
// for calloc/free:
#include <stdlib.h>



// structure to handle the demoscreen object:
typedef struct DemoScreen_ {
        ULONG		ds_ScreenModeID;
		unsigned int	ds_MaxWidth;
		unsigned int	ds_MaxHeight;
		unsigned int	ds_fullscreenWidth; // dimension from modeid
		unsigned int	ds_fullscreenHeight;
		void			*ds_InvisibleMouseRaster;
		struct MsgPort *ds_CurrentPort; 	// this port can change if fullscreen mode change.
		struct Screen 	*ds_IntuitionScreen;
		struct Window *ds_ScreenWindow;
		struct Window	*ds_LittleWindow;

		// cgx lib load management:
		// cgx video (or not) for overlay
/*		struct Library		*ds_CGXVideoLib;
		struct CGXVideoIFace *ds_CGXVI;
*/
		// amiga double buffer stuff for fullscreen:
		struct ScreenBuffer *m_pBufferAlloc; // allocated.
		struct ScreenBuffer *m_pBufferInitial;
		struct ScreenBuffer *m_pBuffer1; // swapped.
		struct ScreenBuffer *m_pBuffer2; // swapped.
		struct RastPort	*m_pRenderRastPort;
} DemoScreen ;

// create a demoscreen with 24bit:
extern	DemoScreen *InitDemoScreen( unsigned int _maxWidth,
				unsigned int _maxHeight ) ;

// close and kill the demoscreen:
extern	void	CloseDemoScreen( DemoScreen *_pScreenToClose );

// if mouse button or escape key pressed, return 0, else: -1
// if 'f' or 'space', change fullscreen/window mode.
extern	int	CheckDemoScreenState( DemoScreen *_pScreenToCheck );

//#define RECTFMT_RGB		(0)
//#define RECTFMT_RGBA		(1)
//#define RECTFMT_ARGB		(2)
//#define RECTFMT_LUT8		(3)
//#define RECTFMT_GREY8	(4)

typedef struct _sRenderInfo {
	APTR 	m_pPixelBuffer;
	ULONG	m_srcWidth;
	ULONG	m_srcHeight;
	ULONG	m_srcMod; // bytes per row
	ULONG	m_srcFormat; // cgx pixel format here
	ULONG	m_RectangleRatio; //65536 for square, less for 16/9 on fullscreen.
} sRenderInfo;


// Redraw the demoScreen according to a chunky image.
extern	void	RefreshDemoScreen( DemoScreen *_pScreenToRefresh, sRenderInfo *_pRenderInfo );


struct Library *CyberGfxBase=NULL;

// private. reopen a fullScreen. -1 OK, 0 failed.
static int DemoScreenOpenFullScreen( DemoScreen *_pScreen  )
{
	struct Window *pwindow;
	struct ColorSpec colspec[2]={0,0,0,0,-1,0,0,0};
 	struct Screen *pscreen = OpenScreenTags( NULL,
			SA_DisplayID,_pScreen->ds_ScreenModeID,
                        SA_Width, _pScreen->ds_fullscreenWidth,
                        SA_Height,_pScreen->ds_fullscreenHeight,
//                        SA_Behind,TRUE,    /* Open behind */
                        SA_Quiet,TRUE,     /* quiet */
			SA_Type,CUSTOMSCREEN,
			SA_Colors,(ULONG)&colspec[0],
                        0 );
	_pScreen->ds_IntuitionScreen = pscreen ; // can be NULL if failed.
	if( pscreen == NULL ) return(0);
	//printf("screen OK:%08x \n", pscreen );
	// --------- open intuition fullscreen window for this screen:
	{
		pwindow = OpenWindowTags(/*&screenwin*/NULL,
			WA_CustomScreen,(ULONG)pscreen,
                        WA_Backdrop,FALSE,
                        WA_Borderless,TRUE,
                        WA_Activate,TRUE,
                        WA_RMBTrap,TRUE,
                        WA_ReportMouse,0,
                        WA_SizeGadget,0,
                        WA_DepthGadget,0,
                        WA_CloseGadget,0,
                        WA_DragBar,0,
			  WA_GimmeZeroZero,FALSE, // test
                        WA_IDCMP,IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY ,
                        0 );
	}
	if( pwindow ==  NULL ) return (0);
	_pScreen->ds_ScreenWindow = pwindow ;
	// --------- note the Current userPort:
	_pScreen->ds_CurrentPort = pwindow->UserPort;
	// ------- set invisible mouse pointer:
	SetPointer( pwindow , _pScreen->ds_InvisibleMouseRaster, 0,1,0,0);
	// amiga double buffer stuff:
	_pScreen->m_pBufferAlloc = AllocScreenBuffer(pscreen, NULL,0);
	if(_pScreen->m_pBufferAlloc == NULL) return (0);
	// get current
	_pScreen->m_pBuffer1 = _pScreen->m_pBufferInitial = AllocScreenBuffer(pscreen, NULL, SB_SCREEN_BITMAP);
	_pScreen->m_pBuffer2 = _pScreen->m_pBufferAlloc;

	// alloc rastport to render to hidden bitmap:
	_pScreen->m_pRenderRastPort = AllocVec(sizeof(struct RastPort), MEMF_ANY);
	if( _pScreen->m_pRenderRastPort == NULL ) return (0);
	InitRastPort(_pScreen->m_pRenderRastPort);

/*
	context->w3dRastPort = AllocVec(sizeof(struct RastPort), MEMF_ANY);
	if (!context->w3dRastPort)
	{
	dprintf("Error: unable to allocate rastport memory\n");
	goto Duh;
	}
	InitRastPort(context->w3dRastPort);
	context->w3dRastPort->BitMap = context->w3dBitMap;
*/


	return(-1); //OK
}
// private one. Close the screen for window switch:
static void DemoScreenCloseFullScreen( DemoScreen *_pScreenToClose  )
{
	if(_pScreenToClose->m_pRenderRastPort)
	{
		FreeVec(_pScreenToClose->m_pRenderRastPort);
		_pScreenToClose->m_pRenderRastPort = NULL;
	}

	if(_pScreenToClose->m_pBufferAlloc != NULL)
	{
		ChangeScreenBuffer( _pScreenToClose->ds_IntuitionScreen , _pScreenToClose->m_pBufferInitial );
		FreeScreenBuffer( _pScreenToClose->ds_IntuitionScreen ,_pScreenToClose->m_pBufferAlloc);
		_pScreenToClose->m_pBufferAlloc = NULL;
	}

	// reset initial screen buffer
	if( _pScreenToClose->ds_ScreenWindow )  CloseWindow( _pScreenToClose->ds_ScreenWindow );
	if( _pScreenToClose->ds_IntuitionScreen )   CloseScreen( _pScreenToClose->ds_IntuitionScreen );
	_pScreenToClose->ds_ScreenWindow =  NULL ;
	_pScreenToClose->ds_IntuitionScreen = NULL ;

}
//private. Open the WB window:
struct NewWindow mynewwin =
{
    114, 60,      /* LeftEdge, TopEdge */
    0,0,     /* Width, Height */
    -1, -1,             /* DetailPen, BlockPen */
    IDCMP_MENUPICK | IDCMP_MOUSEBUTTONS | IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MOUSEMOVE |
   IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS, /* IDCMPFlags */
    WFLG_ACTIVATE | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_SIZEGADGET | WFLG_SIZEBBOTTOM |
   WFLG_SIMPLE_REFRESH | WFLG_GIMMEZEROZERO  /*| WINDOWSIZING | WFLG_SIZEBBOTTOM */  /* WFLG_SIZEBRIGHT*/ ,   /* Flags   */
    // windowsizing added if cybergfx
    // gimmezerozero for coordinates inside the window.
    NULL,      /* FirstGadget */
    NULL,      /* CheckMark */
    " Ukonx - My World",// /* Title */
    NULL,      /* Screen */
    NULL,      /* BitMap */
    100 /*Screen_Width */, 84 /* Screen_Height */ , /* MinWidth, MinHeight */
    -1 /*Screen_Width*/,-1 /*Screen_Height*/,  /* MaxWidth, MaxHeight */
    WBENCHSCREEN, /* Type */
};
static int DemoScreenOpenWBWindow( DemoScreen *_pScreen  )
{
	// get default public screen (workbench?)
	struct Screen *pWbScreen;
	if (!(pWbScreen = LockPubScreen(NULL))) return(0); // bad !

	struct Window *pwindow = OpenWindowTags(&mynewwin,
		WA_PubScreen,pWbScreen,
		WA_IDCMP, /* IDCMP_MOUSEBUTTONS |*/ IDCMP_RAWKEY |
		IDCMP_CLOSEWINDOW ,
		WA_InnerWidth,_pScreen->ds_MaxWidth,
		WA_InnerHeight,_pScreen->ds_MaxHeight,
		TAG_DONE );
	if( pwindow == NULL ) return(0);
	_pScreen->ds_LittleWindow = pwindow ;
	_pScreen->ds_CurrentPort  = pwindow->UserPort ;

   /* if cybergraphics  */
 /*   if (CyberGfxBase)
    {
        mynewwin.Flags |= WINDOWSIZING | WFLG_SIZEBBOTTOM  ;
    }
*/
	return(-1);	//OK !
}
// private one. Close the screen for window switch:
static void DemoScreenCloseWBWindow( DemoScreen *_pScreenToClose  )
{
	if( _pScreenToClose->ds_LittleWindow )  CloseWindow( _pScreenToClose->ds_LittleWindow );
	_pScreenToClose->ds_LittleWindow = NULL ;
}

static void SwitchFullDemoScreen( DemoScreen *_pScreenToSwitch  )
{
	if( _pScreenToSwitch->ds_ScreenWindow != NULL   )
	{
		// switch from screen mode to window mode:
		DemoScreenCloseFullScreen( _pScreenToSwitch );
		DemoScreenOpenWBWindow( _pScreenToSwitch );
	}else
	{
		// switch from window mode to screen mode:
		DemoScreenCloseWBWindow( _pScreenToSwitch );
		DemoScreenOpenFullScreen( _pScreenToSwitch );
	}
}

DemoScreen *InitDemoScreen( unsigned int _maxWidth,
				unsigned int _maxHeight )
{
	DemoScreen	*pDemoScreen = calloc( sizeof( DemoScreen ) , 1  );
	if( pDemoScreen  == NULL ) return(NULL);

	// prepare window mode new struct:
	mynewwin.Width  = _maxWidth;
	mynewwin.Height = _maxHeight;
//printf("init: %d %d\n", _maxWidth,_maxHeight);
	// ------------ open Cybergraphics.library:

    if(CyberGfxBase==NULL)
    {
        CyberGfxBase =  OpenLibrary(  "cybergraphics.library",39 );
    }
    if( CyberGfxBase ==  NULL ){ CloseDemoScreen(pDemoScreen); return(NULL);}


	// ----------- alloc invisible mouse raster for full screen:
	void *pmouseraster =  AllocRaster(8 ,8) ;
	if( pmouseraster == NULL ) { CloseDemoScreen(pDemoScreen);  return (NULL); }
	pDemoScreen->ds_InvisibleMouseRaster = pmouseraster ;

	// ---------------- search screen best mode:
    printf("search res: %d %d\n",_maxWidth,_maxHeight);

	{
         unsigned int askedHeight = _maxHeight;
         if(_maxWidth==512 && askedHeight==256)
         {  // on pistorm/picasso 512x384 modes are usually available.
             // try to get this exact reolution, which would avoid scaling.
             askedHeight = 384;
         }
 		 struct TagItem cgxtags[]={
			CYBRBIDTG_NominalWidth,_maxWidth,
			CYBRBIDTG_NominalHeight,askedHeight,
			CYBRBIDTG_Depth,16,
			TAG_DONE,0 };
		ULONG modeid = BestCModeIDTagList(cgxtags);
		if( modeid ==  INVALID_ID ){ CloseDemoScreen(pDemoScreen);  return (NULL); }
		pDemoScreen->ds_ScreenModeID 	= modeid ;
		pDemoScreen->ds_MaxWidth		= _maxWidth ;
		pDemoScreen->ds_MaxHeight	= _maxHeight ;
		// get size of modeid screen:
		pDemoScreen->ds_fullscreenWidth = GetCyberIDAttr( CYBRIDATTR_WIDTH, modeid);
		pDemoScreen->ds_fullscreenHeight = GetCyberIDAttr( CYBRIDATTR_HEIGHT, modeid );

 printf("final resolution:%d %d render res:%d %d\n",pDemoScreen->ds_fullscreenWidth,pDemoScreen->ds_fullscreenHeight,pDemoScreen->ds_MaxWidth,pDemoScreen->ds_MaxHeight);

	}

//	printf("modeid:%08x \n",modeid);

	// ---------------- open full screen:
	if(! DemoScreenOpenFullScreen( pDemoScreen ) ) { CloseDemoScreen(pDemoScreen);  return (NULL); }


//	p96RequestModeIDTags(  );

	return( pDemoScreen );
}
void	CloseDemoScreen( DemoScreen *_pScreenToClose )
{
	if( _pScreenToClose == NULL  ) return;

	DemoScreenCloseFullScreen( _pScreenToClose );
	DemoScreenCloseWBWindow( _pScreenToClose );

	if( _pScreenToClose->ds_InvisibleMouseRaster   ) FreeRaster( _pScreenToClose->ds_InvisibleMouseRaster ,8,8);

	// close libs:
/*
	if( _pScreenToClose->ds_CGXVI  )  DropInterface((struct Interface *) _pScreenToClose->ds_CGXVI );
	if( _pScreenToClose->ds_CGXVideoLib   )  CloseLibrary( _pScreenToClose->ds_CGXVideoLib );
*/
	if( CyberGfxBase  )  CloseLibrary( CyberGfxBase );
    CyberGfxBase = NULL;
	free( _pScreenToClose );
}
/* ======================= */
// if mouse button or escape key pressed, return 0, else -1.
// if 'f' or 'space', change fullscreen/window mode.
int	CheckDemoScreenState( DemoScreen *_pScreenToCheck )
{
	struct IntuiMessage *iMsg;
	struct MsgPort *pport = _pScreenToCheck->ds_CurrentPort ;
	if( !pport  ) return(0); // shouldn't happen. There should exist a port whatever the screen mode is.
	if( iMsg = (struct IntuiMessage *)GetMsg( pport  ) )
	{
    		UWORD   ICode = iMsg->Code;
    		ULONG   IClass = iMsg->Class;
             	ReplyMsg((struct Message *)iMsg);
		// test for exiting cases:
		if ( IClass == IDCMP_MOUSEBUTTONS && ICode == 0x0068  ) return(0);
		if ( IClass == IDCMP_RAWKEY )
		{
			//printf("iclass:%08x\n", ICode );
			if ( ICode == 0x0045 ) return(0); // esc.
			// 'f' or 'space' switch window mode:
			if( ICode == 0x0023 || ICode == 0x0040 ) SwitchFullDemoScreen( _pScreenToCheck );
			// if ( ICode == 0x0021 ) GrabScreen();
		}
		// window mode gadget exit case:
		if ( IClass == IDCMP_CLOSEWINDOW ) return(0);
	}
	return(-1); // OK to continue.
}
/* ======================= */
void	RefreshDemoScreen( DemoScreen *_pScreenToRefresh,  sRenderInfo *_pRenderInfo )
{
	if( _pScreenToRefresh->ds_ScreenWindow != NULL   )
	{
        _pScreenToRefresh->m_pRenderRastPort->BitMap = _pScreenToRefresh->m_pBuffer2->sb_BitMap;


        if(_pRenderInfo->m_srcWidth == _pScreenToRefresh->ds_fullscreenWidth)
        {
            // in that case no scale
            uint32_t finalheight = _pRenderInfo->m_srcHeight;
            uint32_t ytop = (_pScreenToRefresh->ds_fullscreenHeight-finalheight)>>1;

            WritePixelArray(_pRenderInfo->m_pPixelBuffer,
                            0,0, // starting point in source rectangle
                            _pRenderInfo->m_srcMod, // bytes per row in source

                            _pScreenToRefresh->m_pRenderRastPort, // rastport to render
                            0,ytop, // start point in raster
                            _pRenderInfo->m_srcWidth,
                            _pRenderInfo->m_srcHeight,
                            _pRenderInfo->m_srcFormat
                            );
//            /*
//            ULONG        WritePixelArray(APTR, UWORD, UWORD, UWORD, struct RastPort *, UWORD,
//                                         UWORD, UWORD, UWORD, UBYTE);
//        */
        } else
        {
            // scale
            uint32_t finalheight = (_pScreenToRefresh->ds_fullscreenHeight * _pRenderInfo->m_RectangleRatio)>>16;
            uint32_t ytop = (_pScreenToRefresh->ds_fullscreenHeight-finalheight)>>1;

            ScalePixelArray(_pRenderInfo->m_pPixelBuffer,
                _pRenderInfo->m_srcWidth,
                _pRenderInfo->m_srcHeight ,
                _pRenderInfo->m_srcMod,
                //_pScreenToRefresh->ds_ScreenWindow->RPort,
                _pScreenToRefresh->m_pRenderRastPort,
                            0,ytop,
                _pScreenToRefresh->ds_fullscreenWidth,
                finalheight,_pRenderInfo->m_srcFormat);
        } // end if scale

        //not really enjoyable wait:
//        WaitBOVP( &(_pScreenToRefresh->ds_IntuitionScreen->ViewPort) );

        while( ChangeScreenBuffer(_pScreenToRefresh->ds_IntuitionScreen, _pScreenToRefresh->m_pBuffer2) == 0 )
		{
 //doesnt tick:
            Printf(" double buffer swap error\n");
			WaitBOVP( &(_pScreenToRefresh->ds_IntuitionScreen->ViewPort) );	// wait again.
		}
		// swap screen buffers:
		{

	struct ScreenBuffer *pswap= _pScreenToRefresh->m_pBuffer2;
	_pScreenToRefresh->m_pBuffer2 = _pScreenToRefresh->m_pBuffer1;
	_pScreenToRefresh->m_pBuffer1 = pswap ;

		}
	} else
	{
		// draw to window:
		if(  _pScreenToRefresh->ds_LittleWindow != NULL )
		{
/*
			IP96->p96WritePixelArray( _pRenderInfo,0,0,
								_pScreenToRefresh->ds_LittleWindow->RPort,0,0,
								_pScreenToRefresh->ds_MaxWidth,
								_pScreenToRefresh->ds_MaxHeight	  );
*/
		LONG width=0,height=0;
		width = _pScreenToRefresh->ds_LittleWindow->Width;
        height = _pScreenToRefresh->ds_LittleWindow->Height;
        //OS4:
        //GetWindowAttr(_pScreenToRefresh->ds_LittleWindow,WA_InnerWidth,&width,sizeof(width));
		//GetWindowAttr(_pScreenToRefresh->ds_LittleWindow,WA_InnerHeight,&height,sizeof(height));
//printf("w:%d h:%d\n",width,height);
        ScalePixelArray(_pRenderInfo->m_pPixelBuffer,
            _pRenderInfo->m_srcWidth,
            _pRenderInfo->m_srcHeight ,
            _pRenderInfo->m_srcMod,_pScreenToRefresh->ds_LittleWindow->RPort,
            0,0,width,height,_pRenderInfo->m_srcFormat);

		}
	}

}


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
int osd_create_display(const osd_create_params *params, UINT32 *rgb_components)
{

}
void osd_close_display(void)
{

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
    if(FrameCounter >= NoFrameSkipCount)
    {
      if(FrameCounter < (NoFrameSkipCount + frameskip))
        return(1);
    }

    return(0);
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
    printf("osd_update_video_and_audio\n");

  mame_bitmap *bitmap = display->game_bitmap;

  printf("w:%d h:%d depth:%d rowpixels:%d\n", bitmap->width,bitmap->height,bitmap->depth,bitmap->rowpixels);
  printf("rec:minx:%d miny:%d maxx:%d maxy:%d\n",display->game_visible_area.min_x,display->game_visible_area.min_y,
         display->game_visible_area.max_x,display->game_visible_area.max_y);

  input_update_counter = 0;
  InputUpdate(FALSE);

}


/*
  Provides a hook to allow the OSD system to override processing of a
  snapshot.  This function will either return a new bitmap, for which the
  caller is responsible for freeing.
*/
mame_bitmap *osd_override_snapshot(mame_bitmap *bitmap, rectangle *bounds)
{
    return bitmap;
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


