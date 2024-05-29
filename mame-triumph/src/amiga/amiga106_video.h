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

/** bas virtual */
class MameDisplay
{
public:
    MameDisplay();
    virtual ~MameDisplay();
    virtual void draw(_mame_display *pmame_display) = 0;
    virtual void openWindow() = 0;
    virtual void closeWindow()= 0;
    virtual void openScreen()= 0;
    virtual void closeScreen()= 0;
    inline MsgPort *userPort() { return _pUserPort; }
protected:
    MsgPort *_pUserPort;
};

/** virtual, at this level manage screen and window opening, not rendering */
class Display_Intuition : public MameDisplay
{
public:
    Display_Intuition(const _osd_create_params *params);
    ~Display_Intuition();
    void openWindow() override;
    void closeWindow() override;
    void openScreen() override;
    void closeScreen() override;
//    void draw(_mame_display *pmame_display) override;
protected:
    Screen *_pScreen;
    Window *_pScreenWindow;
    ULONG   _ScreenModeId;
    int _fullscreenWidth; // guessed from modeid.
    int _fullscreenHeight;
    ULONG _pixelFmt,_pixelbytes;
    void *_pMouseRaster;
    // - -
    Window *_pWbWindow;

    int _machineWidth,_machineHeight;
};
class Display_CGX_Paletted : public Display_Intuition
{
public:
    Display_CGX_Paletted(const _osd_create_params *params, ULONG forcedModeID=~0);
    ~Display_CGX_Paletted();
    void draw(_mame_display *pmame_display) override;
protected:
    UBYTE *_clut;
    void updatePaletteRemap(_mame_display *pmame_display);
};
class Display_CGX_TrueColor : public Display_Intuition
{
public:
    Display_CGX_TrueColor(const _osd_create_params *params, ULONG forcedModeID=~0);
    ~Display_CGX_TrueColor();
    void draw(_mame_display *pmame_display) override;
protected:

};
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
