//#include <stdio.h>

#include "cgxhooks.h"
#include <proto/cybergraphics.h>
#include <proto/exec.h>

#include <cybergraphx/cybergraphics.h>

extern struct Library       *CyberGfxBase;

void  FreeCGXHook(struct CGXHook *hook )
{
    //printf("FreeCGXHook\n");
    if(hook) FreeVec(hook);
}

static int rgb16entry()
{
    return 0;
}
static int rgb16subentry()
{
    return 0;
}

struct CGXHook *  AllocRGB16RemapHook(struct RastPort *rp )
{
    // test bitmap is cgx
    if( !CyberGfxBase ||
        !rp ||
        !rp->BitMap ||
        !GetCyberMapAttr(rp->BitMap,CYBRMATTR_ISCYBERGFX)) return NULL;

    struct CGXHook *h = (struct CGXHook *) AllocVec(sizeof(struct CGXHook), MEMF_CLEAR);
    if(!h) return NULL;
//    printf("AllocRGB16RemapHook\n");

    h->PixFmt = GetCyberMapAttr(rp->BitMap,CYBRMATTR_PIXFMT));
    h->BytesPerPixel = GetCyberMapAttr(rp->BitMap,CYBRMATTR_BPPIX));
        /* struct Hook
    struct MinNode h_MinNode;
    ULONG	   (*h_Entry)();	 assembler entry point
    ULONG	   (*h_SubEntry)();	 often HLL entry point
    APTR	   h_Data;		 owner specific
        */
   h->Hook.h_Entry = rgb16entry;
   h->Hook.h_SubEntry = rgb16subentry;
    /*
    struct Hook			Hook;
	ULONG				Type;
	struct Library		*SysBase;
	struct Library		*GfxBase;
	struct Library		*LayersBase;
	struct Library		*CyberGfxBase;
	ULONG				BytesPerPixel;
	ULONG				PixFmt;
	struct Screen		*Screen;
	UBYTE				*Source;
	ULONG				Data;
	ULONG				SrcMod;
	ULONG				SrcX;
	ULONG				SrcY;
	ULONG				DstX;
	ULONG				DstY;
	ULONG				Remap[256];
*/


    return h;
}

void  DoRGB16Hook(struct CGXHook *hook,
							   UBYTE *chunky, struct RastPort *rp ,
							   LONG srcx, LONG srcy ,
							   LONG dstx , LONG dsty ,
							   LONG width , LONG height ,
							   LONG srcmod )
{
    // printf("DoCLUT8RemapHook\n");
}

