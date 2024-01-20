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
#include <intuition/intuition.h>
//#include <devices/keyboard.h>
//#include <devices/keymap.h>

// from mame:
#include "osdepend.h"
#include "input.h"

/******************************************************************************

  Keyboard

******************************************************************************/

/*
  return a list of all available keys (see input.h)
*/
const struct KeyboardInfo *osd_get_key_list(void)
{
 /* from mame input.h
    struct KeyboardInfo
    {
        char *name;  OS dependant name;; 0 terminates the list
        unsigned code;  OS dependant code
        InputCode standardcode;	 CODE_xxx equivalent; from list below, or CODE_OTHER if n/a
    };
  */
    static struct KeyboardInfo keysinfo[]={
        {"A",12,KeyCode_A},
        {"B",13,KeyCode_B},
    };

    return keysinfo;
}

/*
  tell whether the specified key is pressed or not. keycode is the OS dependant
  code specified in the list returned by osd_get_key_list().
*/
int osd_is_key_pressed(int keycode)
{

}

/*
  wait for the user to press a key and return its code. This function is not
  required to do anything, it is here so we can avoid bogging down multitasking
  systems while using the debugger. If you don't want to or can't support this
  function you can just return OSD_KEY_NONE.
*/
int osd_wait_keypress(void)
{
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
    return 0;
}
