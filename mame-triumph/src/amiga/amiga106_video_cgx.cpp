/**************************************************************************
 *
 * Copyright (C) 2024 Vic Krb Ferry
 *
 *************************************************************************/

#include "amiga106_video_cgx.h"

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

//static void waitsec(int s)
//{
//    for(int j=0;j<s;j++)
//    for(int i=0;i<50;i++)
//    {
//        WaitTOF();
//    }
//}

extern struct Library *CyberGfxBase;

// - - - - from driver.h
/* is the video hardware raser or vector base? */
#define	VIDEO_TYPE_RASTER				0x0000
#define	VIDEO_TYPE_VECTOR				0x0001

/* should VIDEO_UPDATE by called at the start of VBLANK or at the end? */
#define	VIDEO_UPDATE_BEFORE_VBLANK		0x0000
#define	VIDEO_UPDATE_AFTER_VBLANK		0x0002

/* set this to use a direct RGB bitmap rather than a palettized bitmap */
#define VIDEO_RGB_DIRECT	 			0x0004


//Display_Intuition_Screen::Display_Intuition_Screen(const _osd_create_params *params) : MameDisplay()
//    , _pScreen(NULL)
//    , _pScreenWindow(NULL)
//    , _ScreenModeId(INVALID_ID)
//    , _fullscreenWidth(0)
//    , _fullscreenHeight(0)
//    , _PixelFmt(0)
//    , _pixelbytes(0)
//    , _pMouseRaster(NULL)
//{}
//Display_Intuition_Screen::~Display_Intuition_Screen()
//{
//    close();
//}
//void Display_Intuition_Screen::open()
//{
//    if(_pScreenWindow) return;

//    if(_ScreenModeId == INVALID_ID) return; // set by inherited class.

//	struct ColorSpec colspec[2]={0,0,0,0,-1,0,0,0};
// 	_pScreen = OpenScreenTags( NULL,
//			SA_DisplayID,_ScreenModeId,
//                        SA_Width, _fullscreenWidth,
//                        SA_Height,_fullscreenHeight,
////                        SA_Behind,TRUE,    /* Open behind */
//                        SA_Quiet,TRUE,     /* quiet */
//			SA_Type,CUSTOMSCREEN,
//			SA_Colors,(ULONG)&colspec[0],
//                        0 );

//	if( _pScreen == NULL ) return;

//	// --------- open intuition fullscreen window for this screen:

//    _pScreenWindow = OpenWindowTags(/*&screenwin*/NULL,
//        WA_CustomScreen,(ULONG)_pScreen,
//                    WA_Backdrop,FALSE,
//                    WA_Borderless,TRUE,
//                    WA_Activate,TRUE,
//                    WA_RMBTrap,TRUE,
//                    WA_ReportMouse,0,
//                    WA_SizeGadget,0,
//                    WA_DepthGadget,0,
//                    WA_CloseGadget,0,
//                    WA_DragBar,0,
//          WA_GimmeZeroZero,FALSE, // test
//                    WA_IDCMP,IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY ,
//                    0 );

//	if( _pScreenWindow ==  NULL )
//	{
//        close();
//        return ;
//	}
//	// --------- note the Current userPort:
//    _pUserPort = _pScreenWindow->UserPort;
//	// ------- set invisible mouse pointer:
//	_pMouseRaster =  AllocRaster(8 ,8) ;
//	if(_pMouseRaster)
//	{
//        SetPointer( _pScreenWindow ,(UWORD *) _pMouseRaster, 0,1,0,0);
//    }

//}
//void Display_Intuition_Screen::close()
//{
//    if(_pScreenWindow) CloseWindow(_pScreenWindow);
//    if(_pScreen) CloseScreen(_pScreen);
//    if(_pMouseRaster) FreeRaster( (PLANEPTR) _pMouseRaster ,8,8);

//    _pScreenWindow = NULL;
//    _pScreen = NULL;
//    _pMouseRaster = NULL;
//    _pUserPort = NULL;
//}


//Display_Intuition_Window::Display_Intuition_Window(const _osd_create_params *params) : MameDisplay()
//    , _pWbWindow(NULL)
//    , _sWbWinSBitmap(NULL)
//    , _machineWidth(params->width),_machineHeight(params->height)
//    , _PixFmt(0),_PixBytes(0)
//{}
//Display_Intuition_Window::~Display_Intuition_Window()
//{
//    close();
//}
//void Display_Intuition_Window::open()
//{
//    if(_pWbWindow) return;

//    Screen *pWbScreen;
//    if (!(pWbScreen = LockPubScreen(NULL))) return;

//    int xcen = (pWbScreen->Width - _machineWidth);
//    int ycen = (pWbScreen->Height - _machineHeight);
//    if(xcen<0) xcen=0;
//    xcen>>=1;
//    if(ycen<0) ycen=0;
//    ycen>>=1;
//    printf("openWindow:_machineWidth:%d _machineHeight:%d xcen:%d ycen:%d \n",_machineWidth,_machineHeight,xcen,ycen);

//// struct BitMap * __stdargs AllocBitMap( ULONG sizex, ULONG sizey, ULONG depth, ULONG flags, CONST struct BitMap *friend_bitmap );

//    _sWbWinSBitmap = AllocBitMap(_machineWidth,_machineHeight,
//            pWbScreen->RastPort.BitMap->Depth,BMF_CLEAR|BMF_DISPLAYABLE,pWbScreen->RastPort.BitMap);
//    if(_sWbWinSBitmap) {

//        _pWbWindow = (Window *)OpenWindowTags(NULL,
//        WA_Left,xcen,
//        WA_Top,ycen,
//     //   WA_Width, _machineWidth,
//     //   WA_Height, _machineHeight,
//        WA_InnerWidth, _machineWidth,
//        WA_InnerHeight, _machineHeight,
//    //    WA_MaxWidth,  WIDTH_SUPER,
//    //    WA_MaxHeight, HEIGHT_SUPER,
//        WA_IDCMP,/* IDCMP_GADGETUP | IDCMP_GADGETDOWN |*/IDCMP_MOUSEBUTTONS |  IDCMP_RAWKEY /*|
//            IDCMP_NEWSIZE*/ /*| IDCMP_INTUITICKS*/ | IDCMP_CLOSEWINDOW,

//        WA_Flags, WFLG_SIZEGADGET /*| WFLG_SIZEBRIGHT | WFLG_SIZEBBOTTOM |
//            WFLG_DRAGBAR*/ | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_ACTIVATE |
//            WFLG_SUPER_BITMAP | WFLG_GIMMEZEROZERO /*| WFLG_NOCAREREFRESH*/
//            | WFLG_SIMPLE_REFRESH
//            ,
//     //   WA_Gadgets, &(SideGad),
//        WA_Title,(ULONG) "Mame 0.106 Krb ", /* take title from version string */
//        WA_PubScreen, (ULONG)pWbScreen,
//        WA_SuperBitMap, (ULONG)_sWbWinSBitmap,
//        TAG_DONE
//        );
//    } // end if sbm ok
//    UnlockPubScreen(NULL,pWbScreen);

//    if( _pWbWindow == NULL ) return;
//    _pUserPort = _pWbWindow->UserPort;
//}
//void Display_Intuition_Window::close()
//{
//    if(_pWbWindow) CloseWindow(_pWbWindow);
//    if(_sWbWinSBitmap) FreeBitMap(_sWbWinSBitmap);
//    _pWbWindow = NULL;
//    _pUserPort = NULL;
//}


//// - - - -

//Display_CGX_Paletted::Display_CGX_Paletted(const _osd_create_params *params, ULONG forcedModeID)
//    : Display_Intuition(params),_needFirstRemap(1)
//{
//    printf("Display_CGX_Paletted()\n");

//    if(!CyberGfxBase) return;
//    int width = params->width;
//    int height = params->height;

//    int screenDepth = (params->colors<=256)?8:16; // more would be Display_CGX_TrueColor.

//    _ScreenModeId = forcedModeID;
//    if(_ScreenModeId == INVALID_ID)
//    {
//         struct TagItem cgxtags[]={
//                CYBRBIDTG_NominalWidth,width,
//                CYBRBIDTG_NominalHeight,height,
//                CYBRBIDTG_Depth,screenDepth,
//                TAG_DONE,0 };
//             printf("bef BestCModeIDTagList()\n");

//        _ScreenModeId = BestCModeIDTagList(cgxtags);
//           printf("aft BestCModeIDTagList()\n");
//               fflush(stdout);
//    }
//    if(_ScreenModeId == INVALID_ID)
//    {
//        logerror("Can't find cyber screen mode for w%d h%d d%d ",width,height,screenDepth);
//        return;
//    }
//    _fullscreenWidth = GetCyberIDAttr( CYBRIDATTR_WIDTH, _ScreenModeId );
//    _fullscreenHeight = GetCyberIDAttr( CYBRIDATTR_HEIGHT, _ScreenModeId );
//    _fullscreenPixelFmt = GetCyberIDAttr( CYBRIDATTR_PIXFMT, _ScreenModeId );
//    _pixelbytes = GetCyberIDAttr( CYBRIDATTR_BPPIX, _ScreenModeId );
//    if(_pixelbytes==3) _pixelbytes=4;

//    //_clut16.resize(65536); // exact alloc
//    // actually always work this way it seems:
//#ifdef PALFULL
//    _clut16.reserve(65536);
//    _clut16.resize(65536); // exact alloc
//    // initialize the palette to a fixed 5-5-5 mapping
//	for (int r = 0; r < 32; r++)
//		for (int g = 0; g < 32; g++)
//			for (int b = 0; b < 32; b++)
//			{
//				int idx = ((r << 10) | (g << 5) | b);

//                // according to pixfmt:
//                // RGB16PC
//                USHORT d = (((USHORT)r<<11)&0xf800)
//                            |(((USHORT)g<<6)&0x07c0)|(((USHORT)g<<1)&0x0020) // +1 extend bit.

//                            |((USHORT)b&0x001f);
//				_clut16[idx] = ((d>>8)&0x00ff)|((d<<8)&0xff00); // win_color16(rr, gg, bb) * 0x10001;
//				//palette_32bit_lookup[idx] = win_color32(rr, gg, bb);
//			}
//#else
//// no, because we dont't know if screen of window yet.
////    // just reserve alloc, better having allocs at init.
////    if(_pixelbytes == 2) {
////        _clut16.reserve(params->colors);
////    } else  if(_pixelbytes == 3 || _pixelbytes == 4) {
////        _clut32.reserve(params->colors);
////    }

//#endif
//    printf(" ** gw:%d gh:%d final res %d %d\n",width,height,_fullscreenWidth,_fullscreenHeight);
//   // _fullscreenPixelMode = GetCyberIDAttr( CYBRIDATTR_PIXFMT, _ScreenModeId );


////	int width, height;			/* width and height */
////	int aspect_x, aspect_y;		/* aspect ratio X:Y */
////	int depth;					/* depth, either 16(palette), 15(RGB) or 32(RGB) */
////	int colors;					/* colors in the palette (including UI) */
////	float fps;					/* frame rate */
////	int video_attributes;		/* video flags from driver */


//}
//Display_CGX_Paletted::~Display_CGX_Paletted()
//{

//}
//void Display_CGX_Paletted::updatePaletteRemap(_mame_display *display)
//{
//	// loop over dirty colors in batches of 32
//#ifdef OPTPAL
// int i;
//	for (i = 0; i < display->game_palette_entries; i += 32)
//	{
//		UINT32 dirtyflags = _needFirstRemap ? ~0 : display->game_palette_dirty[i>>5];
//		if (dirtyflags)
//		{
//			display->game_palette_dirty[i>>5] = 0;
//            const rgb_t *gpal = display->game_palette + i ;
//            int ji=i,je=i+32;
//            if(je>display->game_palette_entries) je = display->game_palette_entries;

//            switch(_pixelFmt)
//            {
//            // - - - - -15b cases
////             case PIXFMT_RGB15:
////                for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c>>9)&0x7c00)|((c>>6)&0x03e0)|((c>>3)&0x001f); }
////                break;
////             case PIXFMT_BGR15:
////                for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c<<7)&0x7c00)|((c>>6)&0x03e0)|((c>>19)&0x001f); }
////                break;
////             case PIXFMT_RGB15PC:
////                for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d = ((c>>9)&0x7c00)|((c>>6)&0x03e0)|((c>>3)&0x001f);
////                    *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
////                }
////                break;
////             case PIXFMT_BGR15PC:
////                for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d = ((c<<7)&0x7c00)|((c>>6)&0x03e0)|((c>>19)&0x001f);
////                    *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
////                }
////                break;
//             // - -- - - - 16b cases
//             case PIXFMT_RGB16:
////                for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c>>8)&0xf800)|((c>>5)&0x07e0)|((c>>3)&0x001f); }
////                break;
//             case PIXFMT_BGR16:
////                for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c<<8)&0xf800)|((c>>5)&0x07e0)|((c>>19)&0x001f); }
////                break;
//             case PIXFMT_RGB16PC:          //  *p++ = d;
////                for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d = ((c>>8)&0xf800)|((c>>5)&0x07e0)|((c>>3)&0x001f);
////        //           USHORT d = ((c>>9)&0xf800);
////        //            *pb++ = (UBYTE)d; *pb++ = (UBYTE)(d>>8);
////                }
//                for (;(ji < je) ; ji++, dirtyflags >>= 1)
//                    if (dirtyflags & 1)
//                    {
//                        ULONG c = *gpal++; USHORT d = ((c>>8)&0xf800)|(((USHORT)c>>5)&0x07e0)|(((USHORT)c>>3)&0x001f);
//                       _clut16[ji]= ((d>>8)&0x00ff)|((d<<8)&0xff00);
//                    }
//                break;
//             case PIXFMT_BGR16PC:
////                for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d =  ((c<<8)&0xf800)|((c>>5)&0x07e0)|((c>>19)&0x001f);
////                    *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
////                }
//                break;
////             // - - -24b cases, also use 32bit source
////             case PIXFMT_RGB24:
////             case PIXFMT_BGR24:
////             // - - -32b cases
////             case PIXFMT_ARGB32:
////                // this is the id one, no need for table, direct palette use.
////                break;
////             case PIXFMT_BGRA32:
////             case PIXFMT_RGBA32:
////                break;
////             case PIXFMT_LUT8: // no sense, should just use RGB32 os palette, do not select Display_CGX_Paletted
//            default:
//                break;
//            }

//		} // if batch dirty
//	} // loop per 32 batch
////#endif
//#else
//    if(_needFirstRemap)
//    {
//        if(_currentPixBytes == 2) {
//            _clut32.clear();
//            _clut16.resize(display->game_palette_entries);

//         }
//        if(_pixelbytes == 3 || _pixelbytes ==4 )
//        {
//            _clut16.clear();
//            _clut32.resize(display->game_palette_entries);
//        }
//    }

//    if(_needFirstRemap && _clut16.size()<display->game_palette_entries) _clut16.resize(display->game_palette_entries);
//    printf("remap: pixfmt:%d\n",_currentPixFmt);
//    const rgb_t *gpal = display->game_palette;

//    USHORT *p= _clut16.data();
//    ULONG *p32= _clut32.data();
//    UBYTE *pb=(UBYTE *)_clut16.data();
//    int i=0;
//    const ULONG nbc= display->game_palette_entries;
//    switch(_currentPixFmt)
//    {
//    // - - - - -15b cases
//     case PIXFMT_RGB15:
//        for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c>>9)&0x7c00)|((c>>6)&0x03e0)|((c>>3)&0x001f); }
//        break;
//     case PIXFMT_BGR15:
//        for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c<<7)&0x7c00)|((c>>6)&0x03e0)|((c>>19)&0x001f); }
//        break;
//     case PIXFMT_RGB15PC:
//        for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d = ((c>>9)&0x7c00)|((c>>6)&0x03e0)|((c>>3)&0x001f);
//            *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
//        }
//        break;
//     case PIXFMT_BGR15PC:
//        for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d = ((c<<7)&0x7c00)|((c>>6)&0x03e0)|((c>>19)&0x001f);
//            *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
//        }
//        break;
//     // - -- - - - 16b cases
//     case PIXFMT_RGB16:
//        for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c>>8)&0xf800)|((c>>5)&0x07e0)|((c>>3)&0x001f); }
//        break;
//     case PIXFMT_BGR16:
//        for(;i<nbc;i++) { ULONG c = *gpal++; *p++ = ((c<<8)&0xf800)|((c>>5)&0x07e0)|((c>>19)&0x001f); }
//        break;
//     case PIXFMT_RGB16PC:          //  *p++ = d;
//        for(;i<nbc;i++) {
//            ULONG c = *gpal++; USHORT d = ((c>>8)&0xf800)|(((USHORT)c>>5)&0x07e0)|(((USHORT)c>>3)&0x001f);
//          // _clut16[ji]= ((d>>8)&0x00ff)|((d<<8)&0xff00);

////        ULONG c = *gpal++; USHORT d = ((c>>8)&0xf800)|((c>>5)&0x07e0)|((c>>3)&0x001f);
////           USHORT d = ((c>>9)&0xf800);
//            *p++ = ((d>>8)&0x00ff)|((d<<8)&0xff00);
//        }
//        break;
//     case PIXFMT_BGR16PC:
//        for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d =  ((c<<8)&0xf800)|((c>>5)&0x07e0)|((c>>19)&0x001f);
//            *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
//        }
//        break;
//     // - - -24b cases, also use 32bit source
//     case PIXFMT_RGB24:
//     case PIXFMT_BGR24:
//     // - - -32b cases
//     case PIXFMT_ARGB32:
//        // this is the id one, no need for table, direct palette use.
//        break;
//     case PIXFMT_BGRA32:
//        for(;i<nbc;i++) { ULONG c = *gpal++; ULONG d = ((c>>16)&0x0000ff00)|((c<<8)&0x00ff0000)|((c<<24)&0xff000000);
//            *p32++= d;
//        }
//        break;
//     case PIXFMT_RGBA32:
//        break;
//     case PIXFMT_LUT8: // no sense, should just use RGB32 os palette, do not select Display_CGX_Paletted
//    default:
//        break;
//    }
//#endif
//	// reset the invalidate flag
//	_needFirstRemap = 0;
//}

//void Display_CGX_Paletted::drawRastPort(RastPort *pRPort,_mame_display *pmame_display, int dx, int dy)
//{
//    mame_bitmap *bitmap = pmame_display->game_bitmap;
//    directDrawScreen ddscreen;
//    int width,height,depth,bpr;
//    int prevPixFmt = _currentPixFmt;
//    if()
//    // - - update palette before bm lock
//    if(prevPixFmt != _currentPixFmt) _needFirstRemap=1;
//    if((pmame_display->changed_flags & GAME_PALETTE_CHANGED) !=0 || _needFirstRemap)
//    {
//        printf("*** DO palette remap:%d\n",counter);
//        counter=0;
//        updatePaletteRemap(pmame_display);
//    }

//    APTR hdl = LockBitMapTags(pRPort->BitMap,
//                              LBMI_WIDTH,(ULONG)&width,
//                              LBMI_HEIGHT,(ULONG)&height,
//                              LBMI_DEPTH,(ULONG)&depth,
//                              LBMI_PIXFMT,(ULONG)&_currentPixFmt,
//                              LBMI_BYTESPERPIX,(ULONG)&_currentPixBytes,
//                              LBMI_BYTESPERROW,(ULONG)&ddscreen._bpr,
//                              LBMI_BASEADDRESS,(ULONG)&ddscreen._base,
//                              TAG_DONE);
//    if(!hdl) return;



//    ddscreen._clipX1 = 0;//10;
//    ddscreen._clipY1 = 0; //10;
//    ddscreen._clipX2 = (WORD)width; //-10;
//    ddscreen._clipY2 = (WORD)height; //-10;

//    // +1 because goes 0,319
//    int sourcewidth = (pmame_display->game_visible_area.max_x-pmame_display->game_visible_area.min_x)+1;
//    int sourceheight =( pmame_display->game_visible_area.max_y-pmame_display->game_visible_area.min_y)+1;

//    int cenx = width-sourcewidth;
//    int ceny = height-sourceheight;
//    if(cenx<0) cenx = 0;
//    if(ceny<0) ceny = 0;
//    cenx>>=1;
//    ceny>>=1;

//    directDrawSource ddsource={bitmap->base,bitmap->rowbytes,
//        pmame_display->game_visible_area.min_x,pmame_display->game_visible_area.min_y,
//        pmame_display->game_visible_area.max_x+1,pmame_display->game_visible_area.max_y+1
//    };
//    switch(_currentPixFmt) {
//     case PIXFMT_RGB15:
//     case PIXFMT_BGR15:
//     case PIXFMT_RGB15PC:
//     case PIXFMT_BGR15PC:
//     case PIXFMT_RGB16:
//     case PIXFMT_BGR16:
//     case PIXFMT_RGB16PC:
//     case PIXFMT_BGR16PC:
//     if(_clut16.size()>0)
//        directDrawClut16(&ddscreen,&ddsource,cenx+dx,ceny+dy,_clut16.data());
//        break;
//     case PIXFMT_RGB24:
//     case PIXFMT_BGR24:
//         break;
//     case PIXFMT_ARGB32:
//     case PIXFMT_BGRA32:
//     case PIXFMT_RGBA32:
//     if(_clut32.size()>0)
//        directDrawClut32(&ddscreen,&ddsource,cenx+dx,ceny+dy,_clut32.data());
//        break;
//    default:
//        break;
//    }

//    UnLockBitMap(hdl);

//}
//void Display_CGX_Paletted::draw(_mame_display *pmame_display)
//{
//    static int counter=0;
//    counter++;
//    /* pixfmt constated:
//     *  UAE picasso : WB PIXFMT_BGRA32 , asked 16b: PIXFMT_RGB16PC
//    */

//    mame_bitmap *bitmap = pmame_display->game_bitmap;
//    if(_pWbWindow && _pWbWindow->RPort->BitMap)
//    {
//        drawRastPort(_pWbWindow->RPort,pmame_display,_pWbWindow->BorderLeft,_pWbWindow->BorderTop);
//    }
//    if(_pScreen)
//    {
//        drawRastPort(&_pScreen->RastPort,pmame_display,0,0);
//    }
//}

//// - - - -

//Display_CGX_TrueColor::Display_CGX_TrueColor(const _osd_create_params *params, ULONG forcedModeID)
//    : Display_Intuition(params)
//{

//    if(!CyberGfxBase) return;
//    int width = params->width;
//    int height = params->height;

////	int width, height;			/* width and height */
////	int aspect_x, aspect_y;		/* aspect ratio X:Y */
////	int depth;					/* depth, either 16(palette), 15(RGB) or 32(RGB) */
////	int colors;					/* colors in the palette (including UI) */
////	float fps;					/* frame rate */
////	int video_attributes;		/* video flags from driver */

//    int depth = (params->depth==32)?32:16; //15 means 16 for CGX.

//    _ScreenModeId = forcedModeID;
//    if(_ScreenModeId == INVALID_ID)
//	{
// 		 struct TagItem cgxtags[]={
//			CYBRBIDTG_NominalWidth,width,
//			CYBRBIDTG_NominalHeight,height,
//			CYBRBIDTG_Depth,depth,
//			TAG_DONE,0 };
//        _ScreenModeId = BestCModeIDTagList(cgxtags);
//    }
//    if(_ScreenModeId == INVALID_ID)
//    {
//        logerror("Can't find cyber screen mode for w%d h%d d%d ",width,height,depth);
//        return;
//    }

//}
//Display_CGX_TrueColor::~Display_CGX_TrueColor()
//{
//    if(_pWbWindow)
//    {

//    }

//}
//void Display_CGX_TrueColor::draw(_mame_display *pmame_display)
//{
//    mame_bitmap *bitmap = pmame_display->game_bitmap;
//    // should be better with P96
//}





// ==================== new impl

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
    void directDrawClut32(register directDrawScreen *screen __asm("a0"),
                    register directDrawSource *source __asm("a1"),
                    register LONG x1 __asm("d0"),
                    register LONG y1 __asm("d1"),
                    register ULONG *lut __asm("a2") // actually UWORD* or anywhat.
                );
}


Paletted_CGX::Paletted_CGX(const _osd_create_params *params, int screenPixFmt, int bytesPerPix)
    : _needFirstRemap(1), _pixFmt(screenPixFmt),_bytesPerPix(bytesPerPix)
{
    printf(" **** Paletted_CGX() pixfmt:%d bpp:%d  nbc:%d\n",screenPixFmt,bytesPerPix,params->colors);
    switch(bytesPerPix){
        case 1: _clut8.reserve(params->colors); break;
        case 2: _clut16.reserve(params->colors); break;
        case 3: case 4: _clut32.reserve(params->colors); break;
    }
}
Paletted_CGX::~Paletted_CGX(){}

void Paletted_CGX::updatePaletteRemap(_mame_display *display)
{
    const rgb_t *gpal = display->game_palette;
    const int nbc = display->game_palette_entries;
//    printf("nbc:%d _bytesPerPix:%d\n",nbc,_bytesPerPix);
    if(_needFirstRemap)
    {
        switch(_bytesPerPix){
            case 1: if(_clut8.size()<nbc) _clut8.resize(nbc); break;
            case 2: if(_clut16.size()<nbc) _clut16.resize(nbc); break;
            case 3: case 4: if(_clut32.size()<nbc) _clut32.resize(nbc);
             break;
        }
    }

    USHORT *p= _clut16.data();
    ULONG *p32= _clut32.data();
    UBYTE *pb=(UBYTE *)_clut16.data();
    int i=0;

    switch(_pixFmt)
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
        for(;i<nbc;i++) {
            ULONG c = *gpal++; USHORT d = (((USHORT)(c>>8))&0xf800)|(((USHORT)c>>5)&0x07e0)|(((USHORT)c>>3)&0x001f);
            *p++ = ((d>>8)&0x00ff)|((d<<8)&0xff00);
        }
        break;
     case PIXFMT_BGR16PC:
        for(;i<nbc;i++) { ULONG c = *gpal++; USHORT d =  ((c<<8)&0xf800)|((c>>5)&0x07e0)|((c>>19)&0x001f);
            *pb++ = (UBYTE)d; d>>=8;  *pb++ = (UBYTE)d;
        }
        break;
    //   - - -24b cases, also use 32bit source
     case PIXFMT_RGB24:
     case PIXFMT_BGR24:
     // - - -32b cases
     case PIXFMT_ARGB32:
        // this is the id one, no need for table, direct palette use.
        break;
     case PIXFMT_BGRA32:
        for(;i<nbc;i++) { ULONG c = *gpal++; ULONG d = ((c>>8)&0x0000ff00)|((c<<8)&0x00ff0000)|((c<<24)&0xff000000);
            *p32++= d;
        }
        break;
     case PIXFMT_RGBA32:
        break;
     case PIXFMT_LUT8: // no sense, should just use RGB32 os palette, do not select Display_CGX_Paletted
    default:
        break;
    }
    _needFirstRemap = 0;
}



IntuitionDrawable::IntuitionDrawable()
: _PixelFmt(0),_PixelBytes(0),_dx(0),_dy(0)
{
}
IntuitionDrawable::~IntuitionDrawable()
{
}

// would draw LUT screens or truecolor, ...
void IntuitionDrawable::drawRastPort_CGX(_mame_display *display,Paletted_CGX *pRemap)
{
    if(!CyberGfxBase) return;
    RastPort *pRPort = rastPort();
    if(!pRPort) return;
    mame_bitmap *bitmap = display->game_bitmap;

    directDrawScreen ddscreen;
    int width,height,depth,pixfmt,pixbytes,bpr;

    APTR hdl = LockBitMapTags(pRPort->BitMap,
                              LBMI_WIDTH,(ULONG)&width,
                              LBMI_HEIGHT,(ULONG)&height,
                              LBMI_DEPTH,(ULONG)&depth,
                              LBMI_PIXFMT,(ULONG)&pixfmt,
                              LBMI_BYTESPERPIX,(ULONG)&pixfmt,
                              LBMI_BYTESPERROW,(ULONG)&ddscreen._bpr,
                              LBMI_BASEADDRESS,(ULONG)&ddscreen._base,
                              TAG_DONE);
    if(!hdl) return;

    ddscreen._clipX1 = 0;//10;
    ddscreen._clipY1 = 0; //10;
    ddscreen._clipX2 = (WORD)width; //-10;
    ddscreen._clipY2 = (WORD)height; //-10;

    // +1 because goes 0,319
    int sourcewidth = (display->game_visible_area.max_x - display->game_visible_area.min_x)+1;
    int sourceheight =( display->game_visible_area.max_y - display->game_visible_area.min_y)+1;

    int cenx = width-sourcewidth;
    int ceny = height-sourceheight;
    if(cenx<0) cenx = 0;
    if(ceny<0) ceny = 0;
    cenx>>=1;
    ceny>>=1;

    directDrawSource ddsource={bitmap->base,bitmap->rowbytes,
        display->game_visible_area.min_x,display->game_visible_area.min_y,
        display->game_visible_area.max_x+1,display->game_visible_area.max_y+1
    };
    if(pRemap)
    {
        switch(_PixelFmt) {
         case PIXFMT_RGB15:case PIXFMT_BGR15:case PIXFMT_RGB15PC:case PIXFMT_BGR15PC:
         case PIXFMT_RGB16:case PIXFMT_BGR16:case PIXFMT_RGB16PC:case PIXFMT_BGR16PC:
         if(pRemap->_clut16.size()>0)
            directDrawClut16(&ddscreen,&ddsource,cenx+_dx,ceny+_dy,pRemap->_clut16.data());
            break;
         case PIXFMT_RGB24:case PIXFMT_BGR24:
            //TODO
             break;
         case PIXFMT_ARGB32:case PIXFMT_BGRA32:case PIXFMT_RGBA32:
         if(pRemap->_clut32.size()>0)
            directDrawClut32(&ddscreen,&ddsource,cenx+_dx,ceny+_dy,pRemap->_clut32.data());
            break;
        default:
            //LUT8, aga:todo
            break;
        }
    } else
    {
        //Truecolor (todo)
    }

    UnLockBitMap(hdl);
}
// =========================== new impl

Intuition_Screen::Intuition_Screen(const _osd_create_params *params,ULONG forcedModeId) : IntuitionDrawable()
    , _pScreen(NULL)
    , _pScreenWindow(NULL)
    , _ScreenModeId(forcedModeId)
    , _fullscreenWidth(0)
    , _fullscreenHeight(0)
    , _pMouseRaster(NULL)
{
    // may get mode from Cybergraphics...
    if(CyberGfxBase)
    {
        int width = params->width;
        int height = params->height;

        int screenDepth = (params->colors<=256)?8:16; // more would be Display_CGX_TrueColor.

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
        _PixelFmt = GetCyberIDAttr( CYBRIDATTR_PIXFMT, _ScreenModeId );
        _PixelBytes = GetCyberIDAttr( CYBRIDATTR_BPPIX, _ScreenModeId );

    } // end if CGX available

}
Intuition_Screen::~Intuition_Screen()
{
    close();
}
void Intuition_Screen::open()
{
    if(_pScreenWindow) return; // already open.
    if(_ScreenModeId == INVALID_ID) return; // set by inherited class.

    // note: all this is regular OS intuition, no CGX
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
        close();
        return ;
	}
	// ------- set invisible mouse pointer:
	_pMouseRaster =  AllocRaster(8 ,8) ;
	if(_pMouseRaster)
	{
        SetPointer( _pScreenWindow ,(UWORD *) _pMouseRaster, 0,1,0,0);
    }

}
void Intuition_Screen::close()
{
    if(_pScreenWindow) CloseWindow(_pScreenWindow);
    if(_pScreen) CloseScreen(_pScreen);
    if(_pMouseRaster) FreeRaster( (PLANEPTR) _pMouseRaster ,8,8);

    _pScreenWindow = NULL;
    _pScreen = NULL;
    _pMouseRaster = NULL;
}
MsgPort *Intuition_Screen::userPort()
{
    if(!_pScreenWindow) return NULL;
    return _pScreenWindow->UserPort;
}
RastPort *Intuition_Screen::rastPort()
{
    if(!_pScreen) return NULL;
    return &_pScreen->RastPort;
}

Intuition_Window::Intuition_Window(const _osd_create_params *params) : IntuitionDrawable()
    , _pWbWindow(NULL)
    , _sWbWinSBitmap(NULL)
    , _machineWidth(params->width),_machineHeight(params->height)
{}
Intuition_Window::~Intuition_Window()
{
    close();
}
void Intuition_Window::open()
{
    if(_pWbWindow) return;

    Screen *pWbScreen;
    if (!(pWbScreen = LockPubScreen(NULL))) return;

    int xcen = (pWbScreen->Width - _machineWidth);
    int ycen = (pWbScreen->Height - _machineHeight);
    if(xcen<0) xcen=0;
    xcen>>=1;
    if(ycen<0) ycen=0;
    ycen>>=1;
    printf("openWindow:_machineWidth:%d _machineHeight:%d xcen:%d ycen:%d \n",_machineWidth,_machineHeight,xcen,ycen);

// struct BitMap * __stdargs AllocBitMap( ULONG sizex, ULONG sizey, ULONG depth, ULONG flags, CONST struct BitMap *friend_bitmap );

    _sWbWinSBitmap = AllocBitMap(_machineWidth,_machineHeight,
            pWbScreen->RastPort.BitMap->Depth,BMF_CLEAR|BMF_DISPLAYABLE,pWbScreen->RastPort.BitMap);
    if(_sWbWinSBitmap) {

        _pWbWindow = (Window *)OpenWindowTags(NULL,
        WA_Left,xcen,
        WA_Top,ycen,
     //   WA_Width, _machineWidth,
     //   WA_Height, _machineHeight,
        WA_InnerWidth, _machineWidth,
        WA_InnerHeight, _machineHeight,
    //    WA_MaxWidth,  WIDTH_SUPER,
    //    WA_MaxHeight, HEIGHT_SUPER,
        WA_IDCMP,/* IDCMP_GADGETUP | IDCMP_GADGETDOWN |*/IDCMP_MOUSEBUTTONS |  IDCMP_RAWKEY /*|
            IDCMP_NEWSIZE*/ /*| IDCMP_INTUITICKS*/ | IDCMP_CLOSEWINDOW,

        WA_Flags, WFLG_SIZEGADGET /*| WFLG_SIZEBRIGHT | WFLG_SIZEBBOTTOM |
            WFLG_DRAGBAR*/ | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_ACTIVATE |
            WFLG_SUPER_BITMAP | WFLG_GIMMEZEROZERO /*| WFLG_NOCAREREFRESH*/
            | WFLG_SIMPLE_REFRESH
            ,
     //   WA_Gadgets, &(SideGad),
        WA_Title,(ULONG) "Mame 0.106 Krb ", /* take title from version string */
        WA_PubScreen, (ULONG)pWbScreen,
        WA_SuperBitMap, (ULONG)_sWbWinSBitmap,
        TAG_DONE
        );
    } // end if sbm ok
    UnlockPubScreen(NULL,pWbScreen);

    if( _pWbWindow == NULL ) return;

    _dx = _pWbWindow->BorderLeft;
    _dy = _pWbWindow->BorderTop;
    // need pixel format at this level
    if(CyberGfxBase)
    {
        if(_sWbWinSBitmap && GetCyberMapAttr(_sWbWinSBitmap,CYBRMATTR_ISCYBERGFX))
        {
            _PixelFmt = GetCyberMapAttr(_sWbWinSBitmap,CYBRMATTR_PIXFMT);
            _PixelBytes = GetCyberMapAttr(_sWbWinSBitmap,CYBRMATTR_BPPIX);
        }
    }

}
void Intuition_Window::close()
{
    if(_pWbWindow) CloseWindow(_pWbWindow);
    if(_sWbWinSBitmap) FreeBitMap(_sWbWinSBitmap);
    _pWbWindow = NULL;
}
MsgPort *Intuition_Window::userPort()
{
    if(!_pWbWindow) return NULL;
    return _pWbWindow->UserPort;
}
RastPort *Intuition_Window::rastPort()
{
    if(!_pWbWindow) return NULL;
    return _pWbWindow->RPort;
}

Display_CGX::Display_CGX()
: MameDisplay()
, _drawable(NULL)
, _remap(NULL)
{

}
Display_CGX::~Display_CGX()
{
    if(_drawable) delete _drawable;
    if(_remap) delete _remap;
}
void Display_CGX::open(const _osd_create_params *params,int window, ULONG forcedModeID)
{
   if(!CyberGfxBase) return;
   if(_drawable) return;

    if(window)
    {
        _drawable = new Intuition_Window(params);
    } else
    {
        _drawable = new Intuition_Screen(params,forcedModeID);
    }

    if(!_drawable) return;
    _drawable->open();

    if((params->video_attributes & VIDEO_RGB_DIRECT)==0 &&
        params->colors>0)
    {
        _remap = new Paletted_CGX(params,_drawable->pixelFmt(),_drawable->pixelBytes());
    }

}
void Display_CGX::close()
{
    if(!_drawable) return;
    _drawable->close();
}
int Display_CGX::good()
{
    if(_drawable && _drawable->userPort()) return 1;
    return 0;
}
void Display_CGX::draw(_mame_display *display)
{
    if(!_drawable) return;

    // - - update palette if exist and is needed.
    if(_remap && ((display->changed_flags & GAME_PALETTE_CHANGED) !=0 || _remap->needRemap()))
    {
        _remap->updatePaletteRemap(display);
    }

    if(CyberGfxBase) _drawable->drawRastPort_CGX(display,_remap);
}
MsgPort *Display_CGX::userPort()
{
    if(!_drawable) return NULL;
    return _drawable->userPort();
}












