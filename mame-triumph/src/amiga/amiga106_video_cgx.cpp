/**************************************************************************
 *
 * Copyright (C) 2024 Vic Krb Ferry
 *
 *************************************************************************/

#include "amiga106_video.h"

#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>
#include <proto/intuition.h>

// Amiga
extern "C" {
    #include <cybergraphx/cybergraphics.h>
    #include <intuition/intuition.h>
    #include <intuition/screens.h>
    #include <graphics/modeid.h>
}

extern "C" {
    // from mame
    #include "mame.h"
    #include "video.h"
    #include "mamecore.h"
    #include "osdepend.h"
    #include "palette.h"
}
#include <stdio.h>
#include <stdlib.h>

static void waitsec(int s)
{
    for(int j=0;j<s;j++)
    for(int i=0;i<50;i++)
    {
        WaitTOF();
    }
}

extern struct Library *CyberGfxBase;

// structure to handle the demoscreen object:
typedef struct DemoScreen_ {
        ULONG		ds_ScreenModeID; // guessed from bestMode functions
        ULONG       ds_forcedModeID; // implied by configuration settings.
		unsigned int	ds_MaxWidth;
		unsigned int	ds_MaxHeight;
		unsigned int	ds_fullscreenWidth; // dimension from modeid
		unsigned int	ds_fullscreenHeight;
        unsigned int    ds_fullscreenPixelMode;
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



// create a demoscreen
DemoScreen *InitDemoScreen( unsigned int _maxWidth,
				unsigned int _maxHeight, int depth, unsigned int forcedModeId=0,int startWithWindow=0 ) ;

// close and kill the demoscreen:
void	CloseDemoScreen( DemoScreen *_pScreenToClose );

// if mouse button or escape key pressed, return 0, else: -1
// if 'f' or 'space', change fullscreen/window mode.
//int	CheckDemoScreenState( DemoScreen *_pScreenToCheck );

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


//extern struct Library *CyberGfxBase;

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
	SetPointer( pwindow ,(UWORD *) _pScreen->ds_InvisibleMouseRaster, 0,1,0,0);
	// amiga double buffer stuff:
	_pScreen->m_pBufferAlloc = AllocScreenBuffer(pscreen, NULL,0);
	if(_pScreen->m_pBufferAlloc == NULL) return (0);
	// get current
	_pScreen->m_pBuffer1 = _pScreen->m_pBufferInitial = AllocScreenBuffer(pscreen, NULL, SB_SCREEN_BITMAP);
	_pScreen->m_pBuffer2 = _pScreen->m_pBufferAlloc;

	// alloc rastport to render to hidden bitmap:
	_pScreen->m_pRenderRastPort = (RastPort	*) AllocVec(sizeof(struct RastPort), MEMF_ANY);
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
   IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | IDCMP_RAWKEY, /* IDCMPFlags */
    WFLG_ACTIVATE | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_SIZEGADGET | WFLG_SIZEBBOTTOM |
   WFLG_SIMPLE_REFRESH | WFLG_GIMMEZEROZERO  /*| WINDOWSIZING | WFLG_SIZEBBOTTOM */  /* WFLG_SIZEBRIGHT*/ ,   /* Flags   */
    // windowsizing added if cybergfx
    // gimmezerozero for coordinates inside the window.
    NULL,      /* FirstGadget */
    NULL,      /* CheckMark */
    "MAME",// /* Title */
    NULL,      /* Screen */
    NULL,      /* BitMap */
    100 /*Screen_Width */, 84 /* Screen_Height */ , /* MinWidth, MinHeight */
    -1 /*Screen_Width*/,-1 /*Screen_Height*/,  /* MaxWidth, MaxHeight */
    WBENCHSCREEN, /* Type */
};
static int DemoScreenOpenWBWindow( DemoScreen *pScreen  )
{
	// get default public screen (workbench?)
	struct Screen *pWbScreen;
	if (!(pWbScreen = LockPubScreen(NULL))) return(0); // bad !

	struct Window *pwindow = (Window *)OpenWindowTags(&mynewwin,
		WA_PubScreen,(ULONG)pWbScreen,
		WA_IDCMP, /* IDCMP_MOUSEBUTTONS |*/ IDCMP_RAWKEY |
		IDCMP_CLOSEWINDOW ,
		WA_InnerWidth,pScreen->ds_MaxWidth,
		WA_InnerHeight,pScreen->ds_MaxHeight,
		TAG_DONE );
	if( pwindow == NULL ) return(0);
	pScreen->ds_LittleWindow = pwindow ;
	pScreen->ds_CurrentPort  = pwindow->UserPort ;

    UnlockPubScreen(NULL,pWbScreen);
   /* if cybergraphics  */
 /*   if (CyberGfxBase)
    {
        mynewwin.Flags |= WINDOWSIZING | WFLG_SIZEBBOTTOM  ;
    }
*/
	return(-1);	//OK !
}
// private one. Close the screen for window switch:
static void DemoScreenCloseWBWindow( DemoScreen *pScreenToClose  )
{
	if( pScreenToClose->ds_LittleWindow )  CloseWindow( pScreenToClose->ds_LittleWindow );
	pScreenToClose->ds_LittleWindow = NULL ;
}

static void SwitchFullDemoScreen( DemoScreen *pScreenToSwitch  )
{
	if( pScreenToSwitch->ds_ScreenWindow != NULL   )
	{
		// switch from screen mode to window mode:
		DemoScreenCloseFullScreen( pScreenToSwitch );
		DemoScreenOpenWBWindow( pScreenToSwitch );
	}else
	{
		// switch from window mode to screen mode:
		DemoScreenCloseWBWindow( pScreenToSwitch );
		DemoScreenOpenFullScreen( pScreenToSwitch );
	}
}

DemoScreen *InitDemoScreen( unsigned int maxWidth,
				unsigned int maxHeight, int depth, unsigned int forcedModeId, int startWithWindow )
{
	DemoScreen	*pDemoScreen = (DemoScreen *) calloc( sizeof( DemoScreen ) , 1  );
	if( pDemoScreen  == NULL ) return(NULL);

	// prepare window mode new struct:
	mynewwin.Width  = maxWidth;
	mynewwin.Height = maxHeight;
	if(depth == 15 ) depth=16;

//printf("init: %d %d\n", _maxWidth,_maxHeight);
	// ------------ open Cybergraphics.library:

    if( CyberGfxBase ==  NULL ){
        CloseDemoScreen(pDemoScreen);
        logerror("Can't find cybergraphics.library.");
        return(NULL);
     }

	// ----------- alloc invisible mouse raster for full screen:
	void *pmouseraster =  AllocRaster(8 ,8) ;
	if( pmouseraster == NULL ) { CloseDemoScreen(pDemoScreen);  return (NULL); }
	pDemoScreen->ds_InvisibleMouseRaster = pmouseraster ;

	// ---------------- search screen best mode:
    printf("search res: %d %d\n",maxWidth,maxHeight);
    ULONG modeid = forcedModeId;
    if(modeid == INVALID_ID)
	{
 		 struct TagItem cgxtags[]={
			CYBRBIDTG_NominalWidth,maxWidth,
			CYBRBIDTG_NominalHeight,maxHeight,
			CYBRBIDTG_Depth,depth,
			TAG_DONE,0 };
        ULONG guessedid = BestCModeIDTagList(cgxtags);
        modeid = guessedid;
    }
    if(modeid == INVALID_ID)
    {
        logerror("Can't find cyber screen mode for %d %d %d ",maxWidth,maxHeight,depth);
        CloseDemoScreen(pDemoScreen);
        return (NULL);
    }

    pDemoScreen->ds_ScreenModeID 	= modeid ;
    pDemoScreen->ds_forcedModeID 	= forcedModeId ;

    pDemoScreen->ds_MaxWidth    = maxWidth ;
    pDemoScreen->ds_MaxHeight	= maxHeight ;
    // get size of modeid screen:
    pDemoScreen->ds_fullscreenWidth = GetCyberIDAttr( CYBRIDATTR_WIDTH, modeid);
    pDemoScreen->ds_fullscreenHeight = GetCyberIDAttr( CYBRIDATTR_HEIGHT, modeid );
    pDemoScreen->ds_fullscreenPixelMode = GetCyberIDAttr( CYBRIDATTR_PIXFMT, modeid );
 printf("final resolution:%d %d render res:%d %d pixelmode:%d\n",
        pDemoScreen->ds_fullscreenWidth,
        pDemoScreen->ds_fullscreenHeight,
        pDemoScreen->ds_MaxWidth,
        pDemoScreen->ds_MaxHeight,
        pDemoScreen->ds_fullscreenPixelMode);


//	printf("modeid:%08x \n",modeid);

	// ---------------- open full screen, or window:
	if(startWithWindow)
	{
        if(! DemoScreenOpenWBWindow( pDemoScreen ) ) { CloseDemoScreen(pDemoScreen);  return (NULL); }
	} else
	{
    	if(! DemoScreenOpenFullScreen( pDemoScreen ) ) { CloseDemoScreen(pDemoScreen);  return (NULL); }
	}

	return( pDemoScreen );
}
void	CloseDemoScreen( DemoScreen *_pScreenToClose )
{
	if( _pScreenToClose == NULL  ) return;

	DemoScreenCloseFullScreen( _pScreenToClose );
	DemoScreenCloseWBWindow( _pScreenToClose );

	if( _pScreenToClose->ds_InvisibleMouseRaster   ) FreeRaster( (PLANEPTR) _pScreenToClose->ds_InvisibleMouseRaster ,8,8);

	// close libs:
/*
	if( _pScreenToClose->ds_CGXVI  )  DropInterface((struct Interface *) _pScreenToClose->ds_CGXVI );
	if( _pScreenToClose->ds_CGXVideoLib   )  CloseLibrary( _pScreenToClose->ds_CGXVideoLib );
*/
	free( _pScreenToClose );
}
/* ======================= */
// if mouse button or escape key pressed, return 0, else -1.
// if 'f' or 'space', change fullscreen/window mode.
/* from demo source
int	CheckDemoScreenState( DemoScreen *pScreenToCheck )
{
	struct IntuiMessage *iMsg;
	struct MsgPort *pport = pScreenToCheck->ds_CurrentPort ;
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
			if( ICode == 0x0023 || ICode == 0x0040 ) SwitchFullDemoScreen( pScreenToCheck );
			// if ( ICode == 0x0021 ) GrabScreen();
		}
		// window mode gadget exit case:
		if ( IClass == IDCMP_CLOSEWINDOW ) return(0);
	}
	return(-1); // OK to continue.
}*/
/* ======================= */
void	RefreshDemoScreen( DemoScreen *pScreenToRefresh,  sRenderInfo *pRenderInfo )
{
	if( pScreenToRefresh->ds_ScreenWindow != NULL   )
	{
        pScreenToRefresh->m_pRenderRastPort->BitMap = pScreenToRefresh->m_pBuffer2->sb_BitMap;

        if(pRenderInfo->m_srcWidth == pScreenToRefresh->ds_fullscreenWidth)
        {
            // in that case no scale
            uint32_t finalheight = pRenderInfo->m_srcHeight;
            uint32_t ytop = (pScreenToRefresh->ds_fullscreenHeight-finalheight)>>1;

            WritePixelArray(pRenderInfo->m_pPixelBuffer,
                            0,0, // starting point in source rectangle
                            pRenderInfo->m_srcMod, // bytes per row in source

                            pScreenToRefresh->m_pRenderRastPort, // rastport to render
                            0,ytop, // start point in raster
                            pRenderInfo->m_srcWidth,
                            pRenderInfo->m_srcHeight,
                            pRenderInfo->m_srcFormat
                            );
//            /*
//            ULONG        WritePixelArray(APTR, UWORD, UWORD, UWORD, struct RastPort *, UWORD,
//                                         UWORD, UWORD, UWORD, UBYTE);
//        */
        } else
        {
            // scale
            uint32_t finalheight = (pScreenToRefresh->ds_fullscreenHeight * pRenderInfo->m_RectangleRatio)>>16;
            uint32_t ytop = (pScreenToRefresh->ds_fullscreenHeight-finalheight)>>1;

            ScalePixelArray(pRenderInfo->m_pPixelBuffer,
                pRenderInfo->m_srcWidth,
                pRenderInfo->m_srcHeight ,
                pRenderInfo->m_srcMod,
                //_pScreenToRefresh->ds_ScreenWindow->RPort,
                pScreenToRefresh->m_pRenderRastPort,
                            0,ytop,
                pScreenToRefresh->ds_fullscreenWidth,
                finalheight,pRenderInfo->m_srcFormat);
        } // end if scale

        //not really enjoyable wait:
//        WaitBOVP( &(_pScreenToRefresh->ds_IntuitionScreen->ViewPort) );

        while( ChangeScreenBuffer(pScreenToRefresh->ds_IntuitionScreen, pScreenToRefresh->m_pBuffer2) == 0 )
		{
 //doesnt tick:
            printf(" double buffer swap error\n");
			WaitBOVP( &(pScreenToRefresh->ds_IntuitionScreen->ViewPort) );	// wait again.
		}
		// swap screen buffers:
		{

	struct ScreenBuffer *pswap= pScreenToRefresh->m_pBuffer2;
	pScreenToRefresh->m_pBuffer2 = pScreenToRefresh->m_pBuffer1;
	pScreenToRefresh->m_pBuffer1 = pswap ;

		}
	} else
	{
		// draw to window:
		if(  pScreenToRefresh->ds_LittleWindow != NULL )
		{
/*
			IP96->p96WritePixelArray( _pRenderInfo,0,0,
								_pScreenToRefresh->ds_LittleWindow->RPort,0,0,
								_pScreenToRefresh->ds_MaxWidth,
								_pScreenToRefresh->ds_MaxHeight	  );
*/
		LONG width=0,height=0;
		width = pScreenToRefresh->ds_LittleWindow->Width;
        height = pScreenToRefresh->ds_LittleWindow->Height;
        //OS4:
        //GetWindowAttr(_pScreenToRefresh->ds_LittleWindow,WA_InnerWidth,&width,sizeof(width));
		//GetWindowAttr(_pScreenToRefresh->ds_LittleWindow,WA_InnerHeight,&height,sizeof(height));
//printf("w:%d h:%d\n",width,height);
        ScalePixelArray(pRenderInfo->m_pPixelBuffer,
            pRenderInfo->m_srcWidth,
            pRenderInfo->m_srcHeight ,
            pRenderInfo->m_srcMod,pScreenToRefresh->ds_LittleWindow->RPort,
            0,0,width,height,pRenderInfo->m_srcFormat);

		}
	}

}

Display_Intuition::Display_Intuition(const _osd_create_params *params) : MameDisplay()
    , _pScreen(NULL)
    , _pScreenWindow(NULL)
    , _ScreenModeId(INVALID_ID)
    , _fullscreenWidth(0)
    , _fullscreenHeight(0)
    , _pixelFmt(0)
    , _pMouseRaster(NULL)
    , _pWbWindow(NULL)
    , _machineWidth(params->width),_machineHeight(params->height)
{}
Display_Intuition::~Display_Intuition()
{
    closeWindow();
    closeScreen();
}
void Display_Intuition::openWindow()
{
    if(_pWbWindow) return;

    closeScreen();

    Screen *pWbScreen;
    if (!(pWbScreen = LockPubScreen(NULL))) return;

    int xcen = (pWbScreen->Width - _machineWidth);
    int ycen = (pWbScreen->Height - _machineHeight);
    if(xcen<0) xcen=0;
    xcen>>=1;
    if(ycen<0) ycen=0;
    ycen>>=1;
    printf("openWindow:_machineWidth:%d _machineHeight:%d xcen:%d ycen:%d \n",_machineWidth,_machineHeight,xcen,ycen);

        _pWbWindow = (Window *)OpenWindowTags(NULL,
        WA_Left,xcen,
        WA_Top,ycen,
        WA_Width, _machineWidth,
        WA_Height, _machineHeight,
    //    WA_MaxWidth,  WIDTH_SUPER,
    //    WA_MaxHeight, HEIGHT_SUPER,
        WA_IDCMP, IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_RAWKEY |
            IDCMP_NEWSIZE | IDCMP_INTUITICKS | IDCMP_CLOSEWINDOW,
        WA_Flags, WFLG_SIZEGADGET | WFLG_SIZEBRIGHT | WFLG_SIZEBBOTTOM |
            WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_ACTIVATE |
            /*WFLG_SUPER_BITMAP |*/ WFLG_GIMMEZEROZERO /*| WFLG_NOCAREREFRESH*/,
     //   WA_Gadgets, &(SideGad),
        WA_Title,(ULONG) "Mame", /* take title from version string */
        WA_PubScreen, (ULONG)pWbScreen,
       // WA_SuperBitMap, bigBitMap,
        TAG_DONE
        );
    UnlockPubScreen(NULL,pWbScreen);

    if( _pWbWindow == NULL ) return;
    _pUserPort = _pWbWindow->UserPort;
}
void Display_Intuition::closeWindow()
{
    if(_pWbWindow) CloseWindow(_pWbWindow);
    _pWbWindow = NULL;
    _pUserPort = NULL;
}
void Display_Intuition::openScreen()
{
    if(_pScreenWindow) return;
    closeWindow();
    if(_ScreenModeId == INVALID_ID) return; // set by inherited class.

	struct ColorSpec colspec[2]={0,0,0,0,-1,0,0,0};
 	_pScreen = OpenScreenTags( NULL,
			SA_DisplayID,_ScreenModeId,
                        SA_Width, _fullscreenWidth,
                        SA_Height,_fullscreenHeight,
//                        SA_Behind,TRUE,    /* Open behind */
                        SA_Quiet,TRUE,     /* quiet */
			SA_Type,CUSTOMSCREEN,
			SA_Colors,(ULONG)&colspec[0],
                        0 );

	if( _pScreen == NULL ) return;

	// --------- open intuition fullscreen window for this screen:

    _pScreenWindow = OpenWindowTags(/*&screenwin*/NULL,
        WA_CustomScreen,(ULONG)_pScreen,
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

	if( _pScreenWindow ==  NULL )
	{
        closeScreen();
        return ;
	}
	// --------- note the Current userPort:
    _pUserPort = _pScreenWindow->UserPort;
	// ------- set invisible mouse pointer:
	_pMouseRaster =  AllocRaster(8 ,8) ;
	if(_pMouseRaster)
	{
        SetPointer( _pScreenWindow ,(UWORD *) _pMouseRaster, 0,1,0,0);
    }

}
void Display_Intuition::closeScreen()
{
    if(_pScreenWindow) CloseWindow(_pScreenWindow);
    if(_pScreen) CloseScreen(_pScreen);
    if(_pMouseRaster) FreeRaster( (PLANEPTR) _pMouseRaster ,8,8);

    _pScreenWindow = NULL;
    _pScreen = NULL;
    _pMouseRaster = NULL;
    _pUserPort = NULL;
}


// - - - -

Display_CGX_Paletted::Display_CGX_Paletted(const _osd_create_params *params, ULONG forcedModeID)
    : Display_Intuition(params),_needFirstRemap(1)
{
    printf("Display_CGX_Paletted()\n");

    if(!CyberGfxBase) return;
    int width = params->width;
    int height = params->height;
    printf(" ***** palette nbc:%d\n",params->colors);

    int screenDepth = (params->colors<=256)?8:16; // more would be Display_CGX_TrueColor.

    _ScreenModeId = forcedModeID;
    if(_ScreenModeId == INVALID_ID)
    {
         struct TagItem cgxtags[]={
                CYBRBIDTG_NominalWidth,width,
                CYBRBIDTG_NominalHeight,height,
                CYBRBIDTG_Depth,screenDepth,
                TAG_DONE,0 };
             printf("bef BestCModeIDTagList()\n");

        _ScreenModeId = BestCModeIDTagList(cgxtags);
           printf("aft BestCModeIDTagList()\n");
               fflush(stdout);
    }
    if(_ScreenModeId == INVALID_ID)
    {
        logerror("Can't find cyber screen mode for w%d h%d d%d ",width,height,screenDepth);
        return;
    }
    _fullscreenWidth = GetCyberIDAttr( CYBRIDATTR_WIDTH, _ScreenModeId );
    _fullscreenHeight = GetCyberIDAttr( CYBRIDATTR_HEIGHT, _ScreenModeId );
    _pixelFmt = GetCyberIDAttr( CYBRIDATTR_PIXFMT, _ScreenModeId );
    _pixelbytes = GetCyberIDAttr( CYBRIDATTR_BPPIX, _ScreenModeId );
    if(_pixelbytes==3) _pixelbytes=4;
    // actually always work this way it seems:
    _clut16.reserve(65536);
    _clut16.resize(65536); // exact alloc
    // initialize the palette to a fixed 5-5-5 mapping
	for (int r = 0; r < 32; r++)
		for (int g = 0; g < 32; g++)
			for (int b = 0; b < 32; b++)
			{
				int idx = (((r << 10) | (g << 5) | b)+32768)& 0x0000ffff;

                // according to pixfmt:
                USHORT d = (((USHORT)r<<11)&0xf800)|(((USHORT)g<<5)&0x07e0)|((USHORT)b&0x001f);

				_clut16[idx] = ((d>>8)&0x00ff)|((d<<8)&0xff00); // win_color16(rr, gg, bb) * 0x10001;
				//palette_32bit_lookup[idx] = win_color32(rr, gg, bb);
			}
    //_clut16.reserve(params->colors);

    printf(" ** gw:%d gh:%d final res %d %d\n",width,height,_fullscreenWidth,_fullscreenHeight);
    printf(" ** reseve clut:%d\n",params->colors);
   // _fullscreenPixelMode = GetCyberIDAttr( CYBRIDATTR_PIXFMT, _ScreenModeId );


//	int width, height;			/* width and height */
//	int aspect_x, aspect_y;		/* aspect ratio X:Y */
//	int depth;					/* depth, either 16(palette), 15(RGB) or 32(RGB) */
//	int colors;					/* colors in the palette (including UI) */
//	float fps;					/* frame rate */
//	int video_attributes;		/* video flags from driver */


}
Display_CGX_Paletted::~Display_CGX_Paletted()
{

}
void Display_CGX_Paletted::updatePaletteRemap(_mame_display *display)
{
    int i, j;
 //no   if(_needFirstRemap && _clut16.size()<display->game_palette_entries) _clut16.resize(display->game_palette_entries);
	// loop over dirty colors in batches of 32
	for (i = 0; i < display->game_palette_entries; i += 32)
	{
		UINT32 dirtyflags = _needFirstRemap ? ~0 : display->game_palette_dirty[i>>5];
		if (dirtyflags)
		{
			display->game_palette_dirty[i>>5] = 0;
            const rgb_t *gpal = display->game_palette + i ;

            switch(_pixelFmt)
            {
            // - - - - -15b cases
//             case PIXFMT_RGB15:
//                for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c>>9)&0x7c00)|((c>>6)&0x03e0)|((c>>3)&0x001f); }
//                break;
//             case PIXFMT_BGR15:
//                for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c<<7)&0x7c00)|((c>>6)&0x03e0)|((c>>19)&0x001f); }
//                break;
//             case PIXFMT_RGB15PC:
//                for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d = ((c>>9)&0x7c00)|((c>>6)&0x03e0)|((c>>3)&0x001f);
//                    *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
//                }
//                break;
//             case PIXFMT_BGR15PC:
//                for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d = ((c<<7)&0x7c00)|((c>>6)&0x03e0)|((c>>19)&0x001f);
//                    *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
//                }
//                break;
             // - -- - - - 16b cases
             case PIXFMT_RGB16:
//                for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c>>8)&0xf800)|((c>>5)&0x07e0)|((c>>3)&0x001f); }
//                break;
             case PIXFMT_BGR16:
//                for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c<<8)&0xf800)|((c>>5)&0x07e0)|((c>>19)&0x001f); }
//                break;
             case PIXFMT_RGB16PC:          //  *p++ = d;
//                for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d = ((c>>8)&0xf800)|((c>>5)&0x07e0)|((c>>3)&0x001f);
//        //           USHORT d = ((c>>9)&0xf800);
//        //            *pb++ = (UBYTE)d; *pb++ = (UBYTE)(d>>8);
//                }
                for (j = 0; (j < 32) && (i+j < display->game_palette_entries); j++, dirtyflags >>= 1)
                    if (dirtyflags & 1)
                    {
                        ULONG c = *gpal++; USHORT d = ((c>>8)&0xf800)|(((USHORT)c>>5)&0x07e0)|(((USHORT)c>>3)&0x001f);
                       _clut16[i+j+32768]=((d>>8)&0x00ff)|((d<<8)&0xff00);
                       // _clut16[32768+i+j]=d;
                        // extract the RGB values
//                        rgb_t rgbvalue = display->game_palette[i + j];
//                        int r = RGB_RED(rgbvalue);
//                        int g = RGB_GREEN(rgbvalue);
//                        int b = RGB_BLUE(rgbvalue);
                        // update both lookup tables
                        //palette_16bit_lookup[i + j] = win_color16(r, g, b) * 0x10001;
                        //palette_32bit_lookup[i + j] = win_color32(r, g, b);
                    }
                break;
             case PIXFMT_BGR16PC:
//                for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d =  ((c<<8)&0xf800)|((c>>5)&0x07e0)|((c>>19)&0x001f);
//                    *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
//                }
                break;
//             // - - -24b cases, also use 32bit source
//             case PIXFMT_RGB24:
//             case PIXFMT_BGR24:
//             // - - -32b cases
//             case PIXFMT_ARGB32:
//                // this is the id one, no need for table, direct palette use.
//                break;
//             case PIXFMT_BGRA32:
//             case PIXFMT_RGBA32:
//                break;
//             case PIXFMT_LUT8: // no sense, should just use RGB32 os palette, do not select Display_CGX_Paletted
            default:
                break;
            }

			// loop over all 32 bits and update dirty entries
			for (j = 0; (j < 32) && (i+j < display->game_palette_entries); j++, dirtyflags >>= 1)
				if (dirtyflags & 1)
				{
					// extract the RGB values
					rgb_t rgbvalue = display->game_palette[i + j];
					int r = RGB_RED(rgbvalue);
					int g = RGB_GREEN(rgbvalue);
					int b = RGB_BLUE(rgbvalue);

					// update both lookup tables
					//palette_16bit_lookup[i + j] = win_color16(r, g, b) * 0x10001;
					//palette_32bit_lookup[i + j] = win_color32(r, g, b);
				}
		} // if batch dirty
	} // loop per 32 batch

	// reset the invalidate flag
	_needFirstRemap = 0;

    /*
    const rgb_t *gpal = pmame_display->game_palette;

    USHORT *p=(USHORT *)_clut;
    UBYTE *pb=(UBYTE *)_clut;
    int i=0;
    const ULONG nbc= pmame_display->game_palette_entries;
    switch(_pixelFmt)
    {
    // - - - - -15b cases
     case PIXFMT_RGB15:
        for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c>>9)&0x7c00)|((c>>6)&0x03e0)|((c>>3)&0x001f); }
        break;
     case PIXFMT_BGR15:
        for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c<<7)&0x7c00)|((c>>6)&0x03e0)|((c>>19)&0x001f); }
        break;
     case PIXFMT_RGB15PC:
        for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d = ((c>>9)&0x7c00)|((c>>6)&0x03e0)|((c>>3)&0x001f);
            *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
        }
        break;
     case PIXFMT_BGR15PC:
        for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d = ((c<<7)&0x7c00)|((c>>6)&0x03e0)|((c>>19)&0x001f);
            *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
        }
        break;
     // - -- - - - 16b cases
     case PIXFMT_RGB16:
        for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c>>8)&0xf800)|((c>>5)&0x07e0)|((c>>3)&0x001f); }
        break;
     case PIXFMT_BGR16:
        for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c<<8)&0xf800)|((c>>5)&0x07e0)|((c>>19)&0x001f); }
        break;
     case PIXFMT_RGB16PC:          //  *p++ = d;
        for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d = ((c>>8)&0xf800)|((c>>5)&0x07e0)|((c>>3)&0x001f);
//           USHORT d = ((c>>9)&0xf800);
//            *pb++ = (UBYTE)d; *pb++ = (UBYTE)(d>>8);
        }
        break;
     case PIXFMT_BGR16PC:
        for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d =  ((c<<8)&0xf800)|((c>>5)&0x07e0)|((c>>19)&0x001f);
            *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
        }
        break;
     // - - -24b cases, also use 32bit source
     case PIXFMT_RGB24:
     case PIXFMT_BGR24:
     // - - -32b cases
     case PIXFMT_ARGB32:
        // this is the id one, no need for table, direct palette use.
        break;
     case PIXFMT_BGRA32:
     case PIXFMT_RGBA32:
        break;
     case PIXFMT_LUT8: // no sense, should just use RGB32 os palette, do not select Display_CGX_Paletted
    default:
        break;
    }
    */
}
struct directDrawScreen {
    void *_base;
    ULONG _bpr;
    WORD _clipX1,_clipY1,_clipX2,_clipY2;
};
struct directDrawSource {
    void *_base;
    ULONG _bpr;
    WORD _x1,_y1,_x2,_y2; // to be drawn.
};
extern "C" {
    //extern int asmval,asmval2;
    // for any of the 8x 16b target mode.
    void directDrawClut16(register directDrawScreen *screen __asm("a0"),
                    register directDrawSource *source __asm("a1"),
                    register LONG x1 __asm("d0"),
                    register LONG y1 __asm("d1"),
                    register USHORT *lut __asm("a2") // actually UWORD* or anywhat.
                );
}
void Display_CGX_Paletted::draw(_mame_display *pmame_display)
{
    static int counter=0;
    counter++;
    /* pixfmt constated:
     *  UAE picasso : WB PIXFMT_BGRA32 , asked 16b: PIXFMT_RGB16PC
    */

    if((pmame_display->changed_flags & GAME_PALETTE_CHANGED) !=0 || _needFirstRemap)
    {
       // printf("*** DO palette remap:%d\n",counter);
        counter=0;
        updatePaletteRemap(pmame_display);
    }

    mame_bitmap *bitmap = pmame_display->game_bitmap;
    if(_pWbWindow && _pWbWindow->RPort->BitMap)
    {
        int width,height,depth,pixfmt,bpr;
        APTR pc;
        APTR hdl = LockBitMapTags(_pWbWindow->RPort->BitMap,
                                  LBMI_WIDTH,(ULONG)&width,
                                  LBMI_HEIGHT,(ULONG)&height,
                                  LBMI_DEPTH,(ULONG)&depth,
                                  LBMI_PIXFMT,(ULONG)&pixfmt,
                                  //LBMI_BYTESPERPIX,(ULONG)&,
                                  LBMI_BYTESPERROW,(ULONG)&bpr,
                                  LBMI_BASEADDRESS,(ULONG)&pc,
                                  TAG_DONE);
        if(hdl) {

            UnLockBitMap(hdl);
        }
       // printf("win pixfmt:%d\n",pixfmt);
    }
    if(_pScreen)
    {
        /*

LBMI_WIDTH
    (PLongWord) Points to a longword which contains the bitmap width after a succesful call
LBMI_HEIGHT
    (PLongWord) Points to a longword which contains the bitmap height after a succesful call
LBMI_DEPTH
    (PLongWord) Points to a longword which contains the bitmap depth after a succesful call
LBMI_PIXFMT
    (PLongWord) Points to a longword which contains the usedpixel format.
Possibly returned colormodels are:
        PIXFMT_LUT8,
        PIXFMT_RGB15,
        PIXFMT_BGR15, PIXFMT_RGB15PC, PIXFMT_BGR15PC, PIXFMT_RGB16, PIXFMT_BGR16, PIXFMT_RGB16PC, PIXFMT_BGR16PC, PIXFMT_RGB24, PIXFMT_BGR24, PIXFMT_ARGB32, PIXFMT_BGRA32, PIXFMT_RGBA32
LBMI_BYTESPERPIX
    (PLongWord) points to a longword which contains the amount of bytes per pixel data.
LBMI_BYTESPERROW
    (PLongWord) points to a longword which contains the number of bytes per row for one bitmap line
LBMI_BASEADDRESS
    (PLongWord) points to a longword which contains the bitmap base address. This address is only valid inside of the Lock/UnLockBitmap call!
*/
        directDrawScreen ddscreen;
        int width,height,depth,pixfmt,bpr;
        APTR hdl = LockBitMapTags(_pScreen->RastPort.BitMap,
                                  LBMI_WIDTH,(ULONG)&width,
                                  LBMI_HEIGHT,(ULONG)&height,
                                  LBMI_DEPTH,(ULONG)&depth,
                                  LBMI_PIXFMT,(ULONG)&pixfmt,
                                  //LBMI_BYTESPERPIX,(ULONG)&,
                                  LBMI_BYTESPERROW,(ULONG)&ddscreen._bpr,
                                  LBMI_BASEADDRESS,(ULONG)&ddscreen._base,
                                  TAG_DONE);
        if(hdl) {
           ddscreen._clipX1 = 10;
           ddscreen._clipY1 = 10;
           ddscreen._clipX2 = (WORD)width-10;
           ddscreen._clipY2 = (WORD)height-10;
//struct directDrawSource {
//    void *_base;
//    ULONG _bpr;
//    WORD _x1,_y1,_width,_height; // to be drawn.
//};
           // +1 because goes 0,319
            int sourcewidth = (pmame_display->game_visible_area.max_x-pmame_display->game_visible_area.min_x)+1;
            int sourceheight =( pmame_display->game_visible_area.max_y-pmame_display->game_visible_area.min_y)+1;

            int cenx = width-sourcewidth;
            int ceny = height-sourceheight;
            if(cenx<0) cenx = 0;
            if(ceny<0) ceny = 0;
            cenx>>=1;
            ceny>>=1;

            directDrawSource ddsource={bitmap->base,bitmap->rowbytes,
                pmame_display->game_visible_area.min_x,pmame_display->game_visible_area.min_y,
                pmame_display->game_visible_area.max_x+1,pmame_display->game_visible_area.max_y+1
            };
            switch(pixfmt) {
             case PIXFMT_RGB15:
             case PIXFMT_BGR15:
             case PIXFMT_RGB15PC:
             case PIXFMT_BGR15PC:
             case PIXFMT_RGB16:
             case PIXFMT_BGR16:
             case PIXFMT_RGB16PC:
             case PIXFMT_BGR16PC:

                directDrawClut16(&ddscreen,&ddsource,cenx,ceny,_clut16.data()+32768);
                break;
            default:
                break;
            }

            UnLockBitMap(hdl);
//            printf("minx:%d maxx:%d miny:%d maxy:%d\n",pmame_display->game_visible_area.min_x,
//                   pmame_display->game_visible_area.max_x,pmame_display->game_visible_area.min_y,
//                   pmame_display->game_visible_area.max_y);
            // printf("asmval:%d %d pxf:%d\n",asmval,asmval2,pixfmt);

        } // end if lock
             //     printf("clut:%d\n",pixfmt);
        //printf("pixfmt:%d\n",pixfmt);
    }
}

// - - - -

Display_CGX_TrueColor::Display_CGX_TrueColor(const _osd_create_params *params, ULONG forcedModeID)
    : Display_Intuition(params)
{

    if(!CyberGfxBase) return;
    int width = params->width;
    int height = params->height;

//	int width, height;			/* width and height */
//	int aspect_x, aspect_y;		/* aspect ratio X:Y */
//	int depth;					/* depth, either 16(palette), 15(RGB) or 32(RGB) */
//	int colors;					/* colors in the palette (including UI) */
//	float fps;					/* frame rate */
//	int video_attributes;		/* video flags from driver */

    int depth = (params->depth==32)?32:16; //15 means 16 for CGX.

    _ScreenModeId = forcedModeID;
    if(_ScreenModeId == INVALID_ID)
	{
 		 struct TagItem cgxtags[]={
			CYBRBIDTG_NominalWidth,width,
			CYBRBIDTG_NominalHeight,height,
			CYBRBIDTG_Depth,depth,
			TAG_DONE,0 };
        _ScreenModeId = BestCModeIDTagList(cgxtags);
    }
    if(_ScreenModeId == INVALID_ID)
    {
        logerror("Can't find cyber screen mode for w%d h%d d%d ",width,height,depth);
        return;
    }

}
Display_CGX_TrueColor::~Display_CGX_TrueColor()
{
    if(_pWbWindow)
    {

    }

}
void Display_CGX_TrueColor::draw(_mame_display *pmame_display)
{
    mame_bitmap *bitmap = pmame_display->game_bitmap;
    // should be better with P96
}

