; phxass I=ii: M=68020 cgxhooks_re.s
	include exec/types.i
	include utility/hooks.i

;struct CGXHook
;{
;	struct Hook			Hook;
;	ULONG				Type;
;	struct Library		*SysBase;
;	struct Library		*GfxBase;
;	struct Library		*LayersBase;
;	struct Library		*CyberGfxBase;
;	ULONG				BytesPerPixel;
;	ULONG				PixFmt;
;	struct Screen		*Screen;
;	UBYTE				*Source;
;	ULONG				Data;
;	ULONG				SrcMod;
;	ULONG				SrcX;
;	ULONG				SrcY;
;	ULONG				DstX;
;	ULONG				DstY;
;	ULONG				Remap[256];
;};

; "corrected " version
;struct CGXHook
;{
;	struct Hook			Hook;  // sizeof=20=$14
;	ULONG				Type;   // $14
;	struct Library		*SysBase; // $18
;	struct Library		*GfxBase; // $1c
;	struct Library		*LayersBase; // $20
;	struct Library		*CyberGfxBase; // $24
;	ULONG				BytesPerPixel; // $28  filled by _AllocCGXHook
;	ULONG				PixFmt; // $2c  filled by _AllocCGXHook
;	struct Screen		*Screen; // $30
;    // looks OK til here

;    // to be verified:
;    UBYTE				*Source; //$34
;	ULONG				Data; // $38
;	ULONG				SrcMod; // $3c
;	ULONG				SrcX; // $40
;	ULONG				SrcY; // $44
;	ULONG				DstX; // $48
;	ULONG				DstY; // $4c
;    // changed by krb against asm code:
;    // $50
;    // obvious missing 24 bytes
;    UBYTE               COMPLETEME[20];
;    APTR                TABLEPOINTER; // $64, point $68 at start

;    // $68
;	ULONG				Remap[256]; // $50   struct size: $450

;    // libs does alloc #$00000468
;    // so there is $18 bytes more.

;};


;STRUCTURE HOOK,MLN_SIZE
; APTR h_Entry		; assembler entry point
; APTR h_SubEntry		; optional HLL entry point
; APTR h_Data		; owner specific
;LABEL h_SIZEOF

 STRUCTURE CGXHook,h_SIZEOF ; extends Hooks
	ULONG	cgh_Type			; $14
	APTR	cgh_SysBase			; $18
	APTR	cgh_GfxBase			; $1c
	APTR	cgh_LayersBase		; $20
	APTR	cgh_CyberGfxBase	; $24
	ULONG	cgh_BytesPerPixel	; $28
	ULONG	cgh_PixFmt			; $2c
	APTR	cgh_Screen  ; struct Screen   $30
	APTR	cgh_Source ; UBYTE *
	; following is to be validated
	ULONG	cgh_Data
	ULONG	cgh_SrcMod
	ULONG	cgh_SrcX
	ULONG	cgh_SrcY
	ULONG	cgh_DstX
	ULONG	cgh_DstY
	; absolutely wrong on the C header side...
	STRUCT	cgh_xxxxx,20
	APTR	cgh_guessme
	STRUCT	cgh_Remap,4*256

 ; should be #$00000468
 LABEL	cgh_SIZEOF

	SECTION "",CODE

	XDEF	_AllocCLUT8RemapHook
	XDEF	_FreeCGXHook

	; used by _AllocCLUT8RemapHook
_AllocCGXHook:
	; a0: probably screen->Bitmap $54(screen)
	MOVEM.L	A3/A5/A6,-(A7)
	MOVEA.L	A0,A5
	MOVEA.W	#$0004,A0
	MOVEA.L	(A0),A1  ; a1 execbase
	MOVE.L	#$00000468,D0   ; size cgxhooks struct + clut table at end.
	MOVEQ	#$01,D1
	SWAP	D1		; memf_clear
	MOVEA.L	A1,A6
	JSR	-$00C6(A6)	; alloc, prob. Alloc() not AllocVec()
	MOVEA.L	D0,A3  ; struct in A3
	MOVE.L	A3,D0
	BEQ.W	L00000E
	MOVEA.W	#$0004,A0
	MOVEA.L	(A0),A1
	MOVE.L	A1,$0018(A3) ; sysbase in struct
	LEA	L000010(PC),A1	; "graphics.library"
	MOVEA.L	$0018(A3),A6
	MOVEQ	#$27,D0  ; ver 39
	JSR	-$0228(A6)
	MOVE.L	D0,$001C(A3)  ; GfxBase in struct
	BEQ.W	L00000D
	LEA	L000011(PC),A1
	MOVEA.L	$0018(A3),A6
	MOVEQ	#$27,D0
	JSR	-$0228(A6)
	MOVE.L	D0,$0020(A3)
	BEQ.B	L00000C
	LEA	L000012(PC),A1
	MOVEA.L	$0018(A3),A6
	MOVEQ	#$28,D0
	JSR	-$0228(A6)
	MOVE.L	D0,$0024(A3)
	BEQ.B	L00000B   ; cybergfx fail
	MOVEA.L	$0004(A5),A0    ; a5 screen rastport or bitmap
	MOVE.L	#$80000008,D0
	MOVEA.L	$0024(A3),A6   ; cybergfx base
	JSR	-$0060(A6)   ; value = GetCyberMapAttr( a0 BitMap, d0 Attribute );
	; #define CYBRMATTR_ISCYBERGFX  (0x80000008) /* returns -1 if supplied bitmap is a cybergfx one */
	TST.L	D0
	BEQ.B	L00000A ; error if screen not cgx
	MOVEA.L	$0004(A5),A0
	MOVE.L	#$80000004,D0  ; CYBRMATTR_PIXFMT return the pixel format
	JSR	-$0060(A6)
	MOVE.L	D0,$002C(A3)
	MOVEA.L	$0004(A5),A0
	MOVEA.L	$0024(A3),A6
	MOVE.L	#$80000002,D0 ; CYBRMATTR_BPPIX    bytes per pixel
	JSR	-$0060(A6)
	MOVE.L	D0,$0028(A3)
	MOVE.L	A3,D0
	BRA.B	L00000F
L00000A:
	MOVEA.L	$0024(A3),A1
	MOVEA.L	$0018(A3),A6
	JSR	-$019E(A6)
L00000B:   ; cybergraphics fail
	MOVEA.L	$0020(A3),A1
	MOVEA.L	$0018(A3),A6
	JSR	-$019E(A6)
L00000C:
	MOVEA.L	$001C(A3),A1
	MOVEA.L	$0018(A3),A6
	JSR	-$019E(A6)
L00000D:
	MOVEA.L	A3,A1
	MOVEA.L	$0018(A1),A6
	MOVE.L	#$00000468,D0
	JSR	-$00D2(A6)
L00000E:
	MOVEQ	#$00,D0
L00000F:
	MOVEM.L	(A7)+,A3/A5/A6
	RTS

L000010:
	dc.l	"grap"
	dc.l	"hics"
	dc.l	".lib"
	dc.l	"rary"
	dc.w	$0000
L000011:
	dc.l	"laye"
	dc.l	"rs.l"
	dc.l	"ibra"
	dc.l	$72790000	;"ry  "
L000012:
	dc.l	"cybe"
	dc.l	"rgra"
	dc.l	"phic"
	dc.l	"s.li"
	dc.l	"brar"
	dc.w	$7900		;"y "

_FreeCGXHook:
	MOVEM.L	D7/A3/A5/A6,-(A7)
	MOVEA.L	A0,A5
	MOVE.L	$0014(A5),D0
	TST.L	D0
	BNE.B	L000014
	TST.L	$002C(A5)
	BNE.B	L000014
	LEA	$0068(A5),A3
	MOVEQ	#$00,D7
L000013:
	CMP.L	$0060(A5),D7
	BGE.B	L000014
	MOVEA.L	$0030(A5),A0
	MOVEQ	#$00,D0
	MOVE.B	(A3)+,D0
	MOVEA.L	$0030(A0),A0
	MOVEA.L	$001C(A5),A6
	JSR	-$03B4(A6)
	ADDQ.L	#1,D7
	BRA.B	L000013
L000014:
	MOVEA.L	$0024(A5),A1
	MOVEA.L	$0018(A5),A6
	JSR	-$019E(A6)
	MOVEA.L	$0020(A5),A1
	JSR	-$019E(A6)
	MOVEA.L	$001C(A5),A1
	JSR	-$019E(A6)
	MOVEA.L	A5,A1
	MOVE.L	#$00000468,D0
	JSR	-$00D2(A6)
	MOVEM.L	(A7)+,D7/A3/A5/A6
	RTS


_WriteCLUT8Hook:
	SUBA.W	#$000C,A7
	MOVEM.L	D2-D7/A2/A3/A5/A6,-(A7)
	MOVEA.L	A1,A3
	MOVEA.L	A0,A5
	MOVE.L	$000C(A3),D0
	SUB.L	$0058(A5),D0
	MOVE.L	D0,D7
	MOVE.L	$0010(A3),D0
	SUB.L	$005C(A5),D0
	MOVE.L	D0,D6
	CLR.L	-(A7)
	PEA	$002C(A7)
	MOVE.L	#$84001006,-(A7)
	PEA	$0038(A7)
	MOVE.L	#$84001007,-(A7)
	MOVEA.L	$0004(A2),A0
	MOVEA.L	$0024(A5),A6
	MOVEA.L	A7,A1
	JSR	-$00A8(A6)
	LEA	$0014(A7),A7
	MOVE.L	D0,$0030(A7)
	BEQ.W	L000015
	MOVEA.L	$000C(A5),A0
	MOVEA.L	$0038(A5),A1
	ADDA.L	D7,A1
	ADDA.L	$0048(A5),A1
	MOVE.L	D6,D0
	ADD.L	$004C(A5),D0
	MOVE.L	$0040(A5),D1
	MULU.L	D1,D0
	ADDA.L	D0,A1
	MOVE.W	$0004(A3),D0
	EXT.L	D0
	MULU.L	$0028(A5),D0
	MOVE.L	$002C(A7),D2
	ADD.L	D0,D2
	MOVE.W	$0006(A3),D0
	EXT.L	D0
	MOVE.L	$0028(A7),D3
	MULU.L	D3,D0
	ADD.L	D0,D2
	MOVE.W	$0008(A3),D0
	EXT.L	D0
	MOVE.W	$0004(A3),D4
	EXT.L	D4
	SUB.L	D4,D0
	ADDQ.L	#1,D0
	MOVE.W	$000A(A3),D4
	EXT.L	D4
	MOVE.W	$0006(A3),D5
	EXT.L	D5
	SUB.L	D5,D4
	ADDQ.L	#1,D4
	MOVEA.L	A0,A6
	MOVE.L	A2,-(A7)
	MOVE.L	D4,D1
	MOVEA.L	A1,A0
	MOVEA.L	D2,A1
	MOVE.L	$0040(A5),D2
	MOVEA.L	$0064(A5),A2
	JSR	(A6)
	MOVEA.L	(A7)+,A2
	MOVEA.L	$0030(A7),A0
	MOVEA.L	$0024(A5),A6
	JSR	-$00AE(A6)
L000015:
	MOVEM.L	(A7)+,D2-D7/A2/A3/A5/A6
	ADDA.W	#$000C,A7
	RTS

_AllocCLUT8RemapHook:
	; a0 screen
	; a1 palette (ULONG *) -> or NULL ?
	MOVEM.L	A2/A3/A5/A6,-(A7)
	MOVEA.L	A1,A3
	MOVEA.L	A0,A5
	LEA	$0054(A5),A0  ; a tout les coups rasport du screen  -> sc_RastPort ou sc_Bitmap -> plutot bitmap
	;BSR.W	L000010   ; foireux, erreur d68k, mais doit etre _AllocCGXHook
	bsr.w _AllocCGXHook

	MOVEA.L	D0,A2   ; d0: retour struct CGXHooks alloked with lib bases and bitmap infos.
	MOVE.L	A2,D0
	BEQ.W	L00001E  ; null if cgx not supported or screen not cgx.
	CLR.L	$0014(A2) ; Type
	MOVE.L	A5,$0030(A2) ; *Screen
	LEA	_WriteCLUT8Hook(PC),A0
	MOVE.L	A0,$0008(A2)  ; struct Hook h_Entry function pointer
	; palette start at $50 ?
	LEA	$0068(A2),A0 ; oO palette start at $50 -> no.
	MOVE.L	A0,$0064(A2)
	MOVE.L	$0028(A2),D0 ; BytesPerPixel (1,2,3,4)
	SUBQ.L	#1,D0
	BEQ.B	L000017
	SUBQ.L	#1,D0
L000016:
	BEQ.B	L000018
	SUBQ.L	#1,D0
	BEQ.B	L000019
	SUBQ.L	#1,D0
	BEQ.B	L00001A
	BRA.B	L00001B
L000017:	; one byte per pixel case
	;FOIREUX LEA	L00005B(PC),A0
	MOVE.L	A0,$000C(A2)  ; hook h_SubEntry
	BRA.B	L00001B
L000018:	; 2 bytes per pixel case
	LEA	L000054(PC),A0
	MOVE.L	A0,$000C(A2) ; hook h_SubEntry
	BRA.B	L00001B
L000019: ; 3 bytes ppixel ??? no way.
	LEA	L00004C(PC),A0
	MOVE.L	A0,$000C(A2) ; hook h_SubEntry
	BRA.B	L00001B
L00001A: ; 4 bytes per pixel
	LEA	L000047(PC),A0
	MOVE.L	A0,$000C(A2) ; hook h_SubEntry
L00001B: ; more then 4 bytes per pixels (???) -> no
	TST.L	$000C(A2)
	BEQ.B	L00001D
	MOVE.L	A3,D0
	BEQ.B	L00001C
	LEA	$0068(A2),A0
	MOVE.L	A2,-(A7)
	MOVEA.L	A0,A1
	MOVE.L	$002C(A2),D0
	MOVEA.L	A3,A0
	MOVEA.L	$001C(A2),A6
	MOVEA.L	$0030(A5),A2
	BSR.W	L00002F
	MOVEA.L	(A7)+,A2
	MOVE.L	D0,$0060(A2)
	BLT.B	L00001D
L00001C:
	MOVE.L	A2,D0
	BRA.B	L00001F
L00001D:
	MOVEA.L	A2,A0
	BSR.W	L000016
L00001E:
	MOVEQ	#$00,D0
L00001F:
	MOVEM.L	(A7)+,A2/A3/A5/A6
	RTS
