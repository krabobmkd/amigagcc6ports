#ifndef CGXHOOKS_H
#define CGXHOOKS_H

#define	CGXHOOK_WRITECLUT8		0
#define	CGXHOOK_COLORIMPOSE		1
#define	CGXHOOK_BITMAPIMPOSE	2

struct CGXHookMsg
{
	struct Layer    *Layer;
	struct Rectangle Bounds;
	LONG             OffsetX;
	LONG             OffsetY;
};

struct CGXHook
{
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
};

#ifdef __GNUC__
#include <macros.h>

void __saveds FreeCGXHook(struct CGXHook *hook __asm("a0"));

struct CGXHook * __saveds AllocCLUT8RemapHook(struct Screen *scr __asm("a0"),
											  ULONG *palette __asm("a1"));

void __saveds DoCLUT8RemapHook(struct CGXHook *hook __asm("a0"),
							   UBYTE *chunky __asm("a1"), struct RastPort *rp __asm("a2"),
							   LONG srcx __asm("d0"), LONG srcy __asm("d1"),
							   LONG dstx __asm("d2"), LONG dsty __asm("d3"),
							   LONG width __asm("d4"), LONG height __asm("d5"),
							   LONG srcmod __asm("d6"));

void __saveds PaletteCLUT8RemapHook(struct CGXHook *hook __asm("a0"),
									ULONG *palette __asm("a1"));

void __saveds CustomRemapCLUT8RemapHook(struct CGXHook *hook __asm("a0"),
										ULONG *remaptable __asm("a1"));

struct CGXHook * __saveds AllocColorImposeHook(struct RastPort *rp __asm("a0"));

void __saveds DoColorImposeHook(struct CGXHook *hook __asm("a0"),
								struct RastPort *rp __asm("a1"),
								LONG minx __asm("d0"), LONG miny __asm("d1"),
								LONG maxx __asm("d2"), LONG maxy __asm("d3"),
								ULONG color __asm("d4"));

struct CGXHook * __saveds AllocBitMapImposeHook(struct RastPort *rp __asm("a0"));

void __saveds DoBitMapImposeHook(struct CGXHook *hook __asm("a0"),
								 struct BitMap *src1 __asm("a1"),
								 struct BitMap *src2 __asm("a2"),
								 struct RastPort *rp __asm("a3"),
								 ULONG src1x __asm("d0"), ULONG src1y __asm("d1"),
								 ULONG src2x __asm("d2"), ULONG src2y __asm("d3"),
								 LONG dstx __asm("d4"), LONG dsty __asm("d5"),
								 ULONG width __asm("d6"), ULONG height __asm("d7"));

#else

void __saveds __asm FreeCGXHook(register __a0 struct CGXHook *hook);

struct CGXHook * __saveds __asm AllocCLUT8RemapHook(register __a0 struct Screen *scr,
													register __a1 ULONG *palette);

void __saveds __asm DoCLUT8RemapHook(register __a0 struct CGXHook *hook,
									register __a1 UBYTE *chunky, register __a2 struct RastPort *rp,
									register __d0 LONG srcx, register __d1 LONG srcy,
									register __d2 LONG dstx, register __d3 LONG dsty,
									register __d4 LONG width, register __d5 LONG height,
									register __d6 LONG srcmod );

void __saveds __asm PaletteCLUT8RemapHook(	register __a0 struct CGXHook *hook,
											register __a1 ULONG *palette);

void __saveds __asm CustomRemapCLUT8RemapHook(	register __a0 struct CGXHook *hook,
											register __a1 ULONG *remaptable);

struct CGXHook * __saveds __asm AllocColorImposeHook(register __a0 struct RastPort *rp);

void __saveds __asm DoColorImposeHook(register __a0 struct CGXHook *hook,
									register __a1 struct RastPort *rp,
									register __d0 LONG minx, register __d1 LONG miny,
									register __d2 LONG maxx, register __d3 LONG maxy,
									register __d4 ULONG color);

struct CGXHook * __saveds __asm AllocBitMapImposeHook(register __a0 struct RastPort *rp);

void __saveds __asm DoBitMapImposeHook(register __a0 struct CGXHook *hook,
									register __a1 struct BitMap *src1,
									register __a2 struct BitMap *src2,
									register __a3 struct RastPort *rp,
									register __d0 ULONG src1x, register __d1 ULONG src1y,
									register __d2 ULONG src2x, register __d3 ULONG src2y,
									register __d4 LONG dstx, register __d5 LONG dsty,
									register __d6 ULONG width, register __d7 ULONG height);

#endif

#endif
