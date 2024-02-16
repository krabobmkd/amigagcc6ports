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


extern "C" {
#include <exec/types.h>
#include <exec/memory.h>
//#include <dos/dos.h>
#include <dos/dosextens.h>
#include <libraries/lowlevel.h>
}


#include "mame.h"
#include "driver.h"
#include "osdepend.h"

#include "main.h"
#include "config_moo.h"
#include "file.h"
#include "audio.h"
// from mame since 0.37:
#include "input.h"

#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>

using namespace std;
// implement parts of osdepend.h
/******************************************************************************

  Joystick & Mouse/Trackball

******************************************************************************/

/*
  return a list of all available joystick inputs (see input.h)
*/
const struct JoystickInfo *osd_get_joy_list(void)
{   /*
    struct JoystickInfo
    {
        char *name;  OS dependant name; 0 terminates the list
        unsigned code;  OS dependant code
        InputCode standardcode;	/CODE_xxx equivalent from list below, or CODE_OTHER if n/a
    };*/
    /*
    	JOYCODE_1_LEFT,JOYCODE_1_RIGHT,JOYCODE_1_UP,JOYCODE_1_DOWN,
	JOYCODE_1_BUTTON1,JOYCODE_1_BUTTON2,JOYCODE_1_BUTTON3,
	JOYCODE_1_BUTTON4,JOYCODE_1_BUTTON5,JOYCODE_1_BUTTON6,
    */

    /* lewlevel
    /* Port types
#define JP_TYPE_NOTAVAIL  (00<<28)	  /* port data unavailable
#define JP_TYPE_GAMECTLR  (01<<28)	  /* port has game controller
#define JP_TYPE_MOUSE	  (02<<28)	  /* port has mouse
#define JP_TYPE_JOYSTK	  (03<<28)	  /* port has joystick
#define JP_TYPE_UNKNOWN   (04<<28)	  /* port has unknown device
#define JP_TYPE_MASK	  (15<<28)	  /* controller type

/* Button types, valid for all types except JP_TYPE_NOTAVAIL
#define JPB_BUTTON_BLUE    23	  /* Blue - Stop; Right Mouse
#define JPB_BUTTON_RED	   22	  /* Red - Select; Left Mouse; Joystick Fire
#define JPB_BUTTON_YELLOW  21	  /* Yellow - Repeat
#define JPB_BUTTON_GREEN   20	  /* Green - Shuffle
#define JPB_BUTTON_FORWARD 19	  /* Charcoal - Forward
#define JPB_BUTTON_REVERSE 18	  /* Charcoal - Reverse
#define JPB_BUTTON_PLAY    17	  /* Grey - Play/Pause; Middle Mouse
#define JPF_BUTTON_BLUE    (1<<JPB_BUTTON_BLUE)
#define JPF_BUTTON_RED	   (1<<JPB_BUTTON_RED)
#define JPF_BUTTON_YELLOW  (1<<JPB_BUTTON_YELLOW)
#define JPF_BUTTON_GREEN   (1<<JPB_BUTTON_GREEN)
#define JPF_BUTTON_FORWARD (1<<JPB_BUTTON_FORWARD)
#define JPF_BUTTON_REVERSE (1<<JPB_BUTTON_REVERSE)
#define JPF_BUTTON_PLAY    (1<<JPB_BUTTON_PLAY)
#define JP_BUTTON_MASK	   (JPF_BUTTON_BLUE|JPF_BUTTON_RED|JPF_BUTTON_YELLOW|JPF_BUTTON_GREEN|JPF_BUTTON_FORWARD|JPF_BUTTON_REVERSE|JPF_BUTTON_PLAY)


rawkey mode ?

#define RAWKEY_PORT0_BUTTON_BLUE	0x72
#define RAWKEY_PORT0_BUTTON_RED	0x78
#define RAWKEY_PORT0_BUTTON_YELLOW	0x77
#define RAWKEY_PORT0_BUTTON_GREEN	0x76
#define RAWKEY_PORT0_BUTTON_FORWARD	0x75
#define RAWKEY_PORT0_BUTTON_REVERSE	0x74
#define RAWKEY_PORT0_BUTTON_PLAY	0x73
#define RAWKEY_PORT0_JOY_UP		0x79
#define RAWKEY_PORT0_JOY_DOWN		0x7A
#define RAWKEY_PORT0_JOY_LEFT		0x7C
#define RAWKEY_PORT0_JOY_RIGHT		0x7B

#define RAWKEY_PORT1_BUTTON_BLUE	0x172
#define RAWKEY_PORT1_BUTTON_RED	0x178
#define RAWKEY_PORT1_BUTTON_YELLOW	0x177
#define RAWKEY_PORT1_BUTTON_GREEN	0x176
#define RAWKEY_PORT1_BUTTON_FORWARD	0x175
#define RAWKEY_PORT1_BUTTON_REVERSE	0x174
#define RAWKEY_PORT1_BUTTON_PLAY	0x173
#define RAWKEY_PORT1_JOY_UP		0x179
#define RAWKEY_PORT1_JOY_DOWN		0x17A
#define RAWKEY_PORT1_JOY_LEFT		0x17C
#define RAWKEY_PORT1_JOY_RIGHT		0x17B

#define RAWKEY_PORT2_BUTTON_BLUE	0x272
#define RAWKEY_PORT2_BUTTON_RED	0x278
#define RAWKEY_PORT2_BUTTON_YELLOW	0x277
#define RAWKEY_PORT2_BUTTON_GREEN	0x276
#define RAWKEY_PORT2_BUTTON_FORWARD	0x275
#define RAWKEY_PORT2_BUTTON_REVERSE	0x274
#define RAWKEY_PORT2_BUTTON_PLAY	0x273
#define RAWKEY_PORT2_JOY_UP		0x279
#define RAWKEY_PORT2_JOY_DOWN		0x27A
#define RAWKEY_PORT2_JOY_LEFT		0x27C
#define RAWKEY_PORT2_JOY_RIGHT		0x27B

#define RAWKEY_PORT3_BUTTON_BLUE	0x372
#define RAWKEY_PORT3_BUTTON_RED	0x378
#define RAWKEY_PORT3_BUTTON_YELLOW	0x377
#define RAWKEY_PORT3_BUTTON_GREEN	0x376
#define RAWKEY_PORT3_BUTTON_FORWARD	0x375
#define RAWKEY_PORT3_BUTTON_REVERSE	0x374
#define RAWKEY_PORT3_BUTTON_PLAY	0x373
#define RAWKEY_PORT3_JOY_UP		0x379
#define RAWKEY_PORT3_JOY_DOWN		0x37A
#define RAWKEY_PORT3_JOY_LEFT		0x37C
#define RAWKEY_PORT3_JOY_RIGHT		0x37B



    */
    static bool listinited=false;
    static std::vector<JoystickInfo> joyinfo;
    static std::vector<string> stringtable;
    if(!listinited)
    {    
        listinited = true;
        stringtable.clear();
        joyinfo.clear();

        size_t stringtable_i = 0;
        for(int j=0;j<4;j++)
        {
            stringstream ss;
            ss  << "JOYPAD"<< (j+1);
            string sjp = ss.str();

            // CD32 joypad names
           // create string names for joypad buttons
           // equivalent of mame enum order, with amiga lowlevel names.
           size_t ijoystr = stringtable.size();
           stringtable.push_back(sjp + " LEFT");
           stringtable.push_back(sjp + " RIGHT");
           stringtable.push_back(sjp + " UP");
           stringtable.push_back(sjp + " DOWN");

        // CD32 pad looking names
           stringtable.push_back(sjp + " RED");
           stringtable.push_back(sjp + " BLUE");
           stringtable.push_back(sjp + " GREEN");
           stringtable.push_back(sjp + " YELLOW");

           stringtable.push_back(sjp + " REVERSE");
           stringtable.push_back(sjp + " FORWARD");
           stringtable.push_back(sjp + " PLAY");

            static const uint32_t mamecodeforjoystart[4] = {
            JOYCODE_1_LEFT,JOYCODE_2_LEFT,JOYCODE_3_LEFT,JOYCODE_4_LEFT
            };
/* we keep order:
  JOYCODE_1_LEFT,JOYCODE_1_RIGHT,JOYCODE_1_UP,JOYCODE_1_DOWN,
	JOYCODE_1_BUTTON1,JOYCODE_1_BUTTON2,JOYCODE_1_BUTTON3,
	JOYCODE_1_BUTTON4,JOYCODE_1_BUTTON5,JOYCODE_1_BUTTON6,
*/
            uint32_t mamejoystart = mamecodeforjoystart[j];
            uint32_t iport=(uint32_t)(j<<8);
            std::vector<JoystickInfo> ji=
            {
                // in mame enum order
                {(char*)stringtable[ijoystr+0].c_str(),iport+RAWKEY_PORT0_JOY_LEFT,mamejoystart+0},
                {(char*)stringtable[ijoystr+1].c_str(),iport+RAWKEY_PORT0_JOY_RIGHT,mamejoystart+1},
                {(char*)stringtable[ijoystr+2].c_str(),iport+RAWKEY_PORT0_JOY_UP,mamejoystart+2},
                {(char*)stringtable[ijoystr+3].c_str(),iport+RAWKEY_PORT0_JOY_DOWN,mamejoystart+3},
                //mapped to JOYCODE_x_BUTTON1 ... JOYCODE_x_BUTTON6
                {(char*)stringtable[ijoystr+4].c_str(),iport+RAWKEY_PORT0_BUTTON_RED,mamejoystart+4},
                {(char*)stringtable[ijoystr+5].c_str(),iport+RAWKEY_PORT0_BUTTON_BLUE,mamejoystart+5},
                {(char*)stringtable[ijoystr+6].c_str(),iport+RAWKEY_PORT0_BUTTON_GREEN,mamejoystart+6},
                {(char*)stringtable[ijoystr+7].c_str(),iport+RAWKEY_PORT0_BUTTON_YELLOW,mamejoystart+7},
                {(char*)stringtable[ijoystr+8].c_str(),iport+RAWKEY_PORT0_BUTTON_REVERSE,mamejoystart+8},
                {(char*)stringtable[ijoystr+9].c_str(),iport+RAWKEY_PORT0_BUTTON_FORWARD,mamejoystart+9},
                // +
                {(char*)stringtable[ijoystr+10].c_str(),iport+RAWKEY_PORT0_BUTTON_PLAY,CODE_OTHER},
// JOYCODE_1_LEFT
            };
            joyinfo.insert(joyinfo.end(),ji.begin(),ji.end());
        }


        // terminate list
        joyinfo.push_back({NULL,0,0});
    }

    return joyinfo.data();
}

/*
  tell whether the specified joystick direction/button is pressed or not.
  joycode is the OS dependant code specified in the list returned by
  osd_get_joy_list().
*/
int osd_is_joy_pressed(int joycode)
{

    return 0;
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
  IPort *Port2 = &(Inputs->Ports[0]);

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

