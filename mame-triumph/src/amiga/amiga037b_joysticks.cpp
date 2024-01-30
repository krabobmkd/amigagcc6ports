/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id: amiga.c,v 1.1 1999/04/28 18:50:15 meh Exp meh $
 *
 * $Log: amiga.c,v $
 * Revision 1.1  1999/04/28 18:50:15  meh
 * Initial revision
 *
 *
 *************************************************************************/

#include <stdio.h>

#include <exec/types.h>
#include <exec/memory.h>
//#include <dos/dos.h>
#include <dos/dosextens.h>

#include "mame.h"
#include "driver.h"
#include "osdepend.h"

#include "main.h"
#include "config_v37.h"
#include "file.h"
#include "audio.h"
// from mame since 0.37:
#include "input.h"
// implement parts of osdepend.h
/******************************************************************************

  Joystick & Mouse/Trackball

******************************************************************************/

struct JoystickInfo joyinfo[4];
/*
  return a list of all available joystick inputs (see input.h)
*/
const struct JoystickInfo *osd_get_joy_list(void)
{


}

/*
  tell whether the specified joystick direction/button is pressed or not.
  joycode is the OS dependant code specified in the list returned by
  osd_get_joy_list().
*/
int osd_is_joy_pressed(int joycode)
{


}

/* We support 4 players for each analog control */
//#define OSD_MAX_JOY_ANALOG	4
//#define X_AXIS          1
//#define Y_AXIS          2

void osd_poll_joysticks(void)
{

}

/* Joystick calibration routines BW 19981216 */
/* Do we need to calibrate the joystick at all? */
int osd_joystick_needs_calibration (void)
{
    return 0;
}
/* Preprocessing for joystick calibration. Returns 0 on success */
void osd_joystick_start_calibration (void)
{

}
/* Prepare the next calibration step. Return a description of this step. */
/* (e.g. "move to upper left") */
char *osd_joystick_calibrate_next (void)
{
    return NULL;
}
/* Get the actual joystick calibration data for the current position */
void osd_joystick_calibrate (void)
{

}
/* Postprocessing (e.g. saving joystick data to config) */
void osd_joystick_end_calibration (void)
{

}

void osd_trak_read(int player, int *deltax, int *deltay)
{
  if((player == 0) && (Port2->Type == IPT_MOUSE))
  {
    *deltax           = Port2->Move.Mouse.MouseX;
    Port2->Move.Mouse.MouseX  = 0;
    *deltay           = Port2->Move.Mouse.MouseY;
    Port2->Move.Mouse.MouseY  = 0;
  }
  else
  {
    *deltax = 0;
    *deltay = 0;
  }
}

/* return values in the range -128 .. 128 (yes, 128, not 127) */
void osd_analogjoy_read(int player,int *analog_x, int *analog_y)
{
    *analog_x = 0;
    *analog_y = 0;
}


/*
  inptport.c defines some general purpose defaults for key and joystick bindings.
  They may be further adjusted by the OS dependant code to better match the
  available keyboard, e.g. one could map pause to the Pause key instead of P, or
  snapshot to PrtScr instead of F12. Of course the user can further change the
  settings to anything he/she likes.
  This function is called on startup, before reading the configuration from disk.
  Scan the list, and change the keys/joysticks you want.
*/
void osd_customize_inputport_defaults(struct ipd *defaults)
{

}

