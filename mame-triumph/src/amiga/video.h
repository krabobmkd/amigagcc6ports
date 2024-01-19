#ifndef VIDEO_H
#define VIDEO_H
/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id: video.h,v 1.1 1999/04/28 18:54:28 meh Exp meh $
 *
 * $Log: video.h,v $
 * Revision 1.1  1999/04/28 18:54:28  meh
 * Initial revision
 *
 *
 *************************************************************************/

#include <exec/types.h>

#ifdef POWERUP
#ifdef __PPC
#include <powerup/gcclib/powerup_protos.h>
#else
#include <exec/execbase.h>
#include <inline/ppc.h>
extern struct Library *PPCLibBase;
#endif
#endif

#include <macros.h>

#define VA_UseScreen     (TAG_USER)
#define VA_UseScreenReq  (TAG_USER+1)
#define VA_Width         (TAG_USER+2)
#define VA_Height        (TAG_USER+3)
#define VA_Depth         (TAG_USER+4)
#define VA_ModeID        (TAG_USER+5)
#define VA_Buffers       (TAG_USER+6)
#define VA_LessFlicker   (TAG_USER+7)
#define VA_Title         (TAG_USER+8)
#define VA_Menu          (TAG_USER+9)
#define VA_NoRefresh     (TAG_USER+10)
#define VA_FPS           (TAG_USER+11)
#define VA_MaxColors     (TAG_USER+12)
#define VA_AutoFrameSkip (TAG_USER+13)
#define VA_MaxFrameSkip  (TAG_USER+14)

typedef enum
{
  None,
  OutOfMemory,
  OpenScreenFailed,
  OpenWindowFailed
} verror_t;

extern verror_t VError;

#define V_FrameHistory  4

struct Video;

struct VPixelArray
{
  struct Video  *Video;
  struct BitMap *BitMap;
  LONG          PixelFormat;
  LONG          Size;
  LONG          Width;
  LONG          Height;
  LONG          StartX;
  LONG          StartY;
  LONG          EndX;
  LONG          EndY;
  LONG          BytesPerRow;
  LONG          BytesPerPixel;
  LONG          PixelsSize;
  UBYTE         *Pixels;
  UBYTE         *DirtyLines;
  LONG          BackgroundPen;
  UBYTE         Palette[256][4];
};

struct VVector
{
  WORD  X;
  WORD  Y;
  LONG  Pen;
};

struct VVectorArray
{
  struct Video   *Video;
  LONG           Size;
  LONG           Width;
  LONG           Height;
  LONG           Length;
  LONG           MaxLength;
  struct VVector *Vectors;
  LONG           BackgroundPen;
  UBYTE          Palette[256][4];
};

struct VDirectArray
{
  struct Video *Video;
  LONG         Size;
  LONG         Width;
  LONG         Height;
  LONG         BytesPerRow;
  UBYTE        *Pixels;
  LONG         BackgroundPen;
  UBYTE        Palette[256][4];
};

struct VFrame
{
  struct timeval Time;
  struct timeval WaitTime;
  LONG           Skipped;
};

typedef enum
{
  WritePixelArray,
  CustomC2P,
  CGXHook,
  WriteChunkyPixels,
  BltBitMapRastPort
} vpixelmode_t;

struct Video
{
  LONG                Left;
  LONG                Top;
  LONG                Width;
  LONG                Height;
  LONG                OffsetY;
  ULONG               Buffers;
  LONG                LessFlicker;
  APTR                VisualInfo;
  struct Menu         *Menu;
  struct RastPort     *RastPort;
  struct Window       *Window;
  struct Hook         BackFillHook;
  struct Screen       *Screen;
  struct ScreenBuffer *ScreenBuffers[3];
  struct MsgPort      ScreenBufferPort;
  struct RastPort     ScreenBufferRP;
  LONG                ScreenBufferChange;
  LONG                CurrentScreenBuffer;
  LONG                CyberMode;
  vpixelmode_t        PixelMode;
  ULONG               PixelFormat;
  struct Rect32       FrameBox[3];
  struct VPixelArray  *CurrentPixelArray;
  struct VVectorArray *CurrentVectorArray;
  struct CGXHook      *CGXHook;
  struct RastPort     TempRastPort;
  struct BitMap       *VectorBitMap;
  struct RastPort     VectorRastPort;
  LONG                VectorX;
  LONG                VectorY;
  LONG                VectorPen;
  LONG                BackFillPen;
  LONG                BackgroundPen;
  ULONG               BackgroundColor;
  ULONG               Palette[3*256+2];
  ULONG               PackedPalette[256];
  ULONG               CMAP[256];
  APTR                Pointer;
  APTR                BitMapLock;
  UBYTE               *RemapBuffer;
  struct timerequest  *TimerRequest;
  struct MsgPort      TimerMsgPort;
  struct Library      *TimerBase;
  LONG                FPS;
  LONG                CurrentFPS;
  LONG                FrameSkip;
  LONG                LimitSpeed;
  LONG                AutoFrameSkip;
  LONG                MaxFrameSkip;
  LONG                CurrentFrame;
  LONG                TotalFrames;
  struct VFrame       Frames[V_FrameHistory];
  struct timeval      TotalWaitTime;
};


struct Video *AllocVideo(Tag tags,...);
void FreeVideo(struct Video *video);
void VBeginFrame(struct Video *video, LONG bgpen, UBYTE *palette);
void VEndFrame(struct Video *video);
struct VPixelArray *VAllocPixelArray(struct Video *video, LONG width, LONG height, LONG dirty, ULONG *pixel_formats);
void VFreePixelArray(struct VPixelArray *pixelarray);
void VDrawPixelArray(struct VPixelArray *pixelarray);
void VSetPixelFrame(struct VPixelArray *pixelarray);
struct VVectorArray *VAllocVectorArray(struct Video *video, LONG width, LONG height, LONG length);
void VFreeVectorArray(struct VVectorArray *vectorarray);
void VDrawVectorArray(struct VVectorArray *vectorarray);
void VSetVectorFrame(struct VVectorArray *vectorarray);
struct VDirectArray *VAllocDirectArray(struct Video *video, LONG width, LONG height);
void VFreeDirectArray(struct VDirectArray *directarray);
void VSetDirectFrame(struct VDirectArray *directarray);
void VSetFrameBox(struct Video *video, LONG sx, LONG sy, LONG ex, LONG ey);
void VRefresh(struct Video *video);
BOOL VBeginDirectDraw(struct Video *video, ULONG *bitmap, ULONG *byteperrow);
void VEndDirectDraw(struct Video *video);
void VSetFrameSkip(struct Video *video, LONG frameskip);
LONG VGetFrameSkip(struct Video *video);
LONG VGetFPS(struct Video *video);
void VSetLimitSpeed(struct Video *video, LONG limitspeed);
LONG VSaveILBM(struct Video *video, STRPTR filename);

static inline void VAddVector(struct VVectorArray *va, LONG x, LONG y, LONG p)
{
  if(va->Length < va->MaxLength)
  {
    va->Vectors[va->Length].X     = x;
    va->Vectors[va->Length].Y     = y;
    va->Vectors[va->Length++].Pen = p;
  }
}

static inline void VResetVectorArray(struct VVectorArray *va)
{
  va->Length = 0;
}

static inline void VSetPixelArrayBox(struct VPixelArray *pa, LONG sx,
                                     LONG sy, LONG ex, LONG ey)
{
  pa->StartX = sx;
  pa->StartY = sy;
  pa->EndX   = ex;
  pa->EndY   = ey;
}

static inline UBYTE *VGetPixelArrayAddr(struct VPixelArray *pa)
{
  return(pa->Pixels + pa->BytesPerPixel*pa->StartX + pa->BytesPerRow*pa->StartY);
}

static inline UBYTE *VGetPixelArrayPointAddr(struct VPixelArray *pa, LONG x, LONG y)
{
  return(pa->Pixels + pa->BytesPerPixel*x + pa->BytesPerRow*y);
}

static inline void VFlushPixelArray(struct VPixelArray *pa)
{
#if defined(POWERUP) && defined(__PPC)
  PPCCacheFlush(pa, pa->Size);

  if(pa->PixelsSize)
    PPCCacheFlush(pa->Pixels, pa->PixelsSize);
#endif
}

static inline void VInvalidPixelArray(struct VPixelArray *pa)
{
#if defined(POWERUP) && !defined(__PPC)
  PPCCacheInvalidE(pa, pa->Size, CACRF_ClearD);

  if(pa->PixelsSize)
    PPCCacheInvalidE(pa->Pixels, pa->PixelsSize, CACRF_ClearD);
#endif
}
#endif
