#ifndef AMIGA_INPUTS_H
#define AMIGA_INPUTS_H

extern "C" {
    #include <exec/types.h>
    #include <utility/tagitem.h>
    #include <devices/inputevent.h>
    #include <devices/gameport.h>
}

void InitLowLevelLib();
void CloseLowLevelLib();

void AllocInputs();
void FreeInputs();
void UpdateInputs(struct MsgPort *pMsgPort);

// used by config ports
#define IPT_NONE     0
#define IPT_JOYSTICK 1
#define IPT_JOYPAD   2
#define IPT_MOUSE    3

#endif
