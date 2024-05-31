#ifndef AMIGA_VIDEO_H
#define AMIGA_VIDEO_H

// note: anything exported from amiga_video.cpp is defined in mame/osdepends.h
#include <vector>
extern "C"
{
    #include <exec/ports.h>
}

struct _osd_create_params;
struct _mame_display;
struct Window;
struct Screen;
struct RastPort;
struct BitMap;

/** full virtual */
class MameDisplay
{
public:
    MameDisplay();
    virtual ~MameDisplay();
    virtual void open(const _osd_create_params *params,int window, ULONG forcedModeID=~0) = 0;
    virtual void close()= 0;
    virtual void draw(_mame_display *pmame_display) = 0;
    virtual int good() = 0;
    virtual MsgPort *userPort() = 0;
protected:
};
//class Paletted {
//    Paletted();
//    virtual ~Paletted();
//    virtual UBYTE *open() = 0;
//};

/* if game send color indexed bitmap, manage remap to final bitmap pixel format  */
class Paletted_CGX
{
public:
    Paletted_CGX(const _osd_create_params *params, int screenPixFmt, int bytesPerPix);
    ~Paletted_CGX();
    void updatePaletteRemap(_mame_display *display);
    int needRemap() const { return _needFirstRemap; }

    std::vector<UBYTE> _clut8;
    std::vector<USHORT> _clut16;
    std::vector<ULONG> _clut32;
protected:
    int _needFirstRemap;
    int _pixFmt,_bytesPerPix;
};

/** virtual, at this level manage screen and window opening, not rendering */

class IntuitionDrawable {
public:
    IntuitionDrawable();
    virtual ~IntuitionDrawable();
    virtual void open() = 0;
    virtual void close()= 0;
    virtual MsgPort *userPort() = 0;
    inline ULONG pixelFmt() const { return _PixelFmt; }
    inline ULONG pixelBytes() const { return _PixelBytes; }

    void drawRastPort(_mame_display *display,Paletted_CGX *pRemap);
protected:
    ULONG _PixelFmt,_PixelBytes;
    int _dx,_dy; // draw delta (for windows borders)
    virtual RastPort *rastPort() = 0;
};

class Intuition_Screen : public IntuitionDrawable
{
public:
    Intuition_Screen(const _osd_create_params *params);
    ~Intuition_Screen();
    void open() override;
    void close() override;
    MsgPort *userPort() override;
protected:
    Screen *_pScreen;
    Window *_pScreenWindow;
    ULONG   _ScreenModeId;
    int _fullscreenWidth; // guessed from modeid.
    int _fullscreenHeight;
    void *_pMouseRaster;
    RastPort *rastPort() override;
};
class Intuition_Window : public IntuitionDrawable
{
public:
    Intuition_Window(const _osd_create_params *params);
    ~Intuition_Window();
    void open() override;
    void close() override;
    MsgPort *userPort() override;
protected:
    Window *_pWbWindow;
    BitMap *_sWbWinSBitmap;
    int _machineWidth,_machineHeight;
    RastPort *rastPort() override;
};


class Display_CGX : public MameDisplay
{
public:
    Display_CGX();
    ~Display_CGX();
    void open(const _osd_create_params *params,int window, ULONG forcedModeID=~0) override;
    void close() override;
    int good() override;
    void draw(_mame_display *pmame_display) override;    
    MsgPort *userPort() override;
protected:
    IntuitionDrawable   *_drawable; // screen or window
    Paletted_CGX        *_remap;    // null if true color app.

 //   void drawRastPort(RastPort *pRPort,_mame_display *pmame_display,int dx,int dy);
};
/*
class Display_CGX_TrueColor : public Display_Intuition
{
public:
    Display_CGX_TrueColor(const _osd_create_params *params, ULONG forcedModeID=~0);
    ~Display_CGX_TrueColor();
    void draw(_mame_display *pmame_display) override;
protected:

};*/
// - - - - - - -
/*
class Display_P96 : public MameDisplay
{
public:
    Display_P96(unsigned int width,
				unsigned int height);
    ~Display_P96();
    void draw(_mame_display *pmame_display) override;
protected:
    Window *_pWindow;
};
*/

#endif
