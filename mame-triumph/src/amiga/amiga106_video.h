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

#endif
