#include <stdio.h>

#include "cgxhooks_re.h"

#include <proto/exec.h>

void  FreeCGXHook(struct CGXHook *hook )
{
        printf("FreeCGXHook\n");
    if(hook) FreeVec(hook);
}

struct CGXHook *  AllocCLUT8RemapHook(struct Screen *scr,
											  ULONG *palette)
{
    struct CGXHook *h = (struct CGXHook *) AllocVec(sizeof(struct CGXHook), MEMF_CLEAR);
        printf("AllocCLUT8RemapHook\n");
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

void  DoCLUT8RemapHook(struct CGXHook *hook,
							   UBYTE *chunky, struct RastPort *rp ,
							   LONG srcx, LONG srcy ,
							   LONG dstx , LONG dsty ,
							   LONG width , LONG height ,
							   LONG srcmod )
{
    // printf("DoCLUT8RemapHook\n");
}

void  CustomRemapCLUT8RemapHook(struct CGXHook *hook,
										ULONG *remaptable )
{
    printf("CustomRemapCLUT8RemapHook\n");
}

