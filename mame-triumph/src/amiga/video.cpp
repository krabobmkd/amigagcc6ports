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
#include "cgxhooks_re.h"

#include <inline/exec.h>
#include <inline/dos.h>
#include <inline/graphics.h>
#include <inline/cybergraphics.h>
#include <inline/intuition.h>
#include <inline/gadtools.h>
#include <inline/asl.h>
#include <inline/utility.h>

#define TIMER_BASE_NAME video->TimerBase
#include <inline/timer.h>

#ifdef POWERUP
#include <exec/execbase.h>
#include <powerup/ppclib/memory.h>
#include <inline/ppc.h>
#endif

#include <macros.h>
}

#include "video.h"

#define ABS(a) ((a>=0)?(a):-(a))
typedef ULONG (*RE_HOOKFUNC)();

struct BackFillMsg
{
  struct Layer    *Layer;
  struct Rectangle  Bounds;
  LONG        OffsetX;
  LONG        OffsetY;
};

struct BMHD
{
  UWORD Width;
  UWORD Height;
  WORD  Left;
  WORD  Top;
  UBYTE Planes;
  UBYTE Masking;
  UBYTE Compression;
  UBYTE pad;
  UWORD TransparentColor;
  UBYTE XAspect;
  UBYTE YAspect;
  WORD  PageWidth;
  WORD  PageHeight;
};

extern struct ExecBase  *SysBase;
extern struct Library   *DOSBase;
extern struct GfxBase   *GfxBase;
extern struct Library   *CyberGfxBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Library   *AslBase;
extern struct Library   *GadToolsBase;
extern struct Library   *UtilityBase;

#ifdef POWERUP
extern struct Library   *PPCLibBase;
#endif

verror_t VError;
extern "C" {
void ASM RemapPixels(UBYTE * REG(a0), UBYTE * REG(a1), UBYTE *REG(a2), LONG REG(d0), LONG REG(d1), LONG REG(d2), LONG REG(d3));
void ASM c2p(UBYTE * REG(a0), struct BitMap * REG(a1), WORD REG(d0), WORD REG(d1), WORD REG(d2), WORD REG(d3), LONG REG(d4));
}

#ifndef POWERUP
static inline APTR memAlloc(ULONG size)
{
  return(AllocVec(size, MEMF_PUBLIC|MEMF_CLEAR));
}

static inline void memFree(APTR mem)
{
  FreeVec(mem);
}
#else
static inline APTR memAlloc(ULONG size)
{
  return(PPCAllocVec(size, MEMF_PUBLIC|MEMF_CLEAR));
}

static inline void memFree(APTR mem)
{
  PPCFreeVec(mem);
}
#endif

ULONG ASM VBackFill(struct Hook *hook REG(a0), struct RastPort *rp REG(a2), struct BackFillMsg *bfm REG(a1))
{
  struct Layer  *l;
  LONG          old_pen, new_pen;

  l         = rp->Layer;
  rp->Layer = NULL;
  old_pen   = GetAPen(rp);
  new_pen   = ((struct Video *) hook->h_Data)->BackFillPen;

  if(new_pen != -1)
    SetAPen(rp, new_pen);
  else
    SetAPen(rp, 0);

  RectFill(rp, bfm->Bounds.MinX, bfm->Bounds.MinY, bfm->Bounds.MaxX, bfm->Bounds.MaxY);

  SetAPen(rp, old_pen);

  rp->Layer = l;

  return(0);
}

ULONG VGetBestMode(LONG width, LONG height, LONG depth)
{
  struct List          *mode_list;
  struct CyberModeNode *mode_node;
  ULONG                mode_id;
  ULONG                fallback_id;
  LONG                 mode_error;
  LONG                 fallback_error;
  LONG                 error;
  
  mode_id = INVALID_ID;

  if(CyberGfxBase)
  {
    mode_list = AllocCModeListTagList(NULL);
    
    if(mode_list)
    {
      mode_node = (struct CyberModeNode *) mode_list->lh_Head;
      
      mode_error     = 0xfffffff;
      fallback_id    = INVALID_ID;
      fallback_error = 0xfffffff;
      
      while(mode_node->Node.ln_Succ)
      {
        if(((depth <= 8) && (mode_node->Depth == 8))
        || ((depth > 8) && (mode_node->Depth == depth)))
        {
          error = ABS(mode_node->Width - width) + ABS(mode_node->Height - height);

          if((mode_node->Width < width) || (mode_node->Height < height))
          {
            if(error < fallback_error)
            {
              fallback_id   = mode_node->DisplayID;
              fallback_error  = error;
            }
          }
          else
          {
            if(error < mode_error)
            {
              mode_id   = mode_node->DisplayID;
              mode_error  = error;
            }
          }
        }
      
        mode_node = (struct CyberModeNode *) mode_node->Node.ln_Succ;
      }
      
      if(mode_id == INVALID_ID)
        mode_id = fallback_id;

      FreeCModeList(mode_list);
    }
  }

  if((mode_id == INVALID_ID) && (depth <= 8))
    mode_id = BestModeID(BIDTAG_Depth,         depth,
                         BIDTAG_NominalWidth,  width,
                         BIDTAG_NominalHeight, height,
                         TAG_END);
  
  return(mode_id);
}

struct Video *AllocVideo(Tag tags,...)
{
  struct Video               *video;
  struct TagItem             *tag, *taglist;
  struct ScreenModeRequester *screen_req;
  struct Rectangle           rect;
  struct Window              *win;

  STRPTR title;
  LONG   i, w, h, depth, rb, gb, bb;
  ULONG  max_colors;
  ULONG  mode_id;
  BOOL   use_screen, use_screen_req, no_refresh;

    printf("AllocVideo in\n");

  if((video = (struct Video *)AllocVec(sizeof(struct Video), MEMF_PUBLIC|MEMF_CLEAR)))
  {
    /* Setup defaults. */

    VError = None;

    mode_id        = INVALID_ID;
    depth          = 0;
    win            = NULL;
    title          = NULL;
    use_screen     = FALSE;
    use_screen_req = FALSE;
    no_refresh     = FALSE;
    max_colors     = 256;

    video->Buffers = 1;

    video->VectorPen   = -1;
    video->BackFillPen = -1;

    video->BackFillHook.h_Entry = (RE_HOOKFUNC) VBackFill;
    video->BackFillHook.h_Data  = (APTR) video;

    video->MaxFrameSkip = 4;

    taglist = (struct TagItem *) &tags;

    while((tag = NextTagItem(&taglist)))
    {
      switch(tag->ti_Tag)
      {
        case VA_UseScreen:
          use_screen = tag->ti_Data;
          break;

        case VA_UseScreenReq:
          use_screen_req = tag->ti_Data;
          break;

        case VA_Width:
          video->Width = tag->ti_Data;
          break;

        case VA_Height:
          video->Height = tag->ti_Data;
          break;

        case VA_Depth:
          depth = tag->ti_Data;
          break;

        case VA_ModeID:
          mode_id = tag->ti_Data;
          break;

        case VA_Buffers:
          video->Buffers  = tag->ti_Data;

          if(video->Buffers > 3)
            video->Buffers  = 3;
          else if(!video->Buffers)
            video->Buffers  = 1;
          break;

        case VA_LessFlicker:
          video->LessFlicker  = tag->ti_Data;
          break;

        case VA_Title:
          title = (STRPTR) tag->ti_Data;
          break;

        case VA_Menu:
          video->Menu = (struct Menu *) tag->ti_Data;
          break;

        case VA_NoRefresh:
          no_refresh  = tag->ti_Data;
          break;

        case VA_FPS:
          video->FPS = tag->ti_Data;
          break;

        case VA_MaxColors:
          max_colors = tag->ti_Data;
          break;

        case VA_AutoFrameSkip:
          video->AutoFrameSkip = tag->ti_Data;
          break;

        case VA_MaxFrameSkip:
          video->MaxFrameSkip = tag->ti_Data;
          break;
      }
    }

    printf("AllocVideo after tags\n");


    if(use_screen || use_screen_req)
    {
      if(ModeNotAvailable(mode_id))
      {
        mode_id = INVALID_ID;

        if(depth == 0)
          for(; (depth < 32) && (1 << depth < max_colors); depth++);

        if(!use_screen_req)
          mode_id = VGetBestMode(video->Width, video->Height, depth);

        if(mode_id == INVALID_ID)
        {
          screen_req = (struct ScreenModeRequester *) AllocAslRequest(ASL_ScreenModeRequest, NULL);

          if(screen_req)
          {
            if(AslRequestTags(screen_req, ASLSM_MinWidth,            video->Width,
                                          ASLSM_MinHeight,           video->Height,
                                          ASLSM_InitialDisplayDepth, depth,
                                          ASLSM_DoDepth,             TRUE,
                                          TAG_END))

            mode_id = screen_req->sm_DisplayID;
            depth   = screen_req->sm_DisplayDepth;

            FreeAslRequest(screen_req);
          }
        }

      }

      if(mode_id != INVALID_ID)
      {
        if(QueryOverscan(mode_id, &rect, OSCAN_TEXT))
        {
          if(CyberGfxBase)
            video->CyberMode = IsCyberModeID(mode_id);
          else
            video->CyberMode = FALSE;

          if(video->CyberMode && (depth < 8))
            depth = 8;

          if(GfxBase->LibNode.lib_Version >= 40)
            video->PixelMode = eWriteChunkyPixels;
          else
            video->PixelMode = eWritePixelArray;

          if(!video->CyberMode && (depth <= 8))
            video->PixelMode = eCustomC2P;

          if(video->Width < rect.MaxX - rect.MinX + 1)
            w = rect.MaxX - rect.MinX + 1;
          else
            w = video->Width;

          if(video->Height < rect.MaxY - rect.MinY + 1)
            h = rect.MaxY - rect.MinY + 1;
          else
            h = video->Height;

          if(video->PixelMode == eCustomC2P)
            w = ((w + 31) >> 5) << 5;

          if((1 << depth) < max_colors)
          {
            bb = depth / 3;
            rb = (depth - bb) / 2;
            gb = depth - rb - bb;

            video->Palette[0] = (1 << depth) << 16;

            for(i = 0; i < (1 << depth); i++)
            {
              video->Palette[3*i+1] = ((i / (1 << (gb + bb))) % (1 << rb)) << (32 - rb);
              video->Palette[3*i+2] = ((i / (1 << bb)) % (1 << gb)) << (32 - gb);
              video->Palette[3*i+3] = (i % (1 << bb)) << (32 - bb);
            }
            
            video->Palette[3*i+1] = 0;
          }

          video->Screen = OpenScreenTags(NULL, SA_DisplayID, mode_id,
                                               SA_Width,     w,
                                               SA_Height,    (video->CyberMode)
                                                             ? h * video->Buffers
                                                             : h,
                                               SA_Depth,     depth,
                                               SA_Colors32,  ((depth <= 8) && ((1 << depth) < max_colors))
                                                             ? (ULONG) video->Palette
                                                             : NULL,
                                               SA_ShowTitle, FALSE,
                                               SA_Overscan,  OSCAN_TEXT,
                                               SA_Type,      CUSTOMSCREEN,
                                               SA_Title,     (ULONG) title,
                                               TAG_END);

          if(video->Screen)
          {
            if(!video->CyberMode && (video->Buffers > 1))
            {
              video->ScreenBuffers[0]   = AllocScreenBuffer(video->Screen, NULL, SB_SCREEN_BITMAP);
              if(video->ScreenBuffers[0])
              {
                video->ScreenBuffers[1]   = AllocScreenBuffer(video->Screen, NULL, 0);
                if(video->ScreenBuffers[1])
                {
                  InitRastPort(&video->ScreenBufferRP);
                  video->ScreenBufferRP.BitMap  = video->ScreenBuffers[1]->sb_BitMap;
                  video->RastPort         = &video->ScreenBufferRP;

                  if(video->Buffers > 2)
                  {
                    video->ScreenBuffers[2]   = AllocScreenBuffer(video->Screen, NULL, 0);
                      
                    if(!video->ScreenBuffers[2])
                      video->Buffers = 2; /* fall back to double buffering. */
                  }

                  if(video->Buffers == 2)
                  {
                    video->ScreenBuffers[0]->sb_DBufInfo->dbi_UserData1                = (APTR) 1;
                    video->ScreenBuffers[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = &video->ScreenBufferPort;
                    video->ScreenBuffers[1]->sb_DBufInfo->dbi_UserData1                = (APTR) 0;
                    video->ScreenBuffers[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = &video->ScreenBufferPort;
                  }
                }
                else
                  video->Buffers  = 1;
              }
              else
                video->Buffers  = 1;
            }

            win = OpenWindowTags(NULL, WA_Width,         video->Screen->Width,
                                       WA_Height,        video->Screen->Height,
                                       WA_Left,          0,
                                       WA_Top,           0,
                                       WA_NoCareRefresh, no_refresh,
                                       WA_SimpleRefresh, TRUE,
                                       WA_Activate,      TRUE,
                                       WA_Borderless,    TRUE,
                                       WA_NewLookMenus,  TRUE,
                                       WA_AutoAdjust,    FALSE,
                                       WA_BackFill,      (ULONG) &video->BackFillHook,
                                       WA_CustomScreen,  (ULONG) video->Screen,
                                       WA_IDCMP,         video->Menu
                                                         ? IDCMP_REFRESHWINDOW|IDCMP_MENUPICK
                                                         : IDCMP_REFRESHWINDOW,
                                       TAG_END);
            
            if(win)
            {
              video->ScreenBufferPort.mp_Node.ln_Type = NT_MSGPORT;
              video->ScreenBufferPort.mp_Flags        = PA_SIGNAL;
              video->ScreenBufferPort.mp_SigBit       = win->UserPort->mp_SigBit;           
              video->ScreenBufferPort.mp_SigTask      = win->UserPort->mp_SigTask;            

              NewList(&video->ScreenBufferPort.mp_MsgList);

              video->Pointer  = AllocVec(8, MEMF_CHIP|MEMF_CLEAR);
              
              if(video->Pointer)
                SetPointer(win,(UWORD*) video->Pointer, 0, 0, 0, 0);
            }
            else
              VError = OpenWindowFailed;
          }
          else
            VError = OpenScreenFailed;
        }
      }
    }
    else
    {
        printf("window case\n");


      win = OpenWindowTags(NULL, WA_InnerWidth,    video->Width,
                                 WA_InnerHeight,   video->Height,
                                 WA_DragBar,       TRUE,
                                 WA_CloseGadget,   TRUE,
                                 WA_DepthGadget,   TRUE,
                                 WA_NoCareRefresh, no_refresh,
                                 WA_SimpleRefresh, TRUE,
                                 WA_Activate,      TRUE,
                                 WA_NewLookMenus,  TRUE,
                                 WA_AutoAdjust,    FALSE,
                                 WA_BackFill,      (ULONG) &video->BackFillHook,
                                 WA_Title,         (ULONG) title,
                                 WA_IDCMP,         video->Menu
                                                   ? IDCMP_REFRESHWINDOW|IDCMP_MENUPICK|IDCMP_CLOSEWINDOW
                                                   : IDCMP_REFRESHWINDOW|IDCMP_CLOSEWINDOW,
                                 TAG_END);

      if(win)
      {
        if(GfxBase->LibNode.lib_Version >= 40)
          video->PixelMode = eWriteChunkyPixels;
        else
          video->PixelMode = eWritePixelArray;

        if(CyberGfxBase && win)
          video->CyberMode = GetCyberMapAttr(win->RPort->BitMap, CYBRMATTR_ISCYBERGFX);
        else
          video->CyberMode = FALSE;
      }
      else
        VError = OpenWindowFailed;
    }
    printf("after win init, return shared cases\n");

    if((video->Window = win))
    {
        printf("AllocVideo intui inits1\n");

      w = video->Width;
      h = video->Height;

      if((video->Buffers > 1) && video->Screen)
        video->CurrentScreenBuffer    = 1;
      else
        video->CurrentScreenBuffer    = 0;

      video->Left   = (win->Width  + win->BorderLeft - win->BorderRight - w) >> 1;

      if((video->Buffers > 1) && video->Screen && video->CyberMode)
      {
        video->Top     = ((win->Height / video->Buffers) + win->BorderTop - win->BorderBottom - h) >> 1;
        video->OffsetY = (win->Height / video->Buffers) * video->CurrentScreenBuffer;
      }
      else
      {
        video->Top     = (win->Height  + win->BorderTop - win->BorderBottom - h) >> 1;
        video->OffsetY = 0;
      }
      printf("AllocVideo intui inits2\n");

      video->FrameBox[0].MinX = video->Left;
      video->FrameBox[0].MinY = video->Top;
      video->FrameBox[0].MaxX = video->Left + video->Width - 1;
      video->FrameBox[0].MaxY = video->Top + video->Height - 1;

      video->FrameBox[1] = video->FrameBox[0];
      video->FrameBox[2] = video->FrameBox[0];
      printf("AllocVideo intui inits3\n");

      if(video->CyberMode)
      {
        video->PixelFormat = GetCyberMapAttr(win->RPort->BitMap, CYBRMATTR_PIXFMT);
     printf("AllocVideo intui inits3a1\n");
        if(video->PixelFormat && (max_colors <= 256))
        {

/*
struct CGXHook * __saveds AllocCLUT8RemapHook(struct Screen *scr __asm("a0"),
											  ULONG *palette __asm("a1"));
*/

     printf("AllocVideo intui inits3aab WSccree,%08x\n",(int)win->WScreen);
          video->CGXHook = AllocCLUT8RemapHook(win->WScreen, NULL);
     printf("AllocVideo intui inits3a2\n");
          if(video->CGXHook)
          {
            video->PixelMode = eCGXHook;
            video->VectorPen = ObtainPen(win->WScreen->ViewPort.ColorMap, -1, 0, 0, 0, PEN_EXCLUSIVE);

            CustomRemapCLUT8RemapHook(video->CGXHook, video->PackedPalette);
          }
     printf("AllocVideo intui inits3a3\n");
        }
        else
          video->PixelMode = eBltBitMapRastPort;
      }
     printf("AllocVideo intui inits3b\n");
      if(video->Screen && ((1 << video->Screen->RastPort.BitMap->Depth) < max_colors))
      {
        video->RemapBuffer = (UBYTE*) AllocVec((((video->Width + 31) >> 5) << 5) * video->Height,
                                      MEMF_PUBLIC|MEMF_CLEAR);
      }
      printf("AllocVideo intui inits4\n");

      if(video->PixelFormat)
        video->BackFillPen = ObtainBestPenA(video->Window->WScreen->ViewPort.ColorMap, 0, 0, 0, NULL);
      else
        video->BackFillPen = -1;

      if(video->RastPort)
      {
        if(video->BackFillPen != -1)
          SetAPen(video->RastPort, video->BackFillPen);
        else
          SetAPen(video->RastPort, 0);

        RectFill(video->RastPort,
                 win->BorderLeft,                    win->BorderTop,
                 win->Width - win->BorderRight - 1,  win->Height - win->BorderBottom - 1);
      }
      else
        video->RastPort = win->RPort;
      printf("AllocVideo intui inits5\n");
      EraseRect(win->RPort,
                win->BorderLeft,                   win->BorderTop,
                win->Width - win->BorderRight - 1, win->Height - win->BorderBottom - 1);
      printf("AllocVideo intui inits6\n");
      if(GadToolsBase && video->Menu)
      {
        video->VisualInfo = GetVisualInfoA(win->WScreen, NULL);

        if(video->VisualInfo)
        {
          if(LayoutMenus(video->Menu, video->VisualInfo, GTMN_NewLookMenus, TRUE, TAG_END))
            SetMenuStrip(win, video->Menu);
        }
      }
      printf("AllocVideo intui inits7\n");
      video->TimerMsgPort.mp_Node.ln_Type = NT_MSGPORT;
      video->TimerMsgPort.mp_Flags        = PA_SIGNAL;
      video->TimerMsgPort.mp_SigBit       = video->Window->UserPort->mp_SigBit;
      video->TimerMsgPort.mp_SigTask      = video->Window->UserPort->mp_SigTask;
      NewList(&video->TimerMsgPort.mp_MsgList);
      
      video->TimerRequest = (struct timerequest*)CreateIORequest(&video->TimerMsgPort, sizeof(struct timerequest));
        
      if(video->TimerRequest)
      {
        if(OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *) video->TimerRequest, 0))
        {
          DeleteIORequest((struct IORequest *) video->TimerRequest);
          video->TimerRequest = NULL;
        }
        else
          video->TimerBase = (struct Library *) video->TimerRequest->tr_node.io_Device;
      }
      printf("AllocVideo intui inits8\n");
      if(video->PixelMode == eWritePixelArray)
      {
        CopyMem(win->RPort, &video->TempRastPort, sizeof(struct RastPort));

        video->TempRastPort.Layer = NULL;

        video->TempRastPort.BitMap = AllocBitMap((((video->Width + 15) >> 4) << 4), 1, 8, 0, NULL);

        if(video->TempRastPort.BitMap)
        {
                      printf("AllocVideo out ok 1\n");
          return(video);
        }
          
        VError = OutOfMemory;
      }
      else
      {
          printf("AllocVideo out ok 2\n");

        return(video);
      }
    }
  }
  else
    VError = OutOfMemory;

  FreeVideo(video);
  printf("AllocVideo out bad\n");

  return(NULL);
}

void FreeVideo(struct Video *video)
{
  if(video)
  {
    if(video->Window)
    {
      if(video->VisualInfo && video->Menu)
        ClearMenuStrip(video->Window);

      if(video->VectorPen != -1)
        ReleasePen(video->Window->WScreen->ViewPort.ColorMap, video->VectorPen);

      if(video->BackFillPen != -1)
        ReleasePen(video->Window->WScreen->ViewPort.ColorMap, video->BackFillPen);

      CloseWindow(video->Window);

      if(video->PixelMode == eWritePixelArray)
      {
        if(video->TempRastPort.BitMap)
          FreeBitMap(video->TempRastPort.BitMap);
      }

      if(video->CGXHook)
        FreeCGXHook(video->CGXHook);

      if(video->RemapBuffer)
        FreeVec(video->RemapBuffer);

      if(video->VectorBitMap)
        FreeBitMap(video->VectorBitMap);

      if(video->VisualInfo)
        FreeVisualInfo(video->VisualInfo);
    }

    if(video->Screen)
    {
      if(video->BitMapLock)
        UnLockBitMap(video->BitMapLock);
    
      if(video->ScreenBufferChange && (video->Buffers == 2))
      {
        WaitPort(&video->ScreenBufferPort);
        GetMsg(&video->ScreenBufferPort);
      }

      WaitBlit();

      if(video->ScreenBuffers[0])
        FreeScreenBuffer(video->Screen, video->ScreenBuffers[0]);

      if(video->ScreenBuffers[1])
        FreeScreenBuffer(video->Screen, video->ScreenBuffers[1]);

      if(video->ScreenBuffers[2])
        FreeScreenBuffer(video->Screen, video->ScreenBuffers[2]);

      if(video->Pointer)
        FreeVec(video->Pointer);

      CloseScreen(video->Screen);
    }
    
    if(video->TimerRequest)
    {
      if(!CheckIO((struct IORequest *) video->TimerRequest))
      {
        AbortIO((struct IORequest *) video->TimerRequest);
        WaitIO((struct IORequest *) video->TimerRequest);
      }

      CloseDevice((struct IORequest *) video->TimerRequest);
      DeleteIORequest((struct IORequest *) video->TimerRequest);
    }

    FreeVec(video);
  }
}

void VBeginFrame(struct Video *video, LONG bgpen, UBYTE *palette)
{
  struct timeval time;

  ULONG *pal;
  ULONG *cmap;
  ULONG t;
  LONG  i, j, k, l, depth, rb, gb, bb;
  LONG  fps;

  if(video->TimerBase)
  {
    video->Frames[video->CurrentFrame].WaitTime.tv_secs  = 0;
    video->Frames[video->CurrentFrame].WaitTime.tv_micro = 0;
  
    if(video->FPS && video->LimitSpeed && video->TimerRequest)
    {
      if(!CheckIO((struct IORequest *) video->TimerRequest))
      {
        GetSysTime(&time);
        WaitIO((struct IORequest *) video->TimerRequest);
        GetSysTime(&video->Frames[video->CurrentFrame].WaitTime);
        SubTime(&video->Frames[video->CurrentFrame].WaitTime, &time);
        AddTime(&video->TotalWaitTime, &video->Frames[video->CurrentFrame].WaitTime);
      }
    
      video->TimerRequest->tr_node.io_Command = TR_ADDREQUEST;
      video->TimerRequest->tr_time.tv_secs    = 0;
      video->TimerRequest->tr_time.tv_micro   = (1 + video->FrameSkip) * 1000000 / video->FPS;
      SendIO((struct IORequest *) video->TimerRequest);
    }
  }

  video->BackgroundPen  = bgpen;

  if((video->Buffers > 1) && video->Screen)
  {
    if(video->CyberMode)
      video->OffsetY  = (video->Window->Height / video->Buffers) * video->CurrentScreenBuffer;
                        
    else if(video->ScreenBufferChange)
    {
      if(video->Buffers == 2)
      {
        WaitPort(&video->ScreenBufferPort);
        video->CurrentScreenBuffer = *((ULONG *) (GetMsg(&video->ScreenBufferPort) + 1));
      }
      else
        video->CurrentScreenBuffer = (video->CurrentScreenBuffer + 1) % video->Buffers;
      
      video->ScreenBufferRP.BitMap  = video->ScreenBuffers[video->CurrentScreenBuffer]->sb_BitMap;
      video->ScreenBufferChange   = FALSE;
    }
  }

  cmap = video->CMAP;

  if(video->Screen && (!video->PixelFormat))
  {
    depth = video->Screen->RastPort.BitMap->Depth;

    if(!video->RemapBuffer)
    {
      pal = video->Palette;

      for(i = 0, k = 0; i < (1 << depth); i++)
      {
        if(palette[4*i])
        {
          palette[4*i]  = 0;

          j = i;
          l = k;
          k++;
          do
          {
            cmap[i] = (palette[4*i+1] << 16) | (palette[4*i+2] << 8) | palette[4*i+3];

            pal[k++]  = palette[4*i+1] << 24;
            pal[k++]  = palette[4*i+2] << 24;
            pal[k++]  = palette[4*i+3] << 24;
            i++;
          } while((i < 256) && (palette[4*i]));

          pal[l]  = ((i - j) << 16) | j;
        }
      }

      if(k)
      {
        pal[k]  = 0;

        LoadRGB32(&video->Screen->ViewPort, pal);
      }
    }
    else
    {
      pal = video->PackedPalette;

      for(i = 0; i < 256; i++)
      {
        if(palette[4*i])
        {
          palette[4*i]  = 0;

          cmap[i] = (palette[4*i+1] << 16) | (palette[4*i+2] << 8) | palette[4*i+3];

          bb = depth / 3;
          rb = (depth - bb) / 2;
          gb = depth - rb - bb;
          
          ((UBYTE *) pal)[i]  = ((palette[4*i+1] >> (8 - rb)) << (gb + bb))
                    | ((palette[4*i+2] >> (8 - gb)) << (bb))
                    | (palette[4*i+3] >> (8 - bb));
        }
      }
    }
  }
  else if(video->CGXHook)
  {
    pal = video->PackedPalette;

    for(i = 0; i < 256; i++)
    {
      if(palette[4*i])
      {
        palette[4*i]  = 0;

        cmap[i] = (palette[4*i+1] << 16) | (palette[4*i+2] << 8) | palette[4*i+3];

        switch(video->PixelFormat)
        {
          case PIXFMT_RGB15:
            ((UWORD *) pal)[i] = ((palette[4*i+1]&0xf8)<<7)|((palette[4*i+2]&0xf8)<<2)|(palette[4*i+3]>>3);
            break;

          case PIXFMT_BGR15:
            ((UWORD *) pal)[i] = ((palette[4*i+3]&0xf8)<<7)|((palette[4*i+2]&0xf8)<<2)|(palette[4*i+1]>>3);
            break;

          case PIXFMT_RGB15PC:
            ((UWORD *) pal)[i] = ((palette[4*i+1]&0xf8)>>1)|(palette[4*i+2]>>6)|((palette[4*i+2]&0x38)<<10)|((palette[4*i+3]&0xf8)<<5);
            break;

          case PIXFMT_BGR15PC:
            ((UWORD *) pal)[i] = ((palette[4*i+3]&0xf8)>>1)|(palette[4*i+2]>>6)|((palette[4*i+2]&0x38)<<10)|((palette[4*i+1]&0xf8)<<5);
            break;

          case PIXFMT_RGB16:
            ((UWORD *) pal)[i] = ((palette[4*i+1]&0xf8)<<8)|((palette[4*i+2]&0xfc)<<3)|(palette[4*i+3]>>3);
            break;

          case PIXFMT_BGR16:
            ((UWORD *) pal)[i] = ((palette[4*i+3]&0xf8)<<8)|((palette[4*i+2]&0xfc)<<3)|(palette[4*i+1]>>3);
            break;

          case PIXFMT_RGB16PC:
            ((UWORD *) pal)[i] = (palette[4*i+1]&0xf8)|(palette[4*i+2]>>5)|((palette[4*i+2]&0x1c)<<11)|((palette[4*i+3]&0xf8)<<5);
            break;

          case PIXFMT_BGR16PC:
            ((UWORD *) pal)[i] = (palette[4*i+3]&0xf8)|(palette[4*i+2]>>5)|((palette[4*i+2]&0x1c)<<11)|((palette[4*i+1]&0xf8)<<5);
            break;

          case PIXFMT_BGR24:
            ((ULONG *) pal)[i]  = (palette[4*i+3]<<16)|(palette[4*i+2]<<8)|palette[4*i+1];
            break;

          case PIXFMT_RGB24:
          case PIXFMT_ARGB32:
            ((ULONG *) pal)[i]  = (palette[4*i+1]<<16)|(palette[4*i+2]<<8)|palette[4*i+3];
            break;

          case PIXFMT_BGRA32:
            ((ULONG *) pal)[i]  = (palette[4*i+3]<<24)|(palette[4*i+2]<<16)|(palette[4*i+1]<<8);
            break;

          case PIXFMT_RGBA32:
            ((ULONG *) pal)[i]  = (palette[4*i+1]<<24)|(palette[4*i+2]<<16)|(palette[4*i+3]<<8);
            break;
        }
        
        video->Palette[3*i+1] = palette[4*i+1];
        video->Palette[3*i+2] = palette[4*i+2];
        video->Palette[3*i+3] = palette[4*i+3];
      }
    }
  }

  if(video->TimerBase)
  {
    /* Calculate fps. */

    GetSysTime(&time);
    video->Frames[video->CurrentFrame].Time   = time;
    video->Frames[video->CurrentFrame].Skipped  = video->FrameSkip + 1;
    video->TotalFrames += video->Frames[video->CurrentFrame].Skipped;
    video->CurrentFrame = (video->CurrentFrame + 1) % V_FrameHistory;
    video->TotalFrames -= video->Frames[video->CurrentFrame].Skipped;
    SubTime(&video->TotalWaitTime, &video->Frames[video->CurrentFrame].WaitTime);

    SubTime(&time, &video->Frames[video->CurrentFrame].Time);
    t = ((time.tv_secs * 1000) + (time.tv_micro / 1000));
    if(t)
      video->CurrentFPS = (video->TotalFrames * 1000) / t;
    else
      video->CurrentFPS = 0;

    /* Auto frameskip. */

    if(video->AutoFrameSkip && video->FPS)
    {
      /* Calculate what the fps would be without speed limiting. */
    
      SubTime(&time, &video->TotalWaitTime);
      t = ((time.tv_secs * 1000) + (time.tv_micro / 1000));
      if(t)
        fps = (video->TotalFrames * 1000) / t;
      else
        fps = 0;

      if((fps < (video->FPS - 5)) && (video->FrameSkip < video->MaxFrameSkip))
        video->FrameSkip++;
      else if((fps > (video->FPS + 5)) && (video->FrameSkip > 0))
        video->FrameSkip--;
    }
  }
}

void VEndFrame(struct Video *video)
{
  if((video->Buffers > 1) && video->Screen)
  {
    if(video->CyberMode)
    {
      video->Screen->ViewPort.RasInfo->RyOffset = (video->Window->Height / video->Buffers) * video->CurrentScreenBuffer;
      ScrollVPort(&video->Screen->ViewPort);
      video->CurrentScreenBuffer  = (video->CurrentScreenBuffer + 1) % video->Buffers;
    }
    else
    {
      video->ScreenBufferChange = ChangeScreenBuffer( video->Screen, 
                                video->ScreenBuffers[video->CurrentScreenBuffer]);
    }
  }
}

struct VPixelArray *VAllocPixelArray(struct Video *video, LONG width, LONG height,
                                     LONG dirty, ULONG *pixel_formats)
{
  struct VPixelArray *pa;

  ULONG pixel_format;
  ULONG depth;
  LONG  size;
  LONG  w, i;

  if(width && height)
  {
    switch(video->PixelMode)
    {
      case eCustomC2P:
        w = ((width + 31) >> 5) << 5;
        break;

      case eCGXHook:
      case eBltBitMapRastPort:
      case eWriteChunkyPixels:
        w = ((width + 3) >> 2) << 2;
        break;

      default:
        if(video->RemapBuffer)
          w = ((width + 3) >> 2) << 2;
        else
          w = ((width + 15) >> 4) << 4;
    }
  }
  else
    w = 0;

  size = sizeof(struct VPixelArray);

  if(video->PixelMode != eBltBitMapRastPort)
    size += (w * height);
  
  if(dirty)
    size += height;

  pa  = (struct VPixelArray *)memAlloc(size);

  if(pa)
  {
    pa->Video  = video;
    pa->Size   = size;
    pa->Width  = width;
    pa->Height = height;
    pa->StartX = 0;
    pa->StartY = 0;
    pa->EndX   = width - 1;
    pa->EndY   = height - 1;

    if(video->PixelMode == eBltBitMapRastPort)
    {
      if(pixel_formats)
      {
        pixel_format = GetCyberMapAttr(video->Window->RPort->BitMap, CYBRMATTR_PIXFMT);

        /* Try to find the pixel format of the video window in the list
         * of possible pixel formats for this pixel array. */
      
        for(i = 0; pixel_formats[i] != ~0; i++)
        {
          if(pixel_formats[i] == pixel_format)
            break;
        }
        
        if(pixel_formats[i] == ~0)
        {
          /* Didn't find the exact pixel format so try to find one that will map
           * to the pixel format of the window. */
        
          if((pixel_format == PIXFMT_LUT8) || (pixel_formats[0] == ~0))
          {
            /* If the possible pixel formats list is empty or the pixel format
             * of the video window is palette based then fail. Any true color
             * pixel format can't be mapped to a palette based one. */
          
            memFree(pa);
            
            return(NULL);
          }
          
          pixel_format = pixel_formats[0];
        }
      }
      else
      {
        /* An empty possible pixel formats list defaults to only
         * a palette based pixel format. */
      
        pixel_format = PIXFMT_LUT8;
      }
    
      switch(pixel_format)
      {
        case PIXFMT_RGB15:
        case PIXFMT_BGR15:
        case PIXFMT_RGB15PC:
        case PIXFMT_BGR15PC:
          depth = 15;
          break;


        case PIXFMT_RGB16:
        case PIXFMT_BGR16:
        case PIXFMT_RGB16PC:
        case PIXFMT_BGR16PC:
          depth = 16;
          break;

        case PIXFMT_RGB24:
        case PIXFMT_BGR24:
          depth = 24;
          break;

        case PIXFMT_ARGB32:
        case PIXFMT_BGRA32:
        case PIXFMT_RGBA32:
          depth = 32;
          break;

        case PIXFMT_LUT8:
        default:
          depth = 8;
          break;
      }
      
      pa->BitMap = AllocBitMap(width, height, depth,
                               BMF_MINPLANES|BMF_CLEAR|BMF_SPECIALFMT|SHIFT_PIXFMT(pixel_format),
                               video->Window->RPort->BitMap);

      if(!pa->BitMap)
      {
        memFree(pa);
        
        return(NULL);
      }

      pa->BytesPerRow   = GetCyberMapAttr(pa->BitMap, CYBRMATTR_XMOD);
      pa->BytesPerPixel = GetCyberMapAttr(pa->BitMap, CYBRMATTR_BPPIX);
      pa->PixelFormat   = GetCyberMapAttr(pa->BitMap, CYBRMATTR_PIXFMT);
      pa->Pixels        = (UBYTE *) GetCyberMapAttr(pa->BitMap, CYBRMATTR_DISPADR);
      pa->PixelsSize    = pa->BytesPerRow * height;

      if(dirty)
        pa->DirtyLines = (UBYTE *) &pa[1];
    }
    else
    {
      pa->BitMap        = NULL;
      pa->BytesPerRow   = w;
      pa->BytesPerPixel = 1;
      pa->PixelFormat   = PIXFMT_LUT8;
      pa->Pixels        = (UBYTE *) &pa[1];
      pa->PixelsSize    = 0;

      if(dirty)
        pa->DirtyLines  = &pa->Pixels[w * height];
    }

    for(i = 0; i < 256; i++)
    {
      pa->Palette[i][0] = 1;
      pa->Palette[i][1] = 0;
      pa->Palette[i][2] = 0;
      pa->Palette[i][3] = 0;
    }

#ifdef POWERUP
    PPCCacheClearE(pa, pa->Size, CACRF_ClearD);
#endif
  }

  return(pa);
}

void VFreePixelArray(struct VPixelArray *pa)
{
  if(pa->BitMap)
    FreeBitMap(pa->BitMap);

  memFree(pa);
}

LONG _WritePixelArray(APTR a, UWORD b, UWORD c, UWORD d, struct RastPort *e, UWORD f, UWORD g, UWORD h, UWORD i, UBYTE j)
{
  LONG r;
  
  r = WritePixelArray(a,b,c,d,e,f,g,h,i,j);

  return(r);  
}

void VDrawPixelArray(struct VPixelArray *pa)
{
  struct Video  *video;

  UBYTE *pixels;
  UBYTE *remap_pixels;
  UBYTE *dirty_lines;
  ULONG bpr;
  ULONG remap_bpr;
  LONG  x, y, w, h;
  LONG  sy, iy;

  if(!pa->Width || !pa->Height)
    return;

  video = pa->Video;
  w   = pa->EndX - pa->StartX + 1;
  h   = pa->EndY - pa->StartY + 1;
  x   = video->Left + ((video->Width - w) >> 1);
  y   = video->Top + video->OffsetY + ((video->Height - h) >> 1);

  if(video->Screen)
    x &= 0xfffffffc;

  if(pa->DirtyLines)
  {
    dirty_lines = pa->DirtyLines + pa->StartY;

    switch(video->PixelMode)
    {
      case eCustomC2P:
        x = x & 0xffffffe0;
        w = ((w + 31) >> 5) << 5;
        VSetFrameBox(video, x, y, x + w - 1, y + h - 1);

        pixels  = VGetPixelArrayAddr(pa);
        bpr   = pa->BytesPerRow;

        if(video->RemapBuffer)
        {
          remap_pixels  = video->RemapBuffer;
          remap_bpr   = ((video->Width + 31) >> 5) << 5;
          
          for(iy = 0; iy < h; iy++)
          {
            if(dirty_lines[iy])
            {
              dirty_lines[iy] = 0;

              sy = iy;
              iy++;
            
              while(dirty_lines[iy] && (iy < h))
                dirty_lines[iy++] = 0;

              RemapPixels(pixels + (bpr * sy), remap_pixels, (UBYTE *) video->PackedPalette,
                    w, remap_bpr, iy - sy, bpr);

              c2p(remap_pixels, video->RastPort->BitMap,
                  w,            iy - sy,
                  x,            y + sy,
                  bpr);
            }
          }
        }
        else
        {
          for(iy = 0; iy < h; iy++)
          {
            if(dirty_lines[iy])
            {
              dirty_lines[iy] = 0;

              sy = iy;
              iy++;
            
              while(dirty_lines[iy] && (iy < h))
                dirty_lines[iy++] = 0;

              c2p(pixels + (bpr * sy), video->RastPort->BitMap,
                  w,                   iy - sy,
                  x,                    y + sy,
                  bpr);
            }
          }
        }
        break;

      case eBltBitMapRastPort:
        VSetFrameBox(video, x, y, x + w - 1, y + h - 1);
        pixels = VGetPixelArrayAddr(pa);
        for(iy = 0; iy < h; iy++)
        {
          if(dirty_lines[iy])
          {
            dirty_lines[iy] = 0;

            sy = iy;
            iy++;
            
            while(dirty_lines[iy] && (iy < h))
              dirty_lines[iy++] = 0;
            
            BltBitMapRastPort(pa->BitMap, pa->StartX, pa->StartY + sy, video->RastPort,
                              x, y + sy, w, iy - sy, 0xc0);
          }
        }
        break;

      case eWriteChunkyPixels:
        VSetFrameBox(video, x, y, x + w - 1, y + h - 1);
        pixels = VGetPixelArrayAddr(pa);
        for(iy = 0; iy < h; iy++)
        {
          if(dirty_lines[iy])
          {
            dirty_lines[iy] = 0;

            sy = iy;
            iy++;
            
            while(dirty_lines[iy] && (iy < h))
              dirty_lines[iy] = 0;
            
            WriteChunkyPixels(  video->RastPort, x, y + sy, x + w - 1, y + iy - 1,
                      pixels + (pa->BytesPerRow * sy), pa->BytesPerRow);
          }
        }
        break;

      case eCGXHook:
        VSetFrameBox(video, x, y, x + w - 1, y + h - 1);
        pixels = VGetPixelArrayAddr(pa);
        for(iy = 0; iy < h; iy++)
        {
          if(dirty_lines[iy])
          {
            dirty_lines[iy] = 0;

            sy = iy;
            iy++;
            
            while(dirty_lines[iy] && (iy < h))
              dirty_lines[iy] = 0;
            
            DoCLUT8RemapHook(video->CGXHook,
                             pixels + (pa->BytesPerRow * sy), video->RastPort,
                             0, 0, x, y + sy, w, iy - sy, pa->BytesPerRow);
          }
        }
        break;    
    }
  }
  else
  {
    switch(video->PixelMode)
    {
      case eCustomC2P:
        x   = x & 0xffffffe0;
        w   = ((w + 31) >> 5) << 5;
        bpr   = pa->BytesPerRow;
        VSetFrameBox(video, x, y, x + w - 1, y + h - 1);
        pixels  = VGetPixelArrayAddr(pa);
        if(video->RemapBuffer)
        {
          bpr = ((video->Width + 31) >> 5) << 5;
          RemapPixels(pixels, video->RemapBuffer,  (UBYTE *) video->PackedPalette,
                w, bpr, h, pa->BytesPerRow);
          pixels = video->RemapBuffer;
        }

        c2p( pixels, video->RastPort->BitMap,
            w,    h,
            x,    y,
            bpr);
        break;

      case eBltBitMapRastPort:
        VSetFrameBox(video, x, y, x + w - 1, y + h - 1);
        BltBitMapRastPort(pa->BitMap, pa->StartX, pa->StartY, video->RastPort,
                          x, y, w, h, 0xc0);
        break;
      case eWriteChunkyPixels:
        VSetFrameBox(video, x, y, x + w - 1, y + h - 1);
        WriteChunkyPixels(  video->RastPort, x, y, x + w - 1, y + h - 1,
                  VGetPixelArrayAddr(pa), pa->BytesPerRow);
        break;
      case eCGXHook:
        VSetFrameBox(video, x, y, x + w - 1, y + h - 1);
        DoCLUT8RemapHook( video->CGXHook,
                  VGetPixelArrayAddr(pa), video->RastPort,
                  0,            0,
                  x,            y,
                  w,            h,
                  pa->BytesPerRow);
        break;    
    }
  }
}

void VSetPixelFrame(struct VPixelArray *pa)
{
  struct Video *video;

  video = pa->Video;

  VBeginFrame(video, pa->BackgroundPen, &pa->Palette[0][0]);

  video->CurrentPixelArray  = pa;
  video->CurrentVectorArray = NULL;

  VDrawPixelArray(pa);

  VEndFrame(video);
}

struct VVectorArray *VAllocVectorArray( struct Video *video, LONG width, LONG height, LONG length)
{
  struct VVectorArray *va;

  va  = (struct VVectorArray *)memAlloc(sizeof(struct VVectorArray) + length * sizeof(struct VVector));

  if(va)
  {
    va->Video   = video;
    va->Size    = sizeof(struct VVectorArray) + length * sizeof(struct VVector);
    va->Width   = width;
    va->Height    = height;
    va->MaxLength = length;
    va->Vectors   = (struct VVector *) &va[1];

#ifdef POWERUP
    PPCCacheClearE(va, va->Size, CACRF_ClearD);
#endif
  }

  return(va);
}

void VFreeVectorArray(struct VVectorArray *va)
{
  memFree(va);
}

void VDrawVectorArray(struct VVectorArray *va)
{
  struct Video  *video;
  struct RastPort *rp;
  LONG      i, t, x1, y1, x2, y2, dx, dy, w, h, left, top, width, height;

  video = va->Video;
  rp    = NULL;
  left  = video->Left;
  top   = video->Top + video->OffsetY;
  width = video->Width;
  height  = video->Height;
  w   = va->Width;
  h   = va->Height;

  VSetFrameBox(video, left, top, left + w - 1, top + h - 1);

  if((video->Buffers > 1) && !video->Screen)
  {
    if(!video->VectorBitMap)
    {
      video->VectorBitMap = AllocBitMap(  width,  height, 
                        GetBitMapAttr(video->Window->RPort->BitMap, BMA_DEPTH),
                        BMF_MINPLANES,  video->Window->RPort->BitMap);
      if(video->VectorBitMap)
      {
        InitRastPort(&video->VectorRastPort);
        video->VectorRastPort.Layer   = NULL;
        video->VectorRastPort.BitMap  = video->VectorBitMap;
      }
    }

    if(video->VectorBitMap)
    {
      rp   = &video->VectorRastPort;
      left = 0;
      top  = 0;
    }
  }
  else
    rp    = video->RastPort;

  if(rp)
  {
    if(video->LessFlicker && !(video->Buffers > 1) && !video->VectorBitMap)
    {
      video->VectorBitMap = AllocBitMap(width, height, 1, 0, 0);
      if(video->VectorBitMap)
      {
        InitRastPort(&video->VectorRastPort);
        video->VectorRastPort.Layer   = NULL;
        video->VectorRastPort.BitMap  = video->VectorBitMap;
      }
    }

    if(!video->LessFlicker || !video->VectorBitMap)
    {
      if(video->CGXHook)
      {
        FillPixelArray( rp,
                left,   top,
                width,    height,
                video->BackgroundColor);
      }
      else
      {
        SetAPen(rp, video->BackgroundPen);

        RectFill( rp,
              left,       top,
              left + width - 1, top + height - 1);
      }
    }
    else
    {
      SetAPen(&video->VectorRastPort, 1);
      RectFill(&video->VectorRastPort, 0, 0, width - 1, height - 1);
      SetAPen(&video->VectorRastPort, 0);
    }

    if(video->CGXHook)
      SetAPen(rp, video->VectorPen);

    x1  = video->VectorX;
    y1  = video->VectorY;

    for(i = 0; i < va->Length; i++)
    {
      if(va->Vectors[i].Pen >= 0)
      {
        x2  = va->Vectors[i].X;
        y2  = va->Vectors[i].Y;

        if(x1 > x2)
        {
          t = x2;
          x2  = x1;
          x1  = t;
          t = y2;
          y2  = y1;
          y1  = t;
        }

        if((x1 < w) && (x2 >= 0))
        {
          if(x1 != x2)
          {
            dy  = ((y2 - y1) << 16) / (x2 - x1);

            if(x1 < 0)
            {
              x1  = 0;
              y1  = y1 - ((x1 * dy) >> 16);
            }

            if(x2 >= w)
            {
              y2  = y2 - (((x2 - w - 1) * dy) >> 16);
              x2  = w - 1;
            }
          }

          if(y1 > y2)
          {
            t = x2;
            x2  = x1;
            x1  = t;
            t = y2;
            y2  = y1;
            y1  = t;
          }
          
          if((y1 < h) && (y2 >= 0))
          {
            if(y1 != y2)
            {
              dx  = ((x2 - x1) << 16) / (y2 - y1);

              if(y1 < 0)
              {
                x1  = x1 - ((y1 * dx) >> 16);
                y1  = 0;
              }

              if(y2 >= h)
              {
                x2  = x2 - (((y2 - h - 1) * dx) >> 16);
                y2  = h - 1;
              }
            }

            if(video->CGXHook)
            {
              SetRGB32( &video->Window->WScreen->ViewPort, video->VectorPen,
                    video->Palette[3*va->Vectors[i].Pen+1] << 24,
                    video->Palette[3*va->Vectors[i].Pen+2] << 24,
                    video->Palette[3*va->Vectors[i].Pen+3] << 24);
            }
            else
              SetAPen(rp, va->Vectors[i].Pen);

            Move(rp, left + x1, top + y1);
            Draw(rp, left + x2, top + y2);

            if(video->LessFlicker && video->VectorBitMap)
            {
              Move(&video->VectorRastPort, x1, y1);
              Draw(&video->VectorRastPort, x2, y2);
            }
          }
        }
      }

      x1 = va->Vectors[i].X;
      y1 = va->Vectors[i].Y;
    }
    
    video->VectorX  = x1;
    video->VectorY  = y1;

    if(video->VectorBitMap)
    {
      if(video->Buffers > 1)
        BltBitMapRastPort(  video->VectorBitMap,  0,        0,
                  video->RastPort,    video->Left,  video->Top + video->OffsetY,
                  width,          height,     0xc0);
      else
      {
        if(video->CGXHook)
        {
          SetRGB32( &video->Window->WScreen->ViewPort, video->VectorPen,
                video->Palette[3*va->BackgroundPen+1] << 24,
                video->Palette[3*va->BackgroundPen+2] << 24,
                video->Palette[3*va->BackgroundPen+3] << 24);
        }
        else
          SetAPen(rp, va->BackgroundPen);

        SetDrMd(rp, JAM1);

        BltTemplate(video->VectorBitMap->Planes[0],   0, 
              video->VectorBitMap->BytesPerRow, rp, 
              video->Left,            video->Top + video->OffsetY,
              width,                height);
      }
    }
  }
}

void VSetVectorFrame(struct VVectorArray *va)
{
  struct Video  *video;

  video = va->Video;

  VBeginFrame(video, va->BackgroundPen, &va->Palette[0][0]);

  video->CurrentPixelArray  = NULL;
  video->CurrentVectorArray = va;

  VDrawVectorArray(va);

  VEndFrame(video);
}

struct VDirectArray *VAllocDirectArray(struct Video *video, LONG width, LONG height)
{
  struct VDirectArray *da;
  LONG        i;

#ifdef POWERUP  
  da = PPCAllocVec(sizeof(struct VDirectArray), MEMF_PUBLIC|MEMF_CLEAR|MEMF_NOCACHESYNCPPC|MEMF_NOCACHESYNCM68K);
#else
  da = (struct VDirectArray *)AllocVec(sizeof(struct VDirectArray), MEMF_PUBLIC|MEMF_CLEAR);
#endif

  if(da)
  {
    da->Video = video;
    da->Width = width;
    da->Height  = height;

    for(i = 0; i < 256; i++)
    {
      da->Palette[i][0] = 1;
      da->Palette[i][1] = 0;
      da->Palette[i][2] = 0;
      da->Palette[i][3] = 0;
    }
  }
  
  return(da);
}

void VFreeDirectArray(struct VDirectArray *da)
{
#ifdef POWERUP
  PPCFreeVec(da);
#else
  FreeVec(da);
#endif
}

void VSetDirectFrame(struct VDirectArray *da)
{
  struct Video  *video;
  LONG      x;
  LONG      y;

  video = da->Video;

  if(video->CyberMode)
  {
    if(video->BitMapLock)
      UnLockBitMap(video->BitMapLock);

    VBeginFrame(video, da->BackgroundPen, &da->Palette[0][0]);
    VEndFrame(video);

    video->OffsetY = (video->Window->Height / video->Buffers) * video->CurrentScreenBuffer;

    x = (video->Left + ((video->Width - da->Width) >> 1)) & 0xfffffffc;;
    y = video->Top + video->OffsetY + ((video->Height - da->Height) >> 1);

    VSetFrameBox(video, x, y, x + da->Width - 1, y + da->Height - 1);

    video->CurrentPixelArray  = NULL;
    video->CurrentVectorArray = NULL;

    video->BitMapLock = LockBitMapTags(video->Screen->RastPort.BitMap,  LBMI_BASEADDRESS, (ULONG) &da->Pixels,
                                      LBMI_BYTESPERROW, (ULONG) &da->BytesPerRow,
                                      TAG_END);
    if(video->BitMapLock)
      da->Pixels += x + (da->BytesPerRow * y);
    else
    {
      da->Pixels    = NULL;
      da->BytesPerRow = 0;
    }
  }
}

void VSetFrameBox(struct Video *video, LONG sx, LONG sy, LONG ex, LONG ey)
{
  struct Rect32 *frame_box;

  frame_box = &video->FrameBox[video->CurrentScreenBuffer];

  if((sx > frame_box->MinX) || (sy > frame_box->MinY)
  || (ex < frame_box->MaxX) || (ey < frame_box->MaxY))
  {
    if(video->CGXHook)
    {
      FillPixelArray( video->RastPort,
              frame_box->MinX,
              frame_box->MinY,
              frame_box->MaxX - frame_box->MinX + 1,
              frame_box->MaxY - frame_box->MinY + 1,
              video->BackgroundColor);
    }
    else
    {
      SetAPen(video->RastPort, video->BackgroundPen);

      RectFill( video->RastPort,
            frame_box->MinX,  frame_box->MinY,
            frame_box->MaxX,  frame_box->MaxY);
    }
  }

  frame_box->MinX = sx;
  frame_box->MinY = sy;
  frame_box->MaxX = ex;
  frame_box->MaxY = ey;
}

void VRefresh(struct Video *video)
{
  if(video->CurrentPixelArray)
    VDrawPixelArray(video->CurrentPixelArray);
  else if(video->CurrentVectorArray)
    VDrawVectorArray(video->CurrentVectorArray);
}

void VSetFrameSkip(struct Video *video, LONG frameskip)
{
  video->FrameSkip = frameskip;
}

void VSetLimitSpeed(struct Video *video, LONG limitspeed)
{
  video->LimitSpeed = limitspeed;
}

LONG VSaveILBM(struct Video *video, STRPTR filename)
{
  BPTR  file;
  APTR  buffer;
  UBYTE  *buf;
  UBYTE *pix;
  UBYTE *line;
  LONG  w;
  LONG  h;
  LONG  i;
  LONG  j;
  LONG  k;
  LONG  l;
  UWORD t;

  if(video->CurrentPixelArray)
  {
    w = video->CurrentPixelArray->EndX - video->CurrentPixelArray->StartX + 1;
    h = video->CurrentPixelArray->EndY - video->CurrentPixelArray->StartY + 1;
    
    buffer = AllocVec(9*sizeof(ULONG)+sizeof(struct BMHD)+(256*3)+(((w+15)&0xfffffff0)*h), MEMF_PUBLIC);
    
    buf = (UBYTE *)buffer;
    
    if(buf)
    {
      file = Open(filename, MODE_NEWFILE);
      
      if(file)
      {
        *((ULONG *) buf)  = ID_FORM;
        buf+=4;
        *((ULONG *) buf)  = 7*sizeof(ULONG)+sizeof(struct BMHD)+(256*3)+(((w+15)&0xfffffff0)*h);
        buf+=4;
        *((ULONG *) buf)  = MAKE_ID('I','L','B','M');
        buf+=4;
        *((ULONG *) buf)  = MAKE_ID('B','M','H','D');
        buf+=4;
        *((ULONG *) buf)  = sizeof(struct BMHD);
        buf+=4;
        ((struct BMHD *) buf)->Width      = w;
        ((struct BMHD *) buf)->Height     = h;
        ((struct BMHD *) buf)->Left       = 0;
        ((struct BMHD *) buf)->Top        = 0;
        ((struct BMHD *) buf)->Planes     = 8;
        ((struct BMHD *) buf)->Masking      = 0;
        ((struct BMHD *) buf)->Compression    = 0;
        ((struct BMHD *) buf)->pad        = 0;
        ((struct BMHD *) buf)->TransparentColor = 0;
        ((struct BMHD *) buf)->XAspect      = 1;
        ((struct BMHD *) buf)->YAspect      = 1;
        ((struct BMHD *) buf)->PageWidth    = w;
        ((struct BMHD *) buf)->PageHeight   = h;
        buf += sizeof(struct BMHD);
        *((ULONG *) buf)  = MAKE_ID('C','M','A','P');
         buf+=4;
        *((ULONG *) buf)  = 256*3;
         buf+=4;
        for(i = 0; i < 256; i++)
        {
          *buf++ = video->CMAP[i] >> 16;
          *buf++ = (video->CMAP[i] >> 8) & 0xff;
          *buf++ = video->CMAP[i] & 0xff;
        }
        *((ULONG *) buf)  = MAKE_ID('B','O','D','Y');
         buf+=4;
        *((ULONG *) buf)  = (((w+15)&0xfffffff0)*h);
         buf+=4;
        line = VGetPixelArrayAddr(video->CurrentPixelArray);
        for(i = 0; i < h; i++)
        {
          for(j = 0; j < 8; j++)
          {
            t = 0;
            l = 15;
            pix = line;

            for(k = 0; k < w; k++)
            {
              if(l > j)
                t |= (*pix++ << (l - j)) & (1 << l);
              else
                t |= (*pix++ >> (j - l)) & (1 << l);
              
              if(l)
                l--;
              else
              {
                *((UWORD *) buf) = t;
                buf+=2;
                t = 0;
                l = 15;
              }
            }
            
            if(l < 15)
              *((UWORD *) buf) = t;
                buf+=2;
          }
          
          line += video->CurrentPixelArray->BytesPerRow;
        }


        Write(file, buffer, 9*sizeof(ULONG)+sizeof(struct BMHD)+(256*3)+(((w+15)&0xfffffff0)*h));
      
        Close(file);
      }
      
      FreeVec(buffer);
    }
  }
  
  return(0);
}

LONG VGetFrameSkip(struct Video *video)
{
  return(video->FrameSkip);
}

LONG VGetFPS(struct Video *video)
{
  return(video->CurrentFPS);
}
