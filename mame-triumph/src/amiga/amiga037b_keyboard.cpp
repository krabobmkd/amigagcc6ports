/**************************************************************************
 *
 * Copyright (C) 2024 Vic Ferry (http://github.com/krabobmkd)
 * forked from 1999 Mats Eirik Hansen (mats.hansen at triumph.no)
 *
 * $Id: amiga.c,v 1.1 1999/04/28 18:50:15 meh Exp meh $
 *
 * $Log: amiga.c,v $
 * Revision 1.1  1999/04/28 18:50:15  meh
 * Initial revision
 *
 *
 *************************************************************************/
// from amiga
#include <proto/intuition.h>
#include "intuiuncollide.h"

//#include <devices/keyboard.h>
//#include <devices/keymap.h>


#include <vector>
// from mame:
#include "osdepend.h"
#include "input.h"

#include <stdio.h>

using namespace std;
/******************************************************************************

  Keyboard

******************************************************************************/

/*
  return a list of all available keys (see input.h)
*/
const struct KeyboardInfo *osd_get_key_list(void)
{
    printf(" * * * ** osd_get_key_list  * * * *  *\n");

 /* from mame input.h
    struct KeyboardInfo
    {
        char *name;  OS dependant name;; 0 terminates the list
        unsigned code;  OS dependant code
        InputCode standardcode;	 CODE_xxx equivalent; from list below, or CODE_OTHER if n/a
    };
  */

    static vector<struct KeyboardInfo> kbi;
    static bool inited=false;
    if(!inited)
    {
        inited = true;
        // all the easy constant ones
        kbi = {
            {"ESC",0x45,KEYCODE_ESC},
            {"F1",0x50,KEYCODE_F1},
            {"F2",0x51,KEYCODE_F2},
            {"F3",0x52,KEYCODE_F3},
            {"F4",0x53,KEYCODE_F4},
            {"F5",0x54,KEYCODE_F5},
            {"F6",0x55,KEYCODE_F6},
            {"F7",0x56,KEYCODE_F7},
            {"F8",0x57,KEYCODE_F8},
            {"F9",0x58,KEYCODE_F9},
            {"F10",0x59,KEYCODE_F10},

            {"BACKSPACE",0x41,KEYCODE_BACKSPACE},
            {"DEL",0x46,KEYCODE_DEL},
            {"HELP",0x5F,KEYCODE_HOME},

            {"TAB",0x42,KEYCODE_TAB},

            {"KEY UP",0x4C,KEYCODE_UP},
            {"KEY DOWN",0x4D,KEYCODE_DOWN},
            {"KEY LEFT",0x4F,KEYCODE_LEFT},
            {"KEY RIGHT",0x4E,KEYCODE_RIGHT},

            {"LSHIFT",0x60,KEYCODE_LSHIFT},
            {"RSHIFT",0x61,KEYCODE_RSHIFT},
            {"LALT",0x64,KEYCODE_LALT},
            {"RALT",0x65,KEYCODE_RALT},
            {"LAMIGA",0x66,KEYCODE_LWIN},
            {"RAMIGA",0x67,KEYCODE_RWIN},

            {"CTRL",0x63,KEYCODE_LCONTROL},
            {"SPACE",0x40,KEYCODE_SPACE},
            {"ENTER",0x44,KEYCODE_ENTER},

            // whole amiga pad
            {"0PAD",0x0F,KEYCODE_0_PAD},
            {"1PAD",0x1D,KEYCODE_1_PAD},
            {"2PAD DOWN",0x1E,KEYCODE_2_PAD},
            {"3PAD",0x1F,KEYCODE_3_PAD},
            {"4PAD LEFT",0x2D,KEYCODE_4_PAD},
            {"5PAD",0x2E,KEYCODE_5_PAD},
            {"6PAD RIGHT",0x2F,KEYCODE_6_PAD},
            {"7PAD",0x3D,KEYCODE_7_PAD},
            {"8PAD UP",0x3E,KEYCODE_8_PAD},
            {"9PAD",0x3F,KEYCODE_9_PAD},
            {"- PAD",0x4A,KEYCODE_MINUS_PAD},
            {"+ PAD",0x5E,KEYCODE_PLUS_PAD},
            {"* PAD",0x5E,KEYCODE_ASTERISK},
            {"/ PAD",0x5E,KEYCODE_SLASH_PAD},
            // is missing '.'
            {"ENTER PAD",0x43,KEYCODE_ENTER_PAD}
        };


        kbi.push_back({NULL,0,0});
    }
    return kbi.data();

}

/*
  tell whether the specified key is pressed or not. keycode is the OS dependant
  code specified in the list returned by osd_get_key_list().
*/
int osd_is_key_pressed(int keycode)
{
    printf("osd_is_key_pressed:%d\n",keycode);

    return 0;
}

/*
  wait for the user to press a key and return its code. This function is not
  required to do anything, it is here so we can avoid bogging down multitasking
  systems while using the debugger. If you don't want to or can't support this
  function you can just return OSD_KEY_NONE.
*/
int osd_wait_keypress(void)
{
    printf("osd_wait_keypress\n");

    return OSD_KEY_NONE;
}

/*
  Return the Unicode value of the most recently pressed key. This
  function is used only by text-entry routines in the user interface and should
  not be used by drivers. The value returned is in the range of the first 256
  bytes of Unicode, e.g. ISO-8859-1. A return value of 0 indicates no key down.

  Set flush to 1 to clear the buffer before entering text. This will avoid
  having prior UI and game keys leak into the text entry.
*/
int osd_readkey_unicode(int flush)
{
    printf("osd_readkey_unicode\n");
    return 0;
}
