/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id: inputs.c,v 1.1 1999/04/28 18:56:13 meh Exp meh $
 *
 * $Log: inputs.c,v $
 * Revision 1.1  1999/04/28 18:56:13  meh
 * Initial revision
 *
 *
 *************************************************************************/

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include "intuiuncollide.h"
#include <proto/utility.h>
#include <proto/keymap.h>
#include <proto/lowlevel.h>
#include <proto/alib.h>


extern "C" {
#include <libraries/lowlevel.h>
#include <devices/inputevent.h>
#include <devices/gameport.h>
#include <devices/timer.h>
#include <devices/input.h>
#include <exec/errors.h>

#ifdef POWERUP
#include <powerup/ppclib/memory.h>
#include <inline/ppc.h>
#endif

#define LOWLEVEL_BASE_NAME inputs->LowLevelBase
#include <inline/lowlevel.h>
} // end extern c
#include <macros.h>

#include <stdio.h>

#include "amiga_inputs.h"

extern struct IntuitionBase *IntuitionBase;
extern struct Library *UtilityBase;
extern struct Library *KeymapBase;

#ifdef POWERUP
extern struct Library *PPCLibBase;

static inline APTR memAlloc(ULONG size)
{
  return(PPCAllocVec(size, MEMF_PUBLIC|MEMF_CLEAR|MEMF_NOCACHESYNCPPC|MEMF_NOCACHESYNCM68K));
}

static inline void memFree(APTR mem)
{
  PPCFreeVec(mem);
}
#else
static inline APTR memAlloc(ULONG size)
{
  return(AllocVec(size, MEMF_PUBLIC|MEMF_CLEAR));
}

static inline void memFree(APTR mem)
{
  FreeVec(mem);
}
#endif

void IAllocPort(struct Inputs *inputs, LONG portnum);
void IFreePort(struct Inputs *inputs, LONG portnum);
void IEnablePort(struct Inputs *inputs, LONG pornum);
void IDisablePort(struct Inputs *inputs, LONG portnum);
void IUpdatePort(struct Inputs *inputs, LONG portnum);
void IUpdateKeys(struct Inputs *inputs);

struct Inputs *AllocInputs(Tag tags,...)
{
  struct Inputs *inputs;
//  struct IKeyMap  *keymap;
  struct TagItem  *tag, *tstate;
  UBYTE     buf[2];
  LONG      i, ttv;
  int       use_ticks;

 printf("AllocInputs()\n");


    inputs = (struct Inputs *) AllocVec(sizeof(struct Inputs), MEMF_CLEAR);
    if(!inputs) return NULL;

//    tag = FindTagItem(IA_KeyMap, (struct TagItem *) &tags);
//    if(tag)
//      keymap  = (struct IKeyMap *) tag->ti_Data;
//    else
//      keymap  = NULL;


//    inputs->Ports[0]  = (struct IPort *) memAlloc(2*sizeof(struct IPort)+ttv+1);
//      inputs->Ports[1]  = &inputs->Ports[0][1];

      inputs->LowLevelBase  = OpenLibrary("lowlevel.library", 0);

      use_ticks = FALSE;

      tstate = (struct TagItem *) &tags;
      while((tag = NextTagItem(&tstate)))
      {
        switch(tag->ti_Tag)
        {
          case IA_Port1:
            inputs->Ports[0].Type  = tag->ti_Data;
            break;
          case IA_Port2:
            inputs->Ports[1].Type  = tag->ti_Data;
            break;
          case IA_Port3:
            inputs->Ports[2].Type  = tag->ti_Data;
            break;
          case IA_Port4:
            inputs->Ports[3].Type  = tag->ti_Data;
            break;
//          case IA_AutoFireRate:
//            if(tag->ti_Data)
//              inputs->Ports[0].AutoFireTime  = 500000 / tag->ti_Data;
//            break;
//          case IA_AutoFireRate:
//            if(tag->ti_Data)
//              inputs->Ports[1].AutoFireTime  = 500000 / tag->ti_Data;
//            break;
//          case IA_P1BlueEmuTime:
//            if(tag->ti_Data)
//              inputs->Ports[0].BlueEmuTime = 100000 * tag->ti_Data;
//            break;
//          case IA_P2BlueEmuTime:
//            if(tag->ti_Data)
//              inputs->Ports[1].BlueEmuTime = 100000 * tag->ti_Data;
//            break;
          case IA_Window:
            inputs->Window  = (struct Window *) tag->ti_Data;
            break;
          case IA_RefreshHook:
            inputs->RefreshHook = (struct Hook *) tag->ti_Data;
            break;
          case IA_MenuHook:
            inputs->MenuHook  = (struct Hook *) tag->ti_Data;
            break;
          case IA_UseTicks:
            use_ticks = TRUE;
            break;
          case IA_IDCMPHook:
            inputs->IDCMPHook = (struct Hook *) tag->ti_Data;
            break;
        }
      }

      if(inputs->Window)
      {
        if(use_ticks)
          ModifyIDCMP(inputs->Window, inputs->Window->IDCMPFlags|IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW|IDCMP_INTUITICKS);
        else
          ModifyIDCMP(inputs->Window, inputs->Window->IDCMPFlags|IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW);
      }
      
      //if(keymap)
      //{
      //no more   inputs->Keys    = (BYTE *) &inputs->Ports[0][2];

//        for(i = 0; keymap[i].Key; i++)
//        {
//          if(keymap[i].Key & IKEY_RAW)
//            inputs->RawKeys[keymap[i].Key & (~IKEY_RAW)] = keymap[i].Code;
//          else if(keymap[i].Key != IKEY_NONE)
//          {

//// LONG  __stdargs MapANSI( CONST_STRPTR string, LONG count, STRPTR buffer, LONG length, CONST struct KeyMap *keyMap );

//            if(MapANSI(((const char *) &keymap[i].Key) + 3, 1,(STRPTR)buf, 1, NULL) == 1)
//            {
//              if(!buf[1])
//                inputs->RawKeys[buf[0] & IKEY_RAWMASK] = keymap[i].Code;
//            }
//          }
//        }

        if(inputs->Window)
          ModifyIDCMP(inputs->Window, inputs->Window->IDCMPFlags|IDCMP_RAWKEY);
      //}

      if(inputs->Ports[0].Type)
      {
        if(!OpenDevice("input.device", NULL, (struct IORequest *) &inputs->InputRequest, NULL))
        {
          BYTE  dummyport = 2;

          inputs->InputRequest.io_Data    = &dummyport;
          inputs->InputRequest.io_Flags   = IOF_QUICK;
          inputs->InputRequest.io_Length    = 1;
          inputs->InputRequest.io_Command = IND_SETMPORT;
          BeginIO((struct IORequest *) &inputs->InputRequest);
        }

        IAllocPort(inputs, 0);
      }

      if(inputs->Ports[1].Type)
        IAllocPort(inputs, 1);

      if(inputs->Window)
        inputs->SignalMask  = 1<<inputs->Window->UserPort->mp_SigBit;

      if( inputs->Ports[0].MsgPort)
        inputs->SignalMask  |= 1<<inputs->Ports[0].MsgPort->mp_SigBit;

      if( inputs->Ports[1].MsgPort)
        inputs->SignalMask  |= 1<<inputs->Ports[1].MsgPort->mp_SigBit;

// v37
      if( inputs->Ports[2].MsgPort)
        inputs->SignalMask  |= 1<<inputs->Ports[2].MsgPort->mp_SigBit;
      if( inputs->Ports[3].MsgPort)
        inputs->SignalMask  |= 1<<inputs->Ports[3].MsgPort->mp_SigBit;


      inputs->Enabled = TRUE;

      return(inputs);




}

void FreeInputs(struct Inputs *inputs)
{
  if(inputs->Ports[0].Type)
    IFreePort(inputs, 0);

  if(inputs->Ports[1].Type)
    IFreePort(inputs, 1);


  if(inputs->Ports[1].Type)
    IFreePort(inputs, 1);

  if(inputs->LowLevelBase)
    CloseLibrary(inputs->LowLevelBase);

  if(inputs->InputRequest.io_Device)
  {
    BYTE  dummyport = 0;

    inputs->InputRequest.io_Data    = &dummyport;
    inputs->InputRequest.io_Flags   = IOF_QUICK;
    inputs->InputRequest.io_Length    = 1;
    inputs->InputRequest.io_Command = IND_SETMPORT;
    BeginIO((struct IORequest *) &inputs->InputRequest);
    CloseDevice((struct IORequest *) &inputs->InputRequest);
  }

  //memFree(inputs->Ports[0]);

  FreeVec(inputs);
}

void IEnable(struct Inputs *inputs)
{
  if(!inputs->Enabled && inputs->Ports[0].Type)
  {
    BYTE dummyport = 2;

    IEnablePort(inputs, 0);

    inputs->InputRequest.io_Data    = &dummyport;
    inputs->InputRequest.io_Flags   = IOF_QUICK;
    inputs->InputRequest.io_Length    = 1;
    inputs->InputRequest.io_Command   = IND_SETMPORT;
    BeginIO((struct IORequest *) &inputs->InputRequest);
  }

  inputs->Enabled = FALSE;
}

void IDisable(struct Inputs *inputs)
{
  if(inputs->Enabled && inputs->Ports[0].Type)
  {
    BYTE dummyport = 0;

    IDisablePort(inputs, 0);

    inputs->InputRequest.io_Data    = &dummyport;
    inputs->InputRequest.io_Flags   = IOF_QUICK;
    inputs->InputRequest.io_Length    = 1;
    inputs->InputRequest.io_Command   = IND_SETMPORT;
    BeginIO((struct IORequest *) &inputs->InputRequest);
  }

  inputs->Enabled = FALSE;
}

void IUpdate(struct Inputs *inputs)
{
  if(inputs->Keys && inputs->Window)
    IUpdateKeys(inputs);

  if(inputs->Ports[0].Type)
    IUpdatePort(inputs, 0);

  if(inputs->Ports[1].Type)
    IUpdatePort(inputs, 1);
}

void IUpdateKeys(struct Inputs *inputs)
{
  struct IntuiMessage *im;
  struct MenuItem   *mitem;
  ULONG       imclass;
  UWORD       imcode;
  UWORD       imqual;

  while((im = (struct IntuiMessage *) GetMsg(inputs->Window->UserPort)))
  {
    imclass = im->Class;
    imcode  = im->Code;
    imqual  = im->Qualifier;

    ReplyMsg((struct Message *) im);

    switch(imclass)
    {
      case IDCMP_RAWKEY:
        printf("idcmlp rawkey\n");
        if(!(imqual & IEQUALIFIER_REPEAT) && inputs->RawKeys[imcode & IKEY_RAWMASK])
        {
          if(imcode & IECODE_UP_PREFIX)
            inputs->Keys[inputs->RawKeys[imcode & IKEY_RAWMASK]] = 0;
          else
            inputs->Keys[inputs->RawKeys[imcode & IKEY_RAWMASK]] = 1;
        }
        break;

      case IDCMP_MENUPICK:
        if(inputs->MenuHook)
        {
          while(imcode != MENUNULL)
          {
            CallHook(inputs->MenuHook, NULL, ITEMNUM(imcode));

            mitem = ItemAddress(inputs->Window->MenuStrip, imcode);
            imcode  = mitem->NextSelect;
          }
        }
        break;

      case IDCMP_REFRESHWINDOW:
        BeginRefresh(inputs->Window);
        if(inputs->RefreshHook)
          CallHookPkt(inputs->RefreshHook, NULL, NULL);
        EndRefresh(inputs->Window, TRUE);
        break;

      case IDCMP_ACTIVEWINDOW:
        IEnable(inputs);
        break;

      case IDCMP_INACTIVEWINDOW:
        IDisable(inputs);
        break;

      default:
        if(inputs->IDCMPHook)
          CallHook(inputs->IDCMPHook, NULL, imclass);
    }
  }
}

void IAllocPort(struct Inputs *inputs, LONG portnum)
{
  struct IPort  *port;
  struct IOStdReq *io;

  port  = &inputs->Ports[portnum];
  io    = NULL;

  if(inputs->LowLevelBase)
  {
    switch(port->Type)
    {
      case IPT_JOYSTICK:
        SetJoyPortAttrs(portnum, SJA_Type, SJA_TYPE_JOYSTK, TAG_END);
        break;

      case IPT_MOUSE:
        SetJoyPortAttrs(portnum, SJA_Type, SJA_TYPE_MOUSE, TAG_END);
        break;

      case IPT_JOYPAD:
        SetJoyPortAttrs(portnum, SJA_Type, SJA_TYPE_GAMECTLR, TAG_END);
        break;
    }
  }
  else
  {
    port->MsgPort = CreateMsgPort();

    if(port->MsgPort)
    {
      io  = (struct IOStdReq *) CreateIORequest(port->MsgPort, sizeof(struct IOStdReq));

      if(io)
        OpenDevice("gameport.device", portnum, (struct IORequest *) io, 0);
    }
  }

  port->PortRequest = io;

  if(inputs->LowLevelBase || io)
  {
    if(port->AutoFireTime || port->BlueEmuTime)
    {
      if(!port->MsgPort)
        port->MsgPort = CreateMsgPort();

      if(port->MsgPort)
      {
        port->TimerRequest  = (struct timerequest *)CreateIORequest(port->MsgPort, sizeof(struct timerequest));

        if(port->TimerRequest)
          OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *) port->TimerRequest, 0);
      }
    }

    IEnablePort(inputs, portnum);
  }
}

void IFreePort(struct Inputs *inputs, LONG portnum)
{
  struct IPort  *port;

  IDisablePort(inputs, portnum);

  port  = &inputs->Ports[portnum];

  if(port->PortRequest)
  {
    if(port->PortRequest->io_Device)
      CloseDevice((struct IORequest *) port->PortRequest);

    DeleteIORequest((struct IORequest *) port->PortRequest);
  }

  if(port->TimerRequest)
  {
    CloseDevice((struct IORequest *) port->TimerRequest);
    DeleteIORequest((struct IORequest *) port->TimerRequest);
  }

  if(port->MsgPort)
    DeleteMsgPort(port->MsgPort);

  if(inputs->LowLevelBase)
    SetJoyPortAttrs(portnum, SJA_Reinitialize, NULL, TAG_END);
}

void IEnablePort(struct Inputs *inputs, LONG portnum)
{
  struct IPort  *port;
  struct IOStdReq *io;

  port  = &inputs->Ports[portnum];

  if(!port->Enabled && port->PortRequest && port->PortRequest->io_Device)
  {
    struct GamePortTrigger  trigger;
    UBYTE         type;

    if((port->Type == IPT_JOYSTICK) || (port->Type == IPT_JOYPAD))
    {
      trigger.gpt_Keys  = GPTF_UPKEYS|GPTF_DOWNKEYS;
      trigger.gpt_Timeout = 0xffff;
      trigger.gpt_XDelta  = 1;
      trigger.gpt_YDelta  = 1;
      type        = GPCT_ABSJOYSTICK;
    }
    else if(port->Type == IPT_MOUSE)
    {
      trigger.gpt_Keys  = GPTF_UPKEYS|GPTF_DOWNKEYS;
      trigger.gpt_Timeout = 0xffff;
      trigger.gpt_XDelta  = 10;
      trigger.gpt_YDelta  = 10;
      type        = GPCT_MOUSE;
    }

    io  = port->PortRequest;

    io->io_Command  = GPD_ASKCTYPE;
    io->io_Flags  = IOF_QUICK;
    io->io_Data   = (APTR) &port->OldType;
    io->io_Length = 1;
    DoIO((struct IORequest *) io);
    io->io_Command  = GPD_SETCTYPE;
    io->io_Flags  = IOF_QUICK;
    io->io_Data   = (APTR) &type;
    io->io_Length = 1;
    DoIO((struct IORequest *) io);
    io->io_Command  = GPD_ASKTRIGGER;
    io->io_Flags  = IOF_QUICK;
    io->io_Data   = (APTR) &port->OldTrigger;
    io->io_Length = sizeof(struct GamePortTrigger);
    DoIO((struct IORequest *) io);
    io->io_Command  = GPD_SETTRIGGER;
    io->io_Flags  = IOF_QUICK;
    io->io_Data   = (APTR) &trigger;
    io->io_Length = sizeof(struct GamePortTrigger);
    DoIO((struct IORequest *) io);
    io->io_Command  = CMD_CLEAR;
    io->io_Flags  = IOF_QUICK;
    io->io_Data   = NULL;
    io->io_Length = 0;
    DoIO((struct IORequest *) io);
    io->io_Command  = GPD_READEVENT;
    io->io_Flags  = 0;
    io->io_Data   = (APTR) &port->InputEvent;
    io->io_Length = sizeof(struct InputEvent);
    SendIO((struct IORequest *) io);

    port->PortRequestActive = TRUE;
  }

  port->Enabled = TRUE;
}

void IDisablePort(struct Inputs *inputs, LONG portnum)
{
  struct IPort  *port;
  struct IOStdReq *io;
  struct Message  *msg;

  port  = &inputs->Ports[portnum];

  if(port->Enabled)
  {
    io  = port->PortRequest;

    if(port->PortRequestActive)
      AbortIO((struct IORequest *) io);

    if(port->TimerRequestActive)
      AbortIO((struct IORequest *) port->TimerRequest);

    while(port->PortRequestActive || port->TimerRequestActive)
    {
      WaitPort(port->MsgPort);

      while((msg = GetMsg(port->MsgPort)))
      {
        if(msg == (struct Message *) io)
          port->PortRequestActive   = FALSE;
        else
          port->TimerRequestActive  = FALSE;
      }
    }

    if(io)
    {
      if(io->io_Device)
      {
        io->io_Command  = GPD_SETCTYPE;
        io->io_Flags  = IOF_QUICK;
        io->io_Data   = (APTR) &port->OldType;
        io->io_Length = 1;
        DoIO((struct IORequest *) io);
        io->io_Command  = GPD_SETTRIGGER;
        io->io_Flags  = IOF_QUICK;
        io->io_Data   = (APTR) &port->OldTrigger;
        io->io_Length = sizeof(struct GamePortTrigger);
        DoIO((struct IORequest *) io);
        io->io_Command  = CMD_CLEAR;
        io->io_Flags  = IOF_QUICK;
        io->io_Data   = NULL;
        io->io_Length = 0;
        DoIO((struct IORequest *) io);
      }
    }
  }

  port->Enabled = FALSE;
}

void IUpdatePort(struct Inputs *inputs, LONG portnum)
{
  struct IPort    *port;
  struct Message    *msg;
  struct InputEvent *ie;
  LONG        button;
  LONG        red;

  port  = &inputs->Ports[portnum];
  ie    = &port->InputEvent;
  red   = port->RealRed;

  if(inputs->LowLevelBase)
  {
    button  = ReadJoyPort(portnum);

    red = (button & JPF_BUTTON_RED) ? 1 : 0;

    if(!port->AutoFireTime || port->BlueEmuTime)
      port->Blue  = (button & JPF_BUTTON_BLUE) ? 1 : 0;

    port->Green   = (button & JPF_BUTTON_GREEN) ? 1 : 0;
    port->Yellow  = (button & JPF_BUTTON_YELLOW) ? 1 : 0;
    port->Forward = (button & JPF_BUTTON_FORWARD) ? 1 : 0;
    port->Reverse = (button & JPF_BUTTON_REVERSE) ? 1 : 0;
    port->Play    = (button & JPF_BUTTON_PLAY) ? 1 : 0;

    if((port->Type == IPT_JOYSTICK) || (port->Type == IPT_JOYPAD))
    {
      port->Move.Joystick.Up    = (button & JPF_JOY_UP) ? 1 : 0;
      port->Move.Joystick.Down  = (button & JPF_JOY_DOWN) ? 1 : 0;
      port->Move.Joystick.Left  = (button & JPF_JOY_LEFT) ? 1 : 0;
      port->Move.Joystick.Right = (button & JPF_JOY_RIGHT) ? 1 : 0;
    }
    else if(port->Type == IPT_MOUSE)
    {
      LONG  delta;

      delta = (button & JP_MHORZ_MASK) - port->Move.Mouse.PrevX;

      port->Move.Mouse.PrevX  = (button & JP_MHORZ_MASK);
      port->Move.Mouse.MouseX += delta;

      if(delta < -128)
        port->Move.Mouse.MouseX += 256;
      else if(delta > 127)
        port->Move.Mouse.MouseX -= 256;

      delta = ((button & JP_MVERT_MASK) >> 8) - port->Move.Mouse.PrevY;

      port->Move.Mouse.PrevY  = (button & JP_MVERT_MASK) >> 8;
      port->Move.Mouse.MouseY += delta;

      if(delta < -128)
        port->Move.Mouse.MouseY += 256;
      else if(delta > 127)
        port->Move.Mouse.MouseY -= 256;
    }
  }

  if(port->MsgPort)
  {
    while((msg  = GetMsg(port->MsgPort)))
    {
      if((msg == (struct Message *) port->PortRequest)
      && (port->PortRequest->io_Actual == sizeof(struct InputEvent)))
      {
        if(ie->ie_Code & IECODE_UP_PREFIX)
          button = 0;
        else
          button = 1;

        switch(ie->ie_Code & (~IECODE_UP_PREFIX))
        {
          case IECODE_LBUTTON:
            red     = button;
            break;
          case IECODE_RBUTTON:
            if(!port->AutoFireTime || port->BlueEmuTime)
              port->Blue  = button;
            break;
          case IECODE_MBUTTON:
            port->Green = button;
            break;
        }

        if((port->Type == IPT_JOYSTICK) || (port->Type == IPT_JOYPAD))
        {
          switch(ie->ie_X)
          {
            case 0:
              port->Move.Joystick.Left  = 0;
              port->Move.Joystick.Right = 0;
              break;
            case -1:
              port->Move.Joystick.Left  = 1;
              port->Move.Joystick.Right = 0;
              break;
            case 1:
              port->Move.Joystick.Left  = 0;
              port->Move.Joystick.Right = 1;
              break;
          }
          switch(ie->ie_Y)
          {
            case 0:
              port->Move.Joystick.Up    = 0;
              port->Move.Joystick.Down  = 0;
              break;
            case -1:
              port->Move.Joystick.Up    = 1;
              port->Move.Joystick.Down  = 0;
              break;
            case 1:
              port->Move.Joystick.Up    = 0;
              port->Move.Joystick.Down  = 1;
              break;
          }
        }
        else if(port->Type == IPT_MOUSE)
        {
          port->Move.Mouse.MouseX += ie->ie_X;
          port->Move.Mouse.MouseY += ie->ie_Y;
        }

        port->PortRequest->io_Command = GPD_READEVENT;
        port->PortRequest->io_Flags   = 0;
        port->PortRequest->io_Data    = (APTR) ie;
        port->PortRequest->io_Length  = sizeof(struct InputEvent);
        SendIO((struct IORequest *) port->PortRequest);
      }
            else if(msg == (struct Message *) port->TimerRequest)
      {
        port->TimerRequestActive  = FALSE;

        if((port->TimerRequest->tr_node.io_Error != IOERR_ABORTED) && red)
        {
          if(port->AutoFireTime)
          {
            if(port->Red)
              port->Red = 0;
            else
              port->Red = 1;

            port->TimerRequest->tr_node.io_Command  = TR_ADDREQUEST;
            port->TimerRequest->tr_node.io_Flags  = 0;
            port->TimerRequest->tr_time.tv_secs   = 0;
            port->TimerRequest->tr_time.tv_micro  = port->AutoFireTime;

            SendIO((struct IORequest *) port->TimerRequest);

            port->TimerRequestActive  = TRUE;
          }
          else
          {
            port->Red = 0;
            port->Blue  = 1;
          }
        }
      }
    }
  }

  if(port->RealRed != red)
  {
    port->Red = red;

    if(port->TimerRequest)
    {
      if(!port->AutoFireTime || port->BlueEmuTime)
        port->Blue  = 0;

      if(red)
      {
        if(port->TimerRequest && port->TimerRequest->tr_node.io_Device && !port->TimerRequestActive)
        {
          port->TimerRequest->tr_node.io_Command  = TR_ADDREQUEST;
          port->TimerRequest->tr_node.io_Flags  = 0;
          port->TimerRequest->tr_time.tv_secs   = 0;

          if(port->AutoFireTime)
            port->TimerRequest->tr_time.tv_micro  = port->AutoFireTime;
          else
            port->TimerRequest->tr_time.tv_micro  = port->BlueEmuTime;

          SendIO((struct IORequest *) port->TimerRequest);

          port->TimerRequestActive  = TRUE;
        }
      }
      else
      {
        if(port->TimerRequestActive)
          AbortIO((struct IORequest *) port->TimerRequest);
      }
    }
  }

  port->RealRed = red;
}

/*************************************************************************************************/
