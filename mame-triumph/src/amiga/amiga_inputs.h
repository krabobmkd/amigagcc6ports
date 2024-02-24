#ifndef AMIGA_INPUTS_H
#define AMIGA_INPUTS_H
/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id: inputs.h,v 1.1 1999/04/28 18:56:13 meh Exp meh $
 *
 * $Log: inputs.h,v $
 * Revision 1.1  1999/04/28 18:56:13  meh
 * Initial revision
 *
 *
 *************************************************************************/

//#include <macros.h>
extern "C" {
#include <exec/types.h>
#include <utility/tagitem.h>
#include <devices/inputevent.h>
#include <devices/gameport.h>
}
#define IKEY_RAW    0x1000
#define IKEY_RAWMASK  0x7f
#define IKEY_NONE   0x2000

#define IKEY_TAB       (IKEY_RAW|0x42)
#define IKEY_CONTROL   (IKEY_RAW|0x63)
#define IKEY_LALT      (IKEY_RAW|0x64)
#define IKEY_RALT      (IKEY_RAW|0x65)
#define IKEY_UP        (IKEY_RAW|0x4c)
#define IKEY_DOWN      (IKEY_RAW|0x4d)
#define IKEY_LEFT      (IKEY_RAW|0x4f)
#define IKEY_RIGHT     (IKEY_RAW|0x4e)
#define IKEY_1         (IKEY_RAW|0x01)
#define IKEY_2         (IKEY_RAW|0x02)
#define IKEY_3         (IKEY_RAW|0x03)
#define IKEY_P         (IKEY_RAW|0x19)
#define IKEY_F1        (IKEY_RAW|0x50)
#define IKEY_F2        (IKEY_RAW|0x51)
#define IKEY_F3        (IKEY_RAW|0x52)
#define IKEY_F4        (IKEY_RAW|0x53)
#define IKEY_F5        (IKEY_RAW|0x54)
#define IKEY_F6        (IKEY_RAW|0x55)
#define IKEY_F7        (IKEY_RAW|0x56)
#define IKEY_F8        (IKEY_RAW|0x57)
#define IKEY_F9        (IKEY_RAW|0x58)
#define IKEY_F10       (IKEY_RAW|0x59)
#define IKEY_ESC       (IKEY_RAW|0x45)
#define IKEY_BACKSPACE (IKEY_RAW|0x41)
#define IKEY_ENTER     (IKEY_RAW|0x44)
#define IKEY_LSHIFT    (IKEY_RAW|0x60)
#define IKEY_RSHIFT    (IKEY_RAW|0x61)
#define IKEY_CAPSLOCK  (IKEY_RAW|0x62)
#define IKEY_MINUS_PAD (IKEY_RAW|0x4a)
#define IKEY_5_PAD     (IKEY_RAW|0x2e)
#define IKEY_PLUS_PAD  (IKEY_RAW|0x5e)
#define IKEY_DEL       (IKEY_RAW|0x46)

//olde
//struct IKeyMap
//{
//  ULONG Code;
//  ULONG Key;
//};

struct IPort
{
  LONG Type;

  BYTE Red;
  BYTE Blue;
  BYTE Green;
  BYTE Yellow;
  BYTE Forward;
  BYTE Reverse;
  BYTE Play;

  union
  {
    struct
    {
      BYTE  Left;
      BYTE  Right;
      BYTE  Up;
      BYTE  Down;
    } Joystick;
    struct
    {
      LONG  PrevX;
      LONG  PrevY;
      LONG  MouseX;
      LONG  MouseY;
    } Mouse;
  } Move;

    // - - - -  -private
  LONG  RealRed;

  ULONG AutoFireTime;
  ULONG BlueEmuTime;

  struct MsgPort *MsgPort;

  struct IOStdReq    *PortRequest;
  struct timerequest *TimerRequest;

  LONG PortRequestActive;
  LONG TimerRequestActive;

  struct InputEvent InputEvent;

  struct GamePortTrigger OldTrigger;
  UBYTE OldType;

  BYTE Enabled;

};

struct Inputs
{
  ULONG        SignalMask;
  //struct IPort *Ports[4];
  struct    IPort Ports[4];

  BYTE         Keys[128]; // now actual rawkeys

    // - - - - -private
  struct Library  *LowLevelBase;
  struct Window   *Window;
  struct Hook     *RefreshHook;
  struct Hook     *MenuHook;
  struct Hook     *IDCMPHook;
  //olde UBYTE           RawKeys[128]; // 128,  masked with IKEY_RAWMASK
  struct IOStdReq InputRequest;

  BYTE      Enabled;

};

#define IPT_NONE     0
#define IPT_JOYSTICK 1
#define IPT_JOYPAD   2
#define IPT_MOUSE    3

#define IA_Port1          (TAG_USER)
#define IA_Port2          (TAG_USER+1)
#define IA_Port3          (TAG_USER+2)
#define IA_Port4          (TAG_USER+3)
//olde #define IA_KeyMap         (TAG_USER+4)
#define IA_AutoFireRate (TAG_USER+5)
//#define IA_P2AutoFireRate (TAG_USER+6)
#define IA_BlueEmuTime  (TAG_USER+7)
//#define IA_P2BlueEmuTime  (TAG_USER+8)
#define IA_Window         (TAG_USER+9)
#define IA_RefreshHook    (TAG_USER+10)
#define IA_MenuHook       (TAG_USER+11)
#define IA_UseTicks       (TAG_USER+12)
#define IA_IDCMPHook      (TAG_USER+13)

struct Inputs *AllocInputs(Tag tags,...);
void FreeInputs(struct Inputs *inputs);
void IEnable(struct Inputs *inputs);
void IDisable(struct Inputs *inputs);
void IUpdate(struct Inputs *inputs);

#endif
