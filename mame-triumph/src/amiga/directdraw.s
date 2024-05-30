;struct directDrawScreen {
;    void *_base;
;    ULONG _bpr;
;    WORD _clipX1,_clipY1,_clipX2,_clipY2;
;};
;struct directDrawSource {
;    void *_base;
;    ULONG _bpr;
;    WORD _x1,_y1,_width,_height; // to be drawn.
;};
;// for any of the 8 16b target mode?
;extern void directDrawClut16(register directDrawScreen *screen __asm("a0"),
;                register directDrawSource *source __asm("a1"),
;                register UBYTE *lut __asm("a2"), // actually UWORD* or anywhat.
;            );
;	incdir	include:
;	include	graphics/gfx.i
	include exec/types.i

	STRUCTURE directDrawScreen,0
		APTR	dsc_base
		ULONG	dsc_bpr
		WORD	dsc_clipX1
		WORD	dsc_clipY1
		WORD	dsc_clipX2
		WORD	dsc_clipY2
	LABEL	dsc_sizeof

	STRUCTURE directDrawSource,0
		APTR	dso_base
		ULONG	dso_bpr
		WORD	dso_x1
		WORD	dso_y1
		WORD	dso_width
		WORD	dso_height
	LABEL	dso_sizeof

	XDEF	directDrawClut16
	XDEF	_directDrawClut16



	section	code,code

_directDrawClut16:
directDrawClut16:
	movem.l	d2-d7/a2-a6,-(sp)




	movem.l	(sp)+,d2-d7/a2-a6
	rts
