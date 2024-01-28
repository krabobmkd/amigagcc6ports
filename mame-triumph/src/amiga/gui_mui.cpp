/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id: gui_mui.c,v 1.2 1999/05/13 13:51:02 meh Exp meh $
 *
 * $Log: gui_mui.c,v $
 * Revision 1.2  1999/05/13 13:51:02  meh
 * *** empty log message ***
 *
 * Revision 1.1  1999/04/28 18:54:28  meh
 * Initial revision
 *
 *
 *************************************************************************/

// krb:
/* mui includes generated with:
 /opt/amiga/bin/fd2sfd muimaster_lib.fd ../C/Include/clib/muimaster_protos.h -o muimaster_lib.sfd
  /opt/amiga/bin/sfdc muimaster_lib.sfd --mode clib >../../../../MUI/include/clib/muimaster_protos.h
  /opt/amiga/bin/sfdc muimaster_lib.sfd --mode macros >../../../../MUI/include/inline/muimaster.h
*/
#include <ctype.h>
extern "C" {
#include <clib/alib_protos.h>

#include <exec/types.h>
#include <dos/dosextens.h>
#include <libraries/mui.h>
#include <libraries/iffparse.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>

#include <inline/exec.h>
#include <inline/dos.h>
#include <inline/graphics.h>
#include <clib/intuition_protos.h>
#include <inline/muimaster.h>
#include <inline/utility.h>
//#include <clib/keymap_protos.h>
#include <inline/keymap.h>
#include "intuiuncollide.h"
}
#define CATCOMP_NUMBERS
#include "messages.h"

#include "macros.h"
#include "main.h"
#include "driver.h"
#include "gui.h"
#include "config.h"
#include "version.h"

#include <vector>
#include <string>
#include <cstdio>

typedef ULONG (*RE_HOOKFUNC)();

#define RID_Start      1
#define RID_CloseAbout 2
#define RID_Scan       3

#define MID_About     104
#define MID_AboutMUI  105

#define SMT_DISPLAYID 0
#define SMT_DEPTH     1

#ifdef MESS
#define DRIVER_OFFSET 0

#define TEXT_ABOUT \
"\33c\n\33b\33uMESS - Multi-Emulator Super System\33n\n\n" \
"0."REVISION" ("REVDATE")\n\n" \
"Copyright (C) 1998 by the MESS team\n" \
"http://www.internetter.com/titan/mess\n\n" \
"Amiga port by Mats Eirik Hansen\n" \
"http://www.triumph.no/mess\n" \
"CGXHooks routines by Trond Werner Hansen\n\n" \
"Chunky to planar routine by Mikael Kalms\n\n" \
"This program uses MUI - Magic User Interface\n" \
"MUI is © 1992 - 1997 by Stefan Stuntz\n" \
"http://www.sasg.com"
#else
#define DRIVER_OFFSET 2

#define TEXT_ABOUT \
"\33c\n\33b\33uMAME - Multiple Arcade Machine Emulator\33n\n\n" \
"0."REVISION" ("REVDATE")\n\n" \
"Copyright (C) 1997-1999 by Nicola Salmoria and the MAME team\n" \
"http://www.media.dsi.unimi.it/mame\n\n" \
"Amiga port by Mats Eirik Hansen\n" \
"http://www.triumph.no/mame\n" \
"CGXHooks routines by Trond Werner Hansen\n\n" \
"Chunky to planar routine by Mikael Kalms\n\n" \
"This program uses MUI - Magic User Interface\n" \
"MUI is © 1992 - 1997 by Stefan Stuntz\n" \
"http://www.sasg.com"
#endif

struct DriverData
{
  struct MUI_EventHandlerNode EventHandler;
  ULONG           CurrentEntry;
  ULONG           CharIndex;
  ULONG           Seconds;
  ULONG           Micros;
  APTR            List;
};

// this is from mui.h, but unreachable from C++ there
#define get(obj,attr,store) GetAttr(attr,(Object *)obj,(ULONG *)store)
#define set(obj,attr,value) SetAttrs(obj,attr,value,TAG_DONE)

extern struct Library *KeymapBase;

struct Library *MUIMasterBase = NULL;

static struct GameDriver ***SortedDrivers = NULL;

static char  *DriversFound = NULL;
static ULONG NumDrivers;

static struct MUI_CustomClass *DriverClass;

static Object *App=NULL;
static Object *MainWin=NULL;
static Object *AboutWin=NULL;
static Object *DisplayName=NULL;

static Object * RE_Options=NULL;
static Object * CM_Allow16Bit=NULL;
static Object * CM_FlipX=NULL;
static Object * CM_FlipY=NULL;
static Object * CM_DirtyLines=NULL;
static Object * CM_AutoFrameSkip=NULL;
#ifndef MESS
static Object * CY_Show;
static Object *CM_UseDefaults;
static Object * BU_Scan;
#endif
static Object * CM_Antialiasing;
static Object * CM_Translucency;
static Object * SL_BeamWidth;
static Object * SL_VectorFlicker;
static Object * SL_AudioChannel[4];
static Object * SL_MinFreeChip;
static Object * SL_FrameSkip;
static Object * SL_Joy1ButtonBTime;
static Object * SL_Joy1AutoFireRate;
static Object * SL_Joy2ButtonBTime;
static Object * SL_Joy2AutoFireRate;
static Object * ST_Width;
static Object * ST_Height;
static Object * ST_RomPath;
static Object * ST_SamplePath;
static Object * CY_ScreenType;
static Object * CY_DirectMode;
static Object * CY_Sound;
static Object * CY_Buffering;
static Object * CY_Rotation;
static Object * CY_Joy1Type;
static Object * CY_Joy2Type;
static Object * PA_ScreenMode;
static Object * PA_RomPath;
static Object * PA_SamplePath;
static Object *LI_Driver;
static Object * LV_Driver;
static Object * BU_Start;
static Object * BU_Quit;
static Object * BU_About_OK;
static Object * PU_ScreenMode;

#ifdef POWERUP
static Object * CM_AsyncPPC;
#endif

//static UBYTE           DisplayNameBuffer[256];
std::string DisplayNameBuffer;

static STRPTR Shows[] =
{
  (STRPTR) MSG_ALL,
  (STRPTR) MSG_FOUND,
  NULL
};

static STRPTR ScreenTypes[] =
{
  (STRPTR) MSG_BEST,
  (STRPTR) MSG_WORKBENCH,
  (STRPTR) MSG_CUSTOM,
  (STRPTR) MSG_USER_SELECT,
  NULL
};

static APTR DirectModes[] =
{
  (STRPTR) MSG_OFF,
  (STRPTR) MSG_DRAW,
  (STRPTR) MSG_COPY,
  NULL
};

static STRPTR Sounds[] =
{
  (STRPTR) MSG_NONE,
  (STRPTR) MSG_PAULA,
  (STRPTR) MSG_AHI,
  NULL
};

static STRPTR Joy1Types[] =
{
  (STRPTR) MSG_NONE,
  (STRPTR) MSG_JOYSTICK_PORT_2,
  (STRPTR) MSG_JOYPAD_PORT_2,
  (STRPTR) MSG_MOUSE_PORT_1,
  NULL
};

static STRPTR Joy2Types[] =
{
  (STRPTR) MSG_NONE,
  (STRPTR) MSG_JOYSTICK_PORT_1,
  (STRPTR) MSG_JOYPAD_PORT_1,
  NULL
};

static STRPTR Rotations[] =
{
  (STRPTR) MSG_NO,
  (STRPTR) MSG_LEFT,
  (STRPTR) MSG_RIGHT,
  NULL
};

static STRPTR RegisterTitles[] =
{
  (STRPTR) MSG_DRIVERS,
  (STRPTR) MSG_DISPLAY,
  (STRPTR) MSG_SOUND,
  (STRPTR) MSG_INPUT,
  (STRPTR) MSG_PATHS,
  NULL
};

static STRPTR Bufferings[] =
{
  (STRPTR) MSG_SINGLE,
  (STRPTR) MSG_DOUBLE,
  (STRPTR) MSG_TRIPLE,
  NULL
};

static struct TagItem ScreenModeTags[] =
{
  { ASLSM_InitialDisplayID,   0 },
  { ASLSM_InitialDisplayDepth,  8 },
  { ASLSM_DoDepth,        TRUE  },
  { TAG_END }
};

static struct Hook ScreenModeStartHook;
static struct Hook ScreenModeStopHook;
static struct Hook ScreenTypeNotifyHook;
static struct Hook DirectModeNotifyHook;
static struct Hook SoundNotifyHook;
static struct Hook DriverDisplayHook;
static struct Hook DriverNotifyHook;
#ifndef MESS
static struct Hook ShowNotifyHook;
static struct Hook UseDefaultsNotifyHook;
#endif

#ifndef MESS
static char *DriverString;
static char *DirectoryString;
static char *TypeString;
static char *WidthString;
static char *HeightString;
static char *ColorsString;
static char *CommentString;
static char *NotWorkingString;
static char *WrongColorsString;
static char *ImperfectColorsString;
static char *BitmapString;
static char *VectorString;
static char *BitmapGameDefaultsString;
static char *VectorGameDefaultsString;
#endif

static void CreateApp(void);
static void GetOptions(BOOL get_driver);
static void SetOptions(BOOL set_driver);
static void SetDisplayName(ULONG);
static ULONG ASM ScreenModeStart(struct Hook *hook REG(a0), APTR popasl REG(a2), struct TagItem *taglist REG(a1));
static ULONG ASM ScreenModeStop(struct Hook *hook REG(a0), APTR popasl REG(a2), struct ScreenModeRequester *smreq REG(a1));
static ULONG ASM ScreenTypeNotify(struct Hook *hook REG(a0), APTR obj REG(a2), ULONG *par REG(a1));
static ULONG ASM DirectModeNotify(struct Hook *hook REG(a0), APTR obj REG(a2), ULONG *par REG(a1));
static ULONG ASM SoundNotify(struct Hook *hook REG(a0), APTR obj REG(a2), ULONG *par REG(a1));
static ULONG ASM DriverNotify(struct Hook *hook REG(a0), APTR obj REG(a2), ULONG *par REG(a1));
#ifndef MESS
static ULONG ASM ShowNotify(struct Hook *hook REG(a0), APTR obj REG(a2), ULONG *par REG(a1));
static ULONG ASM UseDefaultsNotify(struct Hook *hook REG(a0), APTR obj REG(a2), ULONG *par REG(a1));
#endif

/* Convert an index in SortedDrivers into an index for the same
 * driver in Drivers. */

static inline int GetSortedDriverIndex(int sorted_index)
{
  int index;

  if(sorted_index < DRIVER_OFFSET)
    index = sorted_index - DRIVER_OFFSET;
  else
    index = (((ULONG) SortedDrivers[sorted_index]) - ((ULONG) &Drivers[0])) / sizeof(struct GameDriver *);

  return(index);
}

static inline int GetEntryDriverIndex(ULONG entry)
{
  int index;

  index = (entry - ((ULONG) &Drivers[0])) / sizeof(struct GameDriver *);

  return(index);
}

static int GetDriverIndex(void)
{
  ULONG entry;
  ULONG list_index;
  int   index;

  get(LI_Driver, MUIA_List_Active, &list_index);

  if(list_index == MUIV_List_Active_Off)
  {
    index = -DRIVER_OFFSET - 1;
  }
  else if(list_index < DRIVER_OFFSET)
  {
    index = list_index - DRIVER_OFFSET;
  }
  else
  {
    DoMethod((Object*)LI_Driver,(ULONG) MUIM_List_GetEntry, (ULONG)list_index, (ULONG)&entry);

    index = GetEntryDriverIndex(entry);
  }

  return(index);
}

static struct GameDriver *GetDriver(void)
{
  struct GameDriver *drv;

  ULONG entry;
  ULONG list_index;

  get(LI_Driver, MUIA_List_Active, &list_index);

  if((list_index == MUIV_List_Active_Off) || (list_index < DRIVER_OFFSET))
    return(NULL);

  DoMethod((Object*)LI_Driver, MUIM_List_GetEntry, list_index, &entry);

  if(!entry)
    return(NULL);

  drv = *((struct GameDriver **) entry);

  return(drv);
}

#ifndef MESS
static void ScanDrivers(void)
{
  struct FileInfoBlock *fib;
  const char           *str;

  BPTR locks[4];
  LONG i, j, len;
  char buf[13];    /* 8.3 filename. */
  int  bitmap_lock;
  int  vector_lock;

  if(DriversFound)
  {
    memset(DriversFound, 0, NumDrivers);

    fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);

    if(fib)
    {
      bitmap_lock = -1;
      vector_lock = -1;

      j = 0;

      locks[j++] = DupLock(((struct Process *) FindTask(NULL))->pr_CurrentDir);

      locks[j++] = Lock("roms", ACCESS_READ);

      str = GetRomPath(0, 0);

      if(str)
      {
        if(str[0])
        {
          locks[j] = Lock((STRPTR) str, ACCESS_READ);

          if(locks[j])
          {
            if( (SameLock(locks[0], locks[j]) == LOCK_SAME)
            ||  (SameLock(locks[1], locks[j]) == LOCK_SAME))
              UnLock(locks[j]);
            else
              bitmap_lock = j++;
          }
        }
      }

      str = GetRomPath(1, 0);

      if(str)
      {
        if(strlen(str))
        {
          locks[j] = Lock((STRPTR) str, ACCESS_READ);

          if(locks[j])
          {
            if( (SameLock(locks[0], locks[j]) == LOCK_SAME)
            ||  (SameLock(locks[1], locks[j]) == LOCK_SAME))
              UnLock(locks[j]);
            else
              vector_lock = j++;
          }
        }
      }

      for(; --j >= 0;)
      {
        if(Examine(locks[j], fib))
        {
          if(fib->fib_DirEntryType > 0)
          {
            while(ExNext(locks[j], fib))
            {
              for(i = 0; i < NumDrivers; i++)
              {
                if( !DriversFound[i]
                &&  ((j != bitmap_lock) || !((*SortedDrivers[i+DRIVER_OFFSET])->drv->video_attributes & VIDEO_TYPE_VECTOR))
                &&  ((j != vector_lock) || ((*SortedDrivers[i+DRIVER_OFFSET])->drv->video_attributes & VIDEO_TYPE_VECTOR)))
                {
                  len = strlen((*SortedDrivers[i+DRIVER_OFFSET])->name);

                  if(!strnicmp(fib->fib_FileName, (*SortedDrivers[i+DRIVER_OFFSET])->name, len))
                  {
                    if(!fib->fib_FileName[len])
                    {
                      if(fib->fib_DirEntryType > 0)
                      {
                        DriversFound[i] = 1;
                        break;
                      }
                    }
                    else if(fib->fib_DirEntryType < 0)
                    {
                      if( !stricmp(&fib->fib_FileName[len], ".zip")
                      ||  !stricmp(&fib->fib_FileName[len], ".lha")
                      ||  !stricmp(&fib->fib_FileName[len], ".lzx"))
                      {
                        DriversFound[i] = 1;
                        break;
                      }
                    }
                  }
                }
              }
            }
          }
        }

        if(locks[j])
          UnLock(locks[j]);
      }
    }

    /* The code above searched in current dir, roms/ and any the rom path specified for
     * the bitmap and vector driver defaults. Now I'll look for any driver that has its
     * own rom path. */

    for(i = 0; i < NumDrivers; i++)
    {
      if(!DriversFound[i])
      {
        if(!GetUseDefaults(GetSortedDriverIndex(DRIVER_OFFSET+i)))
        {
          str = GetRomPath(GetSortedDriverIndex(DRIVER_OFFSET+i), 0);

          if(str && strlen(str))
          {
            locks[0] = Lock((STRPTR) str, ACCESS_READ);

            if(locks[0])
            {
              locks[1] = CurrentDir(locks[0]);

              locks[2] = Lock((char *) (*SortedDrivers[i+DRIVER_OFFSET])->name, ACCESS_READ);

              if(!locks[2])
              {
                sprintf(buf, "%s.zip", (*SortedDrivers[i+DRIVER_OFFSET])->name);
                locks[2] = Lock(buf, ACCESS_READ);

                if(!locks[2])
                {
                  sprintf(buf, "%s.lha", (*SortedDrivers[i+DRIVER_OFFSET])->name);
                  locks[2] = Lock(buf, ACCESS_READ);

                  if(!locks[2])
                  {
                    sprintf(buf, "%s.lzx", (*SortedDrivers[i+DRIVER_OFFSET])->name);
                    locks[2] = Lock(buf, ACCESS_READ);
                  }
                }
              }

              if(locks[2])
              {
                UnLock(locks[2]);

                DriversFound[i] = 1;
              }

              CurrentDir(locks[1]);
              UnLock(locks[0]);
            }
          }
        }
      }
    }

    for(i = 0; i < NumDrivers; i++)
      SetFound(DRIVER_OFFSET+GetSortedDriverIndex(DRIVER_OFFSET+i),DriversFound[i]);
  }
}

static void ShowFound(void)
{
  int start;
  int end;
  int num;
  int max;

  start = 0;
  end   = DRIVER_OFFSET;
  max   = NumDrivers + DRIVER_OFFSET;

  while(start < max)
  {
    for(; (end < max) && GetFound(DRIVER_OFFSET+GetSortedDriverIndex(end)); end++);

    num = end - start;

    if(num > 0)
      DoMethod((Object *)LI_Driver, MUIM_List_Insert, &SortedDrivers[start], num, MUIV_List_Insert_Bottom);

    for(; (end < max) && !GetFound(DRIVER_OFFSET+GetSortedDriverIndex(end)); end++);

    start = end;

    end++;
  }
}
#endif

static int DriverCompare(struct GameDriver ***drv1, struct GameDriver ***drv2)
{
  return(stricmp((**drv1)->description, (**drv2)->description));
}

static ULONG ASM DriverDisplay(struct Hook *hook REG(a0), char **array REG(a2), struct GameDriver **drv_indirect REG(a1))
{
  struct GameDriver *drv;

#ifdef MESS
  *array++ = (char *) drv->description;
#else
  static char driver[55];
  static char directory[55];
  static char type[55];
  static char width[55];
  static char height[55];
  static char colors[55];
  static char comment[105];

  if(!drv_indirect)
  {
    sprintf(driver,   "\033b\033u%s", DriverString);
    sprintf(directory,  "\033b\033u%s", DirectoryString);
    sprintf(type,   "\033b\033u%s", TypeString);
    sprintf(width,    "\033b\033u%s", WidthString);
    sprintf(height,   "\033b\033u%s", HeightString);
    sprintf(colors,   "\033b\033u%s", ColorsString);
    sprintf(comment,  "\033b\033u%s", CommentString);

    *array++  = driver;
    *array++  = directory;
    *array++  = type;
    *array++  = width;
    *array++  = height;
    *array++  = colors;
    *array++  = comment;

    return(0);
  }

  if(drv_indirect == (struct GameDriver **) 1)
  {
    sprintf(driver, "\0338%s", BitmapGameDefaultsString);

    *array++  = driver;
    *array++  = "";
    *array++  = "";
    *array++  = "";
    *array++  = "";
    *array++  = "";
    *array++  = "";

    return(0);
  }

  if(drv_indirect == (struct GameDriver **) 2)
  {
    sprintf(driver, "\0338%s", VectorGameDefaultsString);

    *array++  = driver;
    *array++  = "";
    *array++  = "";
    *array++  = "";
    *array++  = "";
    *array++  = "";
    *array++  = "";

    return(0);
  }

  drv = *drv_indirect;

  *array++ = (char *) drv->description;
  *array++ = (char *) drv->name;

  if(drv->drv->video_attributes & VIDEO_TYPE_VECTOR)
  {
    *array++  = VectorString;
    *array++  = "";
    *array++  = "";
  }
  else
  {
  /*

	UINT32 flags;	 orientation and other flags; see defines below
 values for the flags field

#define ORIENTATION_MASK        	0x0007
#define	ORIENTATION_FLIP_X			0x0001	 mirror everything in the X direction
#define	ORIENTATION_FLIP_Y			0x0002	 mirror everything in the Y direction
#define ORIENTATION_SWAP_XY			0x0004	 mirror along the top-left/bottom-right diagonal

  */
    // krb note: there was flags changes between 0.35 and 0.37

    *array++  = BitmapString;
    if(drv->flags & ORIENTATION_SWAP_XY)
      sprintf(width, "%d", drv->drv->screen_height);
    else
      sprintf(width, "%d", drv->drv->screen_width);
    *array++  = width;
    if(drv->flags & ORIENTATION_SWAP_XY)
      sprintf(height, "%d", drv->drv->screen_width);
    else
      sprintf(height, "%d", drv->drv->screen_height);
    *array++  = height;
  }
// GAME_REQUIRES_16BIT
//0.35  if(drv->drv->video_attributes & VIDEO_SUPPORTS_16BIT)
  if(drv->flags & GAME_REQUIRES_16BIT ||
    drv->drv->total_colors > 256
    )
    sprintf(colors, "16Bit");
  else
    sprintf(colors, "%d", drv->drv->total_colors);

  *array++ = colors;

  if(drv->flags & GAME_NOT_WORKING)
    *array++ = NotWorkingString;
  else if(drv->flags & GAME_WRONG_COLORS)
    *array++ = WrongColorsString;
  else if(drv->flags & GAME_IMPERFECT_COLORS)
    *array++ = ImperfectColorsString;
  else
    *array++ = "";
#endif
  return(0);
}

static ULONG ASM DriverDispatcher(struct IClass *cclass REG(a0), Object * obj REG(a2), Msg msg REG(a1))
{
  struct DriverData   *data;
  struct IntuiMessage *imsg;
  struct GameDriver   **drv_indirect;
  struct GameDriver   *drv;
  struct InputEvent   ie;

  Object  *list;
  APTR  active_obj;
  ULONG i;
  UBYTE key;

  switch(msg->MethodID)
  {
    case MUIM_Setup:
      data = (struct DriverData *)INST_DATA(cclass, obj);

      if(DoSuperMethodA(cclass, obj, msg))
      {
        data->Seconds = 0;
        data->Micros  = 0;

        data->EventHandler.ehn_Priority = 0;
        data->EventHandler.ehn_Flags    = 0;
        data->EventHandler.ehn_Object   = obj;
        data->EventHandler.ehn_Class    = cclass;
        data->EventHandler.ehn_Events   = IDCMP_RAWKEY;
        DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->EventHandler);

        return(TRUE);
      }

      return(FALSE);

    case MUIM_Cleanup:
      data = (struct DriverData *)INST_DATA(cclass, obj);

      DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->EventHandler);
      break;

    case MUIM_HandleEvent:
      data =  (struct DriverData *) INST_DATA(cclass, obj);
      imsg = (struct IntuiMessage *) msg[1].MethodID;

      get(_win(obj), MUIA_Window_ActiveObject, &active_obj);

      if(obj == active_obj)
      {
        if(imsg->Class == IDCMP_RAWKEY)
        {
          ie.ie_Class   = IECLASS_RAWKEY;
          ie.ie_SubClass  = 0;
          ie.ie_Code    = imsg->Code;
          ie.ie_Qualifier = 0;

//  MapRawKey( CONST struct InputEvent *event, STRPTR buffer, LONG length, CONST struct KeyMap *keyMap );
          if(MapRawKey(&ie,(STRPTR)&key, 1, NULL) && isalnum(key))
          {
            i = imsg->Seconds - data->Seconds;

            if(imsg->Micros < data->Micros)
              i--;

            if(i < 1)
            {
              data->CharIndex++;
              i = data->CurrentEntry;
            }
            else
            {
              data->CharIndex = 0;
              i = DRIVER_OFFSET;
            }

            data->Seconds = imsg->Seconds;
            data->Micros  = imsg->Micros;

            get(obj, MUIA_Listview_List, &list);

            do
            {
              DoMethod(list, MUIM_List_GetEntry, i, &drv_indirect);

              if(drv_indirect)
              {
                drv = *drv_indirect;

                if(data->CharIndex < strlen(drv->description))
                {
                  if(key <= tolower(drv->description[data->CharIndex]))
                  {
                    data->CurrentEntry = i;

                    set(list, MUIA_List_Active, i);

                    break;
                  }
                }
              }

              i++;

            } while(drv_indirect);

            return(MUI_EventHandlerRC_Eat);
          }
        }
      }

      return(0);
  }

  return(DoSuperMethodA(cclass, obj, msg));
}

void AllocGUI(void)
{
  LONG  i;

  for(NumDrivers = 0; Drivers[NumDrivers]; NumDrivers++);

  SortedDrivers = ( struct GameDriver ***)malloc((NumDrivers + DRIVER_OFFSET) * sizeof(struct GameDriver **));

  if(SortedDrivers)
  {
#ifndef MESS
    SortedDrivers[0]  = (struct GameDriver **) 1;
    SortedDrivers[1]  = (struct GameDriver **) 2;
#endif
    for(i = 0; i < NumDrivers; i++)
      SortedDrivers[i+DRIVER_OFFSET] = const_cast<struct GameDriver **>(&Drivers[i]);

    qsort(&SortedDrivers[DRIVER_OFFSET], NumDrivers, sizeof(struct GameDriver **), (int (*)(const void *, const void *)) DriverCompare);

    MUIMasterBase = OpenLibrary("muimaster.library", 16);

    DriversFound  = (char *)malloc(NumDrivers);

    App     = NULL;
    MainWin   = NULL;
    AboutWin  = NULL;

    for(i = 0; Shows[i]; i++)
      Shows[i] = GetMessage((LONG) Shows[i]);

    for(i = 0; ScreenTypes[i]; i++)
      ScreenTypes[i] = GetMessage((LONG) ScreenTypes[i]);

    for(i = 0; DirectModes[i]; i++)
      DirectModes[i] = GetMessage((LONG) DirectModes[i]);

    for(i = 0; Sounds[i]; i++)
      Sounds[i] = GetMessage((LONG) Sounds[i]);

    for(i = 0; Joy1Types[i]; i++)
      Joy1Types[i] = GetMessage((LONG) Joy1Types[i]);

    for(i = 0; Joy2Types[i]; i++)
      Joy2Types[i] = GetMessage((LONG) Joy2Types[i]);

    for(i = 0; Rotations[i]; i++)
      Rotations[i] = GetMessage((LONG) Rotations[i]);

    for(i = 0; RegisterTitles[i]; i++)
      RegisterTitles[i] = GetMessage((LONG) RegisterTitles[i]);

    for(i = 0; Bufferings[i]; i++)
      Bufferings[i] = GetMessage((LONG) Bufferings[i]);

    ScreenModeStartHook.h_Entry  = (RE_HOOKFUNC) ScreenModeStart;
    ScreenModeStopHook.h_Entry   = (RE_HOOKFUNC) ScreenModeStop;
    ScreenTypeNotifyHook.h_Entry = (RE_HOOKFUNC) ScreenTypeNotify;
    DirectModeNotifyHook.h_Entry = (RE_HOOKFUNC) DirectModeNotify;
    SoundNotifyHook.h_Entry      = (RE_HOOKFUNC) SoundNotify;
    DriverDisplayHook.h_Entry    = (RE_HOOKFUNC) DriverDisplay;
    DriverNotifyHook.h_Entry     = (RE_HOOKFUNC) DriverNotify;
#ifndef MESS
    ShowNotifyHook.h_Entry        = (RE_HOOKFUNC) ShowNotify;
    UseDefaultsNotifyHook.h_Entry = (RE_HOOKFUNC) UseDefaultsNotify;

    DriverString             = GetMessage(MSG_DRIVER);
    DirectoryString          = GetMessage(MSG_DIRECTORY);
    TypeString               = GetMessage(MSG_TYPE);
    WidthString              = GetMessage(MSG_WIDTH);
    HeightString             = GetMessage(MSG_HEIGHT);
    ColorsString             = GetMessage(MSG_COLORS);
    CommentString            = GetMessage(MSG_COMMENT);
    NotWorkingString         = GetMessage(MSG_NOT_WORKING);
    WrongColorsString        = GetMessage(MSG_WRONG_COLORS);
    ImperfectColorsString    = GetMessage(MSG_IMPERFECT_COLORS);
    BitmapString             = GetMessage(MSG_BITMAP);
    VectorString             = GetMessage(MSG_VECTOR);
    BitmapGameDefaultsString = GetMessage(MSG_BITMAP_GAME_DEFAULTS);
    VectorGameDefaultsString = GetMessage(MSG_VECTOR_GAME_DEFAULTS);
#endif

    DriverClass = MUI_CreateCustomClass(NULL, MUIC_Listview, NULL, sizeof(struct DriverData),(APTR) DriverDispatcher);
  }
}

void FreeGUI(void)
{
  if(SortedDrivers)
  {
    if(App)
      MUI_DisposeObject(App);

    if(DriverClass)
      MUI_DeleteCustomClass(DriverClass);

    if(DriversFound)
      free(DriversFound);

    if(MUIMasterBase)
      CloseLibrary(MUIMasterBase);

    free(SortedDrivers);

    SortedDrivers = NULL;
  }
}

// = = = = = = = objects contructors = = = = = =
/*ok but
inline Object *cMenuItem(STRPTR text,STRPTR shortcut,void *userData)
{
    vector<struct TagItem> tags;
    tags.reserve(4);

    if(text) tags.push_back({MUIA_Menuitem_Title,(ULONG)text});
    if(shortcut) tags.push_back({MUIA_Menuitem_Shortcut,(ULONG)shortcut});
    if(userData) tags.push_back({MUIA_UserData,(ULONG)userData});
    tags.push_back({TAG_END,NULL});

    return MUI_NewObjectA(MUIC_Menuitem,tags.data() );
}
inline Object *cMenu1()
{
    return MUI_NewObject(MUIC_Menu,
MUIA_Menu_Title,(ULONG)GetMessage(MSG_MENU_GAME),
MUIA_Family_Child,(ULONG)cMenuItem(GetMessage(MSG_MENU_ABOUT),"?",MID_About),
MUIA_Family_Child,(ULONG)cMenuItem("About MUI...",NULL,MID_AboutMUI),
MUIA_Family_Child,(ULONG)MUI_NewObject(MUIC_Menuitem,MUIA_Menuitem_Title,NM_BARLABEL,End),
MUIA_Family_Child,(ULONG)cMenuItem( GetMessage(MSG_MENU_QUIT),"Q",MUIV_Application_ReturnID_Quit),
            End
            );

}
inline Object *cMainVLayout()
{
// #define RegisterGroup(t)    MUI_NewObject(MUIC_Register,MUIA_Register_Titles,(t)
  return MUI_NewObject(MUIC_Group,
        Child, RE_Options =MUI_NewObject(MUIC_Register,MUIA_Register_Titles,(RegisterTitles)
    TODO
  );
}
inline Object *cMainWin()
{
    Object *pMenu1 = cMenu1();
    Object *pMenuStrip = MUI_NewObject(MUIC_Menustrip,MUIA_Family_Child,(ULONG)pMenu1);

    Object *pMainVLayout = cMainVLayout();

    return MUI_NewObject(MUIC_Window,
          MUIA_Window_Title, APPNAME,
          MUIA_Window_ID   , MAKE_ID('M','A','I','N'),
          MUIA_Window_Menustrip, pMenuStrip,
          WindowContents,(ULONG) pMainVLayout,
            End
            );
}
*/
// use less macros
inline Object *MUINewObject( char *pclassname,ULONG tag1, ... )
{
   return MUI_NewObjectA(pclassname, (struct TagItem *) &tag1);
}
inline ULONG UMUINO(char *pclassname,ULONG tag1, ... )
{
   return (ULONG)MUI_NewObjectA(pclassname, (struct TagItem *) &tag1);
}
inline Object *OMUINO(char *pclassname,ULONG tag1, ... )
{
   return MUI_NewObjectA(pclassname, (struct TagItem *) &tag1);
}
#define UCheckMark(selected)\
	(ULONG)MUI_NewObject(MUIC_Image,\
		ImageButtonFrame,\
		MUIA_InputMode        , MUIV_InputMode_Toggle,\
		MUIA_Image_Spec       , MUII_CheckMark,\
		MUIA_Image_FreeVert   , TRUE,\
		MUIA_Selected         , selected,\
		MUIA_Background       , MUII_ButtonBack,\
		MUIA_ShowSelState     , FALSE,\
		TAG_DONE)
#define OCheckMark(selected)\
	MUI_NewObject(MUIC_Image,\
		ImageButtonFrame,\
		MUIA_InputMode        , MUIV_InputMode_Toggle,\
		MUIA_Image_Spec       , MUII_CheckMark,\
		MUIA_Image_FreeVert   , TRUE,\
		MUIA_Selected         , selected,\
		MUIA_Background       , MUII_ButtonBack,\
		MUIA_ShowSelState     , FALSE,\
		TAG_DONE)
#define OString(contents,maxlen)\
	MUI_NewObject(MUIC_String,\
		StringFrame,\
		MUIA_String_MaxLen  , maxlen,\
		MUIA_String_Contents, contents,\
		TAG_DONE)


int MainGUI(void)
{
  ULONG rid;
  ULONG v;
  ULONG signals = 0;
  BOOL  loop  = TRUE;
//
  if(MUIMasterBase)
  {
    if(!App)
      CreateApp();
    if(App)
    {
      if(!MainWin)
      {
// //#define MUI_NewObject(MUIC_List          MUI_NewObject(MUIC_List
        MainWin =  MUINewObject(MUIC_Window,
          MUIA_Window_Title,(ULONG) APPNAME,
          MUIA_Window_ID   , MAKE_ID('M','A','I','N'),

          MUIA_Window_Menustrip, UMUINO(MUIC_Menustrip,
            MUIA_Family_Child,UMUINO(MUIC_Menu,MUIA_Menu_Title,(ULONG)GetMessage(MSG_MENU_GAME),
              MUIA_Family_Child, UMUINO(MUIC_Menuitem,
                MUIA_Menuitem_Title,  (ULONG)GetMessage(MSG_MENU_ABOUT),
                MUIA_Menuitem_Shortcut,(ULONG) "?",
                MUIA_UserData,      MID_About,
              TAG_DONE),
              MUIA_Family_Child, UMUINO(MUIC_Menuitem,
                MUIA_Menuitem_Title, (ULONG) "About MUI...",
                MUIA_UserData,      MID_AboutMUI,
              TAG_DONE),
              MUIA_Family_Child, UMUINO(MUIC_Menuitem,
                MUIA_Menuitem_Title,  NM_BARLABEL,
              TAG_DONE),
              MUIA_Family_Child, UMUINO(MUIC_Menuitem,
                MUIA_Menuitem_Title,  (ULONG)GetMessage(MSG_MENU_QUIT),
                MUIA_Menuitem_Shortcut,(ULONG) "Q",
                MUIA_UserData,      MUIV_Application_ReturnID_Quit,
              TAG_DONE),
            TAG_DONE),
          TAG_DONE),

          WindowContents, UMUINO(MUIC_Group,

            Child, RE_Options = OMUINO(MUIC_Register,MUIA_Register_Titles,(RegisterTitles),

              Child, UMUINO(MUIC_Group,
                Child, LV_Driver = (Object *)(DriverClass!=NULL)?

                    ((Object *)NewObject(DriverClass->mcc_Class, NULL,
                  MUIA_Listview_Input,    TRUE,
                    MUIA_Listview_List, (ULONG)( LI_Driver = MUINewObject(MUIC_List,
                      MUIA_List_Title,    TRUE,
                      MUIA_List_Format,   "BAR,BAR,BAR,BAR,BAR,BAR,",
                      MUIA_List_DisplayHook,(ULONG)  &DriverDisplayHook,
                    InputListFrame,
                  TAG_DONE)),
                TAG_END)
                     )
                :(Object *) OMUINO(MUIC_Listview,
                  MUIA_Listview_Input, TRUE,
                    MUIA_Listview_List, (ULONG)( LI_Driver = MUINewObject(MUIC_List,
                      MUIA_List_Title, TRUE,
                      MUIA_List_Format, "BAR,BAR,BAR,BAR,BAR,BAR,",
                      MUIA_List_DisplayHook,  &DriverDisplayHook,
                    InputListFrame,
                  TAG_DONE)),
                TAG_DONE)
                ,
#ifndef MESS
                Child, UMUINO(MUIC_Group,MUIA_Group_Horiz,TRUE,
                  Child, Label((ULONG)GetMessage(MSG_USE_DEFAULTS)),
                  Child, (ULONG)(CM_UseDefaults = OCheckMark(Config[CFG_USEDEFAULTS])),
                  Child, Label((ULONG)GetMessage(MSG_SHOW)),
                  Child, (ULONG) (CY_Show = OMUINO(MUIC_Cycle,
                    MUIA_Cycle_Entries, (ULONG) Shows,
                  TAG_DONE)),
                  Child,(ULONG)(BU_Scan = SimpleButton((ULONG)GetMessage(MSG_SCAN))),
                TAG_DONE),
#endif
              TAG_DONE),
              Child, UMUINO(MUIC_Group,
                Child, HVSpace,
                Child, UMUINO(MUIC_Group,MUIA_Group_Horiz,TRUE,
                  Child, HSpace(0),
                  Child, UMUINO(MUIC_Group,MUIA_Group_Columns,4,
                    MUIA_HorizWeight, 1000,
                    Child, Label((ULONG)GetMessage(MSG_ALLOW16BIT)),
                    Child, (ULONG)(CM_Allow16Bit = OCheckMark(Config[CFG_ALLOW16BIT]))),
                    Child, Label((ULONG)GetMessage(MSG_AUTO_FRAMESKIP)),
                    Child, (ULONG)(CM_AutoFrameSkip = OCheckMark(Config[CFG_AUTOFRAMESKIP])),
                    Child, Label((ULONG)GetMessage(MSG_FLIPX)),
                    Child, (ULONG)(CM_FlipX = OCheckMark(Config[CFG_FLIPX])),
                    Child, Label((ULONG)GetMessage(MSG_ANTIALIAS)),
                    Child, (ULONG)(CM_Antialiasing = OCheckMark(Config[CFG_ANTIALIASING])),
                    Child, Label((ULONG)GetMessage(MSG_FLIPY)),
                    Child, (ULONG)(CM_FlipY = OCheckMark(Config[CFG_FLIPY])),
                    Child, Label((ULONG)GetMessage(MSG_TRANSLUCENCY)),
                    Child, (ULONG)(CM_Translucency = OCheckMark(Config[CFG_TRANSLUCENCY])),
                    Child, Label((ULONG)GetMessage(MSG_DIRTY_LINES)),
                    Child, (ULONG)(CM_DirtyLines = OCheckMark(Config[CFG_DIRTYLINES])),
#ifdef POWERUP
                    Child, Label((ULONG)GetMessage(MSG_ASYNCPPC)),
                    Child, CM_AsyncPPC = UCheckMark(Config[CFG_ASYNCPPC]),
#endif
                  TAG_DONE),
                  Child, HSpace(0),
                TAG_DONE),
                Child, UMUINO(MUIC_Group,MUIA_Group_Horiz,TRUE,
                  Child, HSpace(0),
                  Child, UMUINO(MUIC_Group,MUIA_Group_Columns,2,
                    MUIA_HorizWeight, 1000,
                    Child, Label((ULONG)GetMessage(MSG_BEAM)),
                    Child,(ULONG)( SL_BeamWidth = OMUINO(MUIC_Slider,
                      MUIA_Slider_Min, 1,
                      MUIA_Slider_Max, 16,
                    TAG_DONE)),
                    Child, Label((ULONG)GetMessage(MSG_FLICKER)),
                    Child,(ULONG)( SL_VectorFlicker = OMUINO(MUIC_Slider,
                      MUIA_Slider_Min, 0,
                      MUIA_Slider_Max, 100,
                    TAG_DONE)),
                    Child, Label((ULONG)GetMessage(MSG_FRAMESKIP)),
                    Child,(ULONG)( SL_FrameSkip = OMUINO(MUIC_Slider,
                      MUIA_Slider_Min, 0,
                      MUIA_Slider_Max, 3,
                    TAG_DONE)),
                    Child, Label((ULONG)GetMessage(MSG_WIDTH)),
                    Child,(ULONG)( ST_Width = MUI_NewObject(MUIC_String,
                      StringFrame,
                      MUIA_String_Accept, (ULONG) "0123456789",
                    TAG_DONE)),
                    Child, Label((ULONG)GetMessage(MSG_HEIGHT)),
                    Child,(ULONG)( ST_Height = MUI_NewObject(MUIC_String,
                      StringFrame,
                      MUIA_String_Accept, (ULONG) "0123456789",
                    TAG_DONE)),
                  TAG_DONE),
                  Child, HSpace(0),
                TAG_DONE),
                Child, UMUINO(MUIC_Group,MUIA_Group_Horiz,TRUE,
                  Child, HSpace(0),
                  Child, UMUINO(MUIC_Group,MUIA_Group_Columns,2,
                    MUIA_HorizWeight, 1000,
                    Child, Label((ULONG)GetMessage(MSG_SCREEN_TYPE)),
                    Child, CY_ScreenType = OMUINO(MUIC_Cycle,
                      MUIA_Cycle_Entries, (ULONG) ScreenTypes,
                    TAG_DONE),
                    Child, Label((ULONG)GetMessage(MSG_DIRECT_MODE)),
                    Child, CY_DirectMode = OMUINO(MUIC_Cycle,
                      MUIA_Cycle_Entries, (ULONG) DirectModes,
                    TAG_DONE),
                    Child, Label((ULONG)GetMessage(MSG_BUFFERING)),
                    Child, CY_Buffering = OMUINO(MUIC_Cycle,
                      MUIA_Cycle_Entries, (ULONG) Bufferings,
                    TAG_DONE),
                    Child, Label((ULONG)GetMessage(MSG_ROTATION)),
                    Child, CY_Rotation = OMUINO(MUIC_Cycle,
                      MUIA_Cycle_Entries, (ULONG) Rotations,
                    TAG_DONE),
                  TAG_DONE),
                  Child, HSpace(0),
                TAG_DONE),
                Child, UMUINO(MUIC_Group,MUIA_Group_Horiz,TRUE,
                  Child, HSpace(0),
                  Child, UMUINO(MUIC_Group,MUIA_Group_Columns,2,
                    MUIA_HorizWeight, 1000,

                    Child, Label((ULONG)GetMessage(MSG_SCREENMODE)),
                    Child,(ULONG)( PA_ScreenMode = MUI_NewObject(MUIC_Popasl,

                      MUIA_Popstring_String,(ULONG)( DisplayName = MUI_NewObject(MUIC_Text,
                        TextFrame,
                        MUIA_Background, MUII_TextBack,
                      TAG_DONE)),

                      MUIA_Popstring_Button,(ULONG)(PU_ScreenMode = PopButton(MUII_PopUp)),
                      MUIA_Popasl_Type,     ASL_ScreenModeRequest,
                      MUIA_Popasl_StartHook,  (ULONG) &ScreenModeStartHook,
                      MUIA_Popasl_StopHook, (ULONG) &ScreenModeStopHook,
                      MUIA_Disabled, (ULONG) (Config[CFG_SCREENTYPE] != CFGST_CUSTOM),
                    TAG_DONE)),
                  TAG_DONE),
                  Child, HSpace(0),
                TAG_DONE),
                Child, HVSpace,
              TAG_DONE),
              Child, UMUINO(MUIC_Group,
                Child, HVSpace,
                Child, UMUINO(MUIC_Group,MUIA_Group_Horiz,TRUE,
                  Child, HSpace(0),
                  Child, UMUINO(MUIC_Group,MUIA_Group_Columns,4,
                    MUIA_HorizWeight, 1000,
                    Child, Label((ULONG)GetMessage(MSG_SOUND)),
                    Child, CY_Sound =  OMUINO(MUIC_Cycle,
                      MUIA_Cycle_Entries, (ULONG) Sounds,
                    TAG_DONE),
                  TAG_DONE),
                  Child, HSpace(0),
                TAG_DONE),
                Child, UMUINO(MUIC_Group,MUIA_Group_Horiz,TRUE,
                  Child, MUI_NewObject(MUIC_Rectangle, TAG_DONE),
                  Child, UMUINO(MUIC_Group,MUIA_Group_Columns,2,
                    MUIA_HorizWeight, 1000,
                    Child, Label((ULONG)GetMessage(MSG_AUDIO_CH_0)),
                    Child, SL_AudioChannel[0] = OMUINO(MUIC_Slider,
                      MUIA_Slider_Min,    0,
                      MUIA_Slider_Max,    15,
                    TAG_DONE),
                    Child, Label((ULONG)GetMessage(MSG_AUDIO_CH_1)),
                    Child, SL_AudioChannel[1] = OMUINO(MUIC_Slider,
                      MUIA_Slider_Min,    0,
                      MUIA_Slider_Max,    15,
                    TAG_DONE),
                    Child, Label((ULONG)GetMessage(MSG_AUDIO_CH_2)),
                    Child, SL_AudioChannel[2] = OMUINO(MUIC_Slider,
                      MUIA_Slider_Min,    0,
                      MUIA_Slider_Max,    15,
                    TAG_DONE),
                    Child, Label((ULONG)GetMessage(MSG_AUDIO_CH_3)),
                    Child, SL_AudioChannel[3] = OMUINO(MUIC_Slider,
                      MUIA_Slider_Min,    0,
                      MUIA_Slider_Max,    15,
                    TAG_DONE),
                    Child, Label((ULONG)GetMessage(MSG_MIN_CHIP)),
                    Child, SL_MinFreeChip = OMUINO(MUIC_Slider,
                      MUIA_Slider_Min,    0,
                      MUIA_Slider_Max,    2048,
                    TAG_DONE),
                  TAG_DONE),
                  Child, MUI_NewObject(MUIC_Rectangle, TAG_DONE),
                TAG_DONE),
                Child, HVSpace,
              TAG_DONE),
              Child, UMUINO(MUIC_Group,
                Child, HVSpace,
                Child, UMUINO(MUIC_Group,MUIA_Group_Horiz,TRUE,
                  Child, HSpace(0),
                  Child, UMUINO(MUIC_Group,MUIA_Group_Columns,2,
                    GroupFrameT((ULONG)GetMessage(MSG_PRIMARY_CONTROLLER)),
                    MUIA_HorizWeight, 1000,
                    Child, Label((ULONG)GetMessage(MSG_TYPE)),
                    Child, CY_Joy1Type = OMUINO(MUIC_Cycle,
                      MUIA_Cycle_Entries, (ULONG) Joy1Types,
                    TAG_DONE),
                    Child, Label((ULONG)GetMessage(MSG_BUTTON_B_HOLD_TIME)),
                    Child, SL_Joy1ButtonBTime = OMUINO(MUIC_Slider,
                      MUIA_Slider_Min,    0,
                      MUIA_Slider_Max,    9,
                    TAG_DONE),
                    Child, Label((ULONG)GetMessage(MSG_AUTO_FIRE_RATE)),
                    Child, SL_Joy1AutoFireRate = OMUINO(MUIC_Slider,
                      MUIA_Slider_Min,    0,
                      MUIA_Slider_Max,    5,
                    TAG_DONE),
                  TAG_DONE),
                  Child, HSpace(0),
                TAG_DONE),
                Child, UMUINO(MUIC_Group,MUIA_Group_Horiz,TRUE,
                  Child, HSpace(0),
                  Child, UMUINO(MUIC_Group,MUIA_Group_Columns,2,
                    GroupFrameT((ULONG)GetMessage(MSG_SECONDARY_CONTROLLER)),
                    MUIA_HorizWeight, 1000,
                    Child, Label((ULONG)GetMessage(MSG_TYPE)),
                    Child, CY_Joy2Type = OMUINO(MUIC_Cycle,
                      MUIA_Cycle_Entries, (ULONG) Joy2Types,
                    TAG_DONE),
                    Child, Label((ULONG)GetMessage(MSG_BUTTON_B_HOLD_TIME)),
                    Child, SL_Joy2ButtonBTime = OMUINO(MUIC_Slider,
                      MUIA_Slider_Min,    0,
                      MUIA_Slider_Max,    9,
                    TAG_DONE),
                    Child, Label((ULONG)GetMessage(MSG_AUTO_FIRE_RATE)),
                    Child, SL_Joy2AutoFireRate = OMUINO(MUIC_Slider,
                      MUIA_Slider_Min,    0,
                      MUIA_Slider_Max,    5,
                    TAG_DONE),
                  TAG_DONE),
                  Child, HSpace(0),
                TAG_DONE),
                Child, HVSpace,
              TAG_DONE),
              Child, UMUINO(MUIC_Group,
                Child, HVSpace,
                Child, UMUINO(MUIC_Group,MUIA_Group_Horiz,TRUE,
                  Child, HSpace(0),
                  Child, UMUINO(MUIC_Group,MUIA_Group_Columns,2,
                    MUIA_HorizWeight, 1000,
                    Child, Label((ULONG)GetMessage(MSG_ROM_PATH)),
                    Child,(ULONG) (PA_RomPath = MUI_NewObject(MUIC_Popasl,
                      MUIA_Popstring_String,(ULONG)(ST_RomPath = OString(0, 256)),
                      MUIA_Popstring_Button, (ULONG)(PopButton(MUII_PopDrawer)),
                      ASLFR_DrawersOnly,    TRUE,
                    TAG_DONE)),
                    Child, Label((ULONG)GetMessage(MSG_SAMPLE_PATH)),
                    Child,(ULONG)(PA_SamplePath = MUI_NewObject(MUIC_Popasl,
                      MUIA_Popstring_String,(ULONG)(  ST_SamplePath = OString(0, 256)),
                      MUIA_Popstring_Button,(ULONG)(PopButton(MUII_PopDrawer)),
                      ASLFR_DrawersOnly,    TRUE,
                    TAG_DONE)),
                  TAG_DONE),
                  Child, HSpace(0),
                TAG_DONE),
                Child, HVSpace,
              TAG_DONE),
            TAG_DONE),
            Child, UMUINO(MUIC_Group,MUIA_Group_Horiz,TRUE,
              Child, BU_Start   = SimpleButton((ULONG)GetMessage(MSG_START)),
              Child, BU_Quit    = SimpleButton((ULONG)GetMessage(MSG_QUIT)),
            TAG_DONE),
          TAG_DONE);
//        TAG_DONE);


        if(MainWin)
        {
          DoMethod(App, OM_ADDMEMBER, MainWin);

          DoMethod(LV_Driver, MUIM_Notify,  MUIA_Listview_DoubleClick, TRUE,
                   App, 2, MUIM_Application_ReturnID,  RID_Start);

          DoMethod(BU_Start, MUIM_Notify, MUIA_Pressed, FALSE,
                   App, 2, MUIM_Application_ReturnID, RID_Start);

          DoMethod(BU_Quit, MUIM_Notify, MUIA_Pressed, FALSE,
                   App, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

          DoMethod(MainWin, MUIM_Notify,  MUIA_Window_CloseRequest, TRUE,
                   App, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

          DoMethod(CM_AutoFrameSkip, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
                   SL_FrameSkip, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);

          DoMethod(SL_Joy1AutoFireRate, MUIM_Notify, MUIA_Slider_Level, MUIV_EveryTime,
                   SL_Joy1ButtonBTime, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);

          DoMethod(SL_Joy2AutoFireRate, MUIM_Notify, MUIA_Slider_Level, MUIV_EveryTime,
                   SL_Joy2ButtonBTime, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);

          DoMethod(MainWin, MUIM_Notify, MUIA_Window_MenuAction, MUIV_EveryTime,
                   App, 2, MUIM_Application_ReturnID, MUIV_TriggerValue);

          DoMethod(CY_ScreenType, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
                   CY_ScreenType, 3, MUIM_CallHook, &ScreenTypeNotifyHook,  MUIV_TriggerValue);

          DoMethod(CY_DirectMode, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
                   CY_DirectMode, 3, MUIM_CallHook, &DirectModeNotifyHook, MUIV_TriggerValue);

          DoMethod(CY_Sound, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
                   CY_Sound, 3, MUIM_CallHook, &SoundNotifyHook, MUIV_TriggerValue);

          DoMethod(LI_Driver, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
                   LI_Driver, 3, MUIM_CallHook, &DriverNotifyHook, MUIV_TriggerValue);
#ifndef MESS
          DoMethod(CY_Show, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
                   CY_Show, 3, MUIM_CallHook, &ShowNotifyHook, MUIV_TriggerValue);

          DoMethod(CM_UseDefaults, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
                   CM_UseDefaults, 3, MUIM_CallHook, &UseDefaultsNotifyHook, MUIV_TriggerValue);

          DoMethod(BU_Scan, MUIM_Notify, MUIA_Pressed, FALSE,
                   App, 2, MUIM_Application_ReturnID, RID_Scan);
#endif
          DoMethod(MainWin,  MUIM_Window_SetCycleChain,  LV_Driver,
#ifndef MESS
                   CM_UseDefaults, CY_Show, BU_Scan,
#endif
                   BU_Start, BU_Quit, CM_Allow16Bit, CM_FlipX, CM_FlipY, CM_DirtyLines, CM_AutoFrameSkip,
                   CM_Antialiasing, CM_Translucency, SL_BeamWidth, SL_VectorFlicker, SL_FrameSkip,
                   ST_Width, ST_Height, CY_ScreenType, CY_DirectMode, CY_Buffering, CY_Rotation,
                   PU_ScreenMode, CY_Sound, SL_AudioChannel[0], SL_AudioChannel[1], SL_AudioChannel[2],
                   SL_AudioChannel[3], SL_MinFreeChip, CY_Joy1Type, SL_Joy1ButtonBTime, SL_Joy1AutoFireRate,
                   CY_Joy2Type, SL_Joy2ButtonBTime, SL_Joy2AutoFireRate, ST_RomPath, PA_RomPath,
                   ST_SamplePath, PA_SamplePath, RE_Options, NULL);

#ifdef MESS
          DoMethod(LI_Driver, MUIM_List_Insert, SortedDrivers, NumDrivers + DRIVER_OFFSET, MUIV_List_Insert_Bottom);
#else
          ShowNotify(NULL, NULL,(ULONG*) &Config[CFG_SHOW]);
#endif
        }
      }

      if(MainWin)
      {
        SetOptions(TRUE);

        set(MainWin,  MUIA_Window_ActiveObject, (ULONG) LV_Driver);
        set(MainWin,  MUIA_Window_Open,     TRUE);

        /* This must be done after the window has been opened. */

        DoMethod( LI_Driver, MUIM_List_Jump, MUIV_List_Jump_Active);

        do
        {
          rid = DoMethod(App,MUIM_Application_NewInput,&signals);

          switch(rid)
          {
            case RID_Start:
              get(LI_Driver, MUIA_List_Active, &v);

              if(v != MUIV_List_Active_Off)
              {
                GetOptions(TRUE);

                if(Config[CFG_DRIVER] >= 0)
                  loop = FALSE;
              }

              break;
#ifndef MESS
            case RID_Scan:
              GetOptions(FALSE);

              DoMethod(LI_Driver, MUIM_List_Clear);

              ScanDrivers();
              ShowFound();

              break;
#endif
            case MUIV_Application_ReturnID_Quit:
              GetOptions(TRUE);

              loop = FALSE;

              break;

            case MID_About:
              set(AboutWin, MUIA_Window_Open, TRUE);

              break;

            case RID_CloseAbout:
              set(AboutWin, MUIA_Window_Open, FALSE);

              break;

            case MID_AboutMUI:
              DoMethod(App, MUIM_Application_AboutMUI,  MainWin);

              break;
          }

          if(signals && loop)
          {
            signals = Wait(signals | SIGBREAKF_CTRL_C);

            if(signals & SIGBREAKF_CTRL_C)
              loop = FALSE;
          }
        } while(loop);

        set(AboutWin, MUIA_Window_Open, FALSE);
        set(MainWin,  MUIA_Window_Open, FALSE);

        if(rid == RID_Start)
          return(0);
      }
    }
  }
  return(1);
};

void AboutGUI(void)
{
  ULONG signals = 0;

  if(MUIMasterBase)
  {
    if(!App)
      CreateApp();
    if(App)
    {
      set(AboutWin, MUIA_Window_ActiveObject, (ULONG) BU_About_OK);
      set(AboutWin, MUIA_Window_Open, TRUE);

      for(;DoMethod(App, MUIM_Application_NewInput, &signals) != RID_CloseAbout;)
      {
        if(signals)
        {
          if(Inputs)
            signals = Wait(signals|SIGBREAKF_CTRL_C|Inputs->SignalMask);
          else
            signals = Wait(signals|SIGBREAKF_CTRL_C);

          if((signals & SIGBREAKF_CTRL_C))
            break;

          if(signals & Inputs->SignalMask)
            IUpdate(Inputs);
        }
      }

      set(AboutWin, MUIA_Window_Open, FALSE);
    }
  }
}

inline Object *OHCenter(Object *obj)
{
  return MUI_NewObject(MUIC_Group,
                       MUIA_Group_Horiz,TRUE,
                       MUIA_Group_Spacing,0,
                       Child,(ULONG) MUI_MakeObject(MUIO_HSpace,0),
                       Child, (ULONG)(obj),
                       Child,(ULONG) MUI_MakeObject(MUIO_HSpace,0),
                       TAG_DONE);
}


static void CreateApp(void)
{
  App = MUI_NewObject(MUIC_Application,
    MUIA_Application_Title      , (ULONG)APPNAME,
    MUIA_Application_Version    , (ULONG)("$VER: "APPNAME" ("REVDATE")"),
    MUIA_Application_Author     , (ULONG)AUTHOR,
    MUIA_Application_Base       , (ULONG)"MAME",
    SubWindow, (ULONG)(AboutWin = MUI_NewObject(MUIC_Window,
      MUIA_Window_Title, (ULONG)APPNAME,
      MUIA_Window_ID   , MAKE_ID('A','B','O','U'),
      WindowContents, UMUINO(MUIC_Group,
        Child, UMUINO(MUIC_Scrollgroup,
          MUIA_Scrollgroup_FreeHoriz, FALSE,
          MUIA_Scrollgroup_FreeVert, TRUE,
          MUIA_Scrollgroup_Contents, UMUINO(MUIC_Virtgroup,
            VirtualFrame,
            MUIA_Background, MUII_TextBack,
            Child, UMUINO(MUIC_Group,
              Child, UMUINO(MUIC_Text,
                MUIA_Text_Contents, (ULONG)TEXT_ABOUT,
              TAG_DONE),
              Child, VSpace(0),
            TAG_DONE),
          TAG_DONE),
          TAG_DONE),
        Child,(ULONG) OHCenter(BU_About_OK = SimpleButton((ULONG)"_OK")),
      TAG_DONE), // end group
    TAG_DONE)),
  TAG_DONE);

  if(App)
  {
    DoMethod(BU_About_OK, MUIM_Notify, MUIA_Pressed, FALSE,
             App, 2, MUIM_Application_ReturnID, RID_CloseAbout);

    DoMethod(AboutWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
             App, 2, MUIM_Application_ReturnID, RID_CloseAbout);
  }
}

static void GetOptions(BOOL get_driver)
{
  int config_index;

  get(CY_ScreenType,       MUIA_Cycle_Active,    &Config[CFG_SCREENTYPE]);
  get(CY_DirectMode,       MUIA_Cycle_Active,    &Config[CFG_DIRECTMODE]);
  get(CM_DirtyLines,       MUIA_Selected,        &Config[CFG_DIRTYLINES]);
  get(CM_Allow16Bit,       MUIA_Selected,        &Config[CFG_ALLOW16BIT]);
  get(CM_FlipX,            MUIA_Selected,        &Config[CFG_FLIPX]);
  get(CM_FlipY,            MUIA_Selected,        &Config[CFG_FLIPY]);
#ifndef MESS
  get(CY_Show,             MUIA_Cycle_Active,    &Config[CFG_SHOW]);
  get(CM_UseDefaults,      MUIA_Selected,        &Config[CFG_USEDEFAULTS]);
#endif
  get(CM_Antialiasing,     MUIA_Selected,        &Config[CFG_ANTIALIASING]);
  get(CM_Translucency,     MUIA_Selected,        &Config[CFG_TRANSLUCENCY]);
  get(CM_AutoFrameSkip,    MUIA_Selected,        &Config[CFG_AUTOFRAMESKIP]);
  get(SL_BeamWidth,        MUIA_Slider_Level,    &Config[CFG_BEAMWIDTH]);
  get(SL_VectorFlicker,    MUIA_Slider_Level,    &Config[CFG_VECTORFLICKER]);
  get(SL_FrameSkip,        MUIA_Slider_Level,    &Config[CFG_FRAMESKIP]);
  get(ST_Width,            MUIA_String_Integer,  &Config[CFG_WIDTH]);
  get(ST_Height,           MUIA_String_Integer,  &Config[CFG_HEIGHT]);
  get(CY_Buffering,        MUIA_Cycle_Active,    &Config[CFG_BUFFERING]);
  get(CY_Rotation,         MUIA_Cycle_Active,    &Config[CFG_ROTATION]);
  get(CY_Sound,            MUIA_Cycle_Active,    &Config[CFG_SOUND]);
  get(SL_AudioChannel[0],  MUIA_Slider_Level,    &Config[CFG_AUDIOCHANNEL0]);
  get(SL_AudioChannel[1],  MUIA_Slider_Level,    &Config[CFG_AUDIOCHANNEL1]);
  get(SL_AudioChannel[2],  MUIA_Slider_Level,    &Config[CFG_AUDIOCHANNEL2]);
  get(SL_AudioChannel[3],  MUIA_Slider_Level,    &Config[CFG_AUDIOCHANNEL3]);
  get(SL_MinFreeChip,      MUIA_Slider_Level,    &Config[CFG_MINFREECHIP]);
  get(CY_Joy1Type,         MUIA_Cycle_Active,    &Config[CFG_JOY1TYPE]);
  get(SL_Joy1ButtonBTime,  MUIA_Slider_Level,    &Config[CFG_JOY1BUTTONBTIME]);
  get(SL_Joy1AutoFireRate, MUIA_Slider_Level,    &Config[CFG_JOY1AUTOFIRERATE]);
  get(CY_Joy2Type,         MUIA_Cycle_Active,    &Config[CFG_JOY2TYPE]);
  get(SL_Joy2ButtonBTime,  MUIA_Slider_Level,    &Config[CFG_JOY2BUTTONBTIME]);
  get(SL_Joy2AutoFireRate, MUIA_Slider_Level,    &Config[CFG_JOY2AUTOFIRERATE]);
  get(ST_RomPath,          MUIA_String_Contents, &Config[CFG_ROMPATH]);
  get(ST_SamplePath,       MUIA_String_Contents, &Config[CFG_SAMPLEPATH]);
#ifdef POWERUP
  get(CM_AsyncPPC,         MUIA_Selected,        &Config[CFG_ASYNCPPC]);
#endif

  Config[CFG_SCREENMODE] = ScreenModeTags[SMT_DISPLAYID].ti_Data;
  Config[CFG_DEPTH]      = ScreenModeTags[SMT_DEPTH].ti_Data;

  if(get_driver)
    Config[CFG_DRIVER] = GetDriverIndex();

  config_index = DRIVER_OFFSET + ((LONG) Config[CFG_DRIVER]);

  if(config_index >= 0)
    SetConfig(config_index, Config);
}

static void SetOptions(BOOL set_driver)
{
  ULONG i, v;

  GetConfig(Config[CFG_DRIVER] + DRIVER_OFFSET, Config);

#ifndef MESS
  if(Config[CFG_DRIVER] < 0)
    Config[CFG_USEDEFAULTS] = FALSE;

  set(CY_Show,             MUIA_Cycle_Active,    Config[CFG_SHOW]);
  set(CM_UseDefaults,      MUIA_Selected,        Config[CFG_USEDEFAULTS]);
#endif
  set(CY_ScreenType,       MUIA_Cycle_Active,    Config[CFG_SCREENTYPE]);
  set(CY_DirectMode,       MUIA_Cycle_Active,    Config[CFG_DIRECTMODE]);
  set(CM_DirtyLines,       MUIA_Selected,        Config[CFG_DIRTYLINES]);
  set(CM_Allow16Bit,       MUIA_Selected,        Config[CFG_ALLOW16BIT]);
  set(CM_FlipX,            MUIA_Selected,        Config[CFG_FLIPX]);
  set(CM_FlipY,            MUIA_Selected,        Config[CFG_FLIPY]);
  set(CM_Antialiasing,     MUIA_Selected,        Config[CFG_ANTIALIASING]);
  set(CM_Translucency,     MUIA_Selected,        Config[CFG_TRANSLUCENCY]);
  set(CM_AutoFrameSkip,    MUIA_Selected,        Config[CFG_AUTOFRAMESKIP]);
  set(SL_BeamWidth,        MUIA_Slider_Level,    Config[CFG_BEAMWIDTH]);
  set(SL_VectorFlicker,    MUIA_Slider_Level,    Config[CFG_VECTORFLICKER]);
  set(SL_FrameSkip,        MUIA_Slider_Level,    Config[CFG_FRAMESKIP]);
  set(ST_Width,            MUIA_String_Integer,  Config[CFG_WIDTH]);
  set(ST_Height,           MUIA_String_Integer,  Config[CFG_HEIGHT]);
  set(CY_Buffering,        MUIA_Cycle_Active,    Config[CFG_BUFFERING]);
  set(CY_Rotation,         MUIA_Cycle_Active,    Config[CFG_ROTATION]);
  set(CY_Sound,            MUIA_Cycle_Active,    Config[CFG_SOUND]);
  set(SL_AudioChannel[0],  MUIA_Slider_Level,    Config[CFG_AUDIOCHANNEL0]);
  set(SL_AudioChannel[1],  MUIA_Slider_Level,    Config[CFG_AUDIOCHANNEL1]);
  set(SL_AudioChannel[2],  MUIA_Slider_Level,    Config[CFG_AUDIOCHANNEL2]);
  set(SL_AudioChannel[3],  MUIA_Slider_Level,    Config[CFG_AUDIOCHANNEL3]);
  set(SL_MinFreeChip,      MUIA_Slider_Level,    Config[CFG_MINFREECHIP]);
  set(CY_Joy1Type,         MUIA_Cycle_Active,    Config[CFG_JOY1TYPE]);
  set(SL_Joy1ButtonBTime,  MUIA_Slider_Level,    Config[CFG_JOY1BUTTONBTIME]);
  set(SL_Joy1AutoFireRate, MUIA_Slider_Level,    Config[CFG_JOY1AUTOFIRERATE]);
  set(CY_Joy2Type,         MUIA_Cycle_Active,    Config[CFG_JOY2TYPE]);
  set(SL_Joy2ButtonBTime,  MUIA_Slider_Level,    Config[CFG_JOY2BUTTONBTIME]);
  set(SL_Joy2AutoFireRate, MUIA_Slider_Level,    Config[CFG_JOY2AUTOFIRERATE]);
  set(ST_RomPath,          MUIA_String_Contents, Config[CFG_ROMPATH]);
  set(ST_SamplePath,       MUIA_String_Contents, Config[CFG_SAMPLEPATH]);
#ifdef POWERUP
  set(CM_AsyncPPC,         MUIA_Selected,        Config[CFG_ASYNCPPC]);
#endif

  ScreenModeTags[SMT_DISPLAYID].ti_Data = Config[CFG_SCREENMODE];
  ScreenModeTags[SMT_DEPTH].ti_Data     = Config[CFG_DEPTH];

  SetDisplayName(Config[CFG_SCREENMODE]);

  if((Config[CFG_DRIVER] < 0) || (!Config[CFG_USEDEFAULTS]
     && /*(Drivers[Config[CFG_DRIVER]]->drv->video_attributes & VIDEO_SUPPORTS_16BIT)*/
       ((Drivers[Config[CFG_DRIVER]]->flags & GAME_REQUIRES_16BIT) ||
        (Drivers[Config[CFG_DRIVER]]->drv->total_colors > 256)
       )
     ))
  {
    set(CM_Allow16Bit, MUIA_Disabled, FALSE);
  }
  else
    set(CM_Allow16Bit, MUIA_Disabled, TRUE);

  if(set_driver)
  {
#ifdef MESS
    if(Config[CFG_DRIVER] < 0)
      Config[CFG_DRIVER] = 0;

    for(i = 0; ; i++)
#else
    if(Config[CFG_DRIVER] == -2)
      set(LI_Driver,  MUIA_List_Active, 0);
    else if(Config[CFG_DRIVER] == -1)
      set(LI_Driver,  MUIA_List_Active, 1);
    else
    {
      for(i = 2; ; i++)
#endif
      {
        DoMethod(LI_Driver, MUIM_List_GetEntry, i, &v);

        if(!v)
          break;

        if(v == (ULONG) &Drivers[Config[CFG_DRIVER]])
        {
          set(LI_Driver,  MUIA_List_Active, i);
          break;
        }
      }
#ifndef MESS
    }
#endif
  }
}

static void SetDisplayName(ULONG displayid)
{
    if(!DisplayName) return;
  if(displayid == INVALID_ID)
    DisplayNameBuffer = GetMessage(MSG_INVALID);
  else
  {
    LONG v;
    struct NameInfo DisplayNameInfo;
    v = GetDisplayInfoData(NULL, (UBYTE *) &DisplayNameInfo, sizeof(DisplayNameInfo),
                         DTAG_NAME, displayid);

    if(v > sizeof(struct QueryHeader))
    {
        DisplayNameBuffer = (const char *) DisplayNameInfo.Name;
    }
    char temp[16];
    snprintf(temp,15," (0x%08x)",(int)displayid);
    DisplayNameBuffer += temp;
  }

  set(DisplayName, MUIA_Text_Contents, (ULONG) DisplayNameBuffer.c_str());
}

static ULONG ASM ScreenModeStart(struct Hook *hook REG(a0), APTR popasl REG(a2), struct TagItem *taglist REG(a1))
{
  LONG  i;

  for(i = 0; taglist[i].ti_Tag != TAG_END; i++);

  taglist[i].ti_Tag = TAG_MORE;
  taglist[i].ti_Data  = (ULONG) ScreenModeTags;

  return(TRUE);
}

static ULONG ASM ScreenModeStop(struct Hook *hook REG(a0), APTR popasl REG(a2), struct ScreenModeRequester *smreq REG(a1))
{
  ScreenModeTags[SMT_DISPLAYID].ti_Data = smreq->sm_DisplayID;
  ScreenModeTags[SMT_DEPTH].ti_Data   = smreq->sm_DisplayDepth;

  SetDisplayName(smreq->sm_DisplayID);

  return(0);
}

#ifndef MESS
static ULONG ASM ShowNotify(struct Hook *hook REG(a0), APTR obj REG(a2), ULONG *par REG(a1))
{
  DoMethod(LI_Driver, MUIM_List_Clear);

  switch(*par)
  {
    case CFGS_ALL:
      set(BU_Scan, MUIA_Disabled, TRUE);
      DoMethod(LI_Driver, MUIM_List_Insert, SortedDrivers, NumDrivers + DRIVER_OFFSET, MUIV_List_Insert_Bottom);
      break;

    case CFGS_FOUND:
      set(BU_Scan, MUIA_Disabled, FALSE);
      ShowFound();
      break;
  }

  return(0);
}
#endif

static ULONG ASM ScreenTypeNotify(struct Hook *hook REG(a0), APTR obj REG(a2), ULONG *par REG(a1))
{
  if(*par == CFGST_CUSTOM)
  {
    set(PA_ScreenMode,  MUIA_Disabled, FALSE);
    set(PU_ScreenMode,  MUIA_Disabled, FALSE);
  }
  else
  {
    set(PA_ScreenMode,  MUIA_Disabled, TRUE);
    set(PU_ScreenMode,  MUIA_Disabled, TRUE);
  }

  return(0);
}

static ULONG ASM DirectModeNotify(struct Hook *hook REG(a0), APTR obj REG(a2), ULONG *par REG(a1))
{
  if(*par == CFGDM_DRAW)
    set(CM_DirtyLines, MUIA_Disabled, TRUE);
  else
    set(CM_DirtyLines, MUIA_Disabled, FALSE);

  return(0);
}

static ULONG ASM SoundNotify(struct Hook *hook REG(a0), APTR obj REG(a2), ULONG *par REG(a1))
{
  switch(*par)
  {
    case 1:
      set(SL_AudioChannel[0], MUIA_Disabled, FALSE);
      set(SL_AudioChannel[1], MUIA_Disabled, FALSE);
      set(SL_AudioChannel[2], MUIA_Disabled, FALSE);
      set(SL_AudioChannel[3], MUIA_Disabled, FALSE);
      set(SL_MinFreeChip, MUIA_Disabled, FALSE);
      break;
    default:
      set(SL_AudioChannel[0], MUIA_Disabled, TRUE);
      set(SL_AudioChannel[1], MUIA_Disabled, TRUE);
      set(SL_AudioChannel[2], MUIA_Disabled, TRUE);
      set(SL_AudioChannel[3], MUIA_Disabled, TRUE);
      set(SL_MinFreeChip, MUIA_Disabled, TRUE);
      break;
  }

  return(0);
}

static ULONG ASM DriverNotify(struct Hook *hook REG(a0), APTR obj REG(a2), ULONG *par REG(a1))
{
  GetOptions(FALSE);

  if(*par < DRIVER_OFFSET)
  {
    set(CM_UseDefaults, MUIA_Disabled, TRUE);
    set(CM_Allow16Bit,  MUIA_Disabled, FALSE);
  }
  else
  {
    set(CM_UseDefaults, MUIA_Disabled, FALSE);
  }

  Config[CFG_DRIVER] = GetDriverIndex();

  SetOptions(FALSE);

  return(0);
}

#ifndef MESS
static ULONG ASM UseDefaultsNotify(struct Hook *hook REG(a0), APTR obj REG(a2), ULONG *par REG(a1))
{
  struct GameDriver *drv;

  ULONG v;

  if(*par)
  {
    set(CY_ScreenType,       MUIA_Disabled, TRUE);
    set(CY_DirectMode,       MUIA_Disabled, TRUE);
    set(CM_Allow16Bit,       MUIA_Disabled, TRUE);
    set(CM_FlipX,            MUIA_Disabled, TRUE);
    set(CM_FlipY,            MUIA_Disabled, TRUE);
    set(CM_Antialiasing,     MUIA_Disabled, TRUE);
    set(CM_Translucency,     MUIA_Disabled, TRUE);
    set(CM_AutoFrameSkip,    MUIA_Disabled, TRUE);
    set(SL_BeamWidth,        MUIA_Disabled, TRUE);
    set(SL_VectorFlicker,    MUIA_Disabled, TRUE);
    set(SL_FrameSkip,        MUIA_Disabled, TRUE);
    set(ST_Width,            MUIA_Disabled, TRUE);
    set(ST_Height,           MUIA_Disabled, TRUE);
    set(CY_Buffering,        MUIA_Disabled, TRUE);
    set(CY_Rotation,         MUIA_Disabled, TRUE);
    set(CY_Sound,            MUIA_Disabled, TRUE);
    set(SL_MinFreeChip,      MUIA_Disabled, TRUE);
    set(CY_Joy1Type,         MUIA_Disabled, TRUE);
    set(SL_Joy1ButtonBTime,  MUIA_Disabled, TRUE);
    set(SL_Joy1AutoFireRate, MUIA_Disabled, TRUE);
    set(CY_Joy2Type,         MUIA_Disabled, TRUE);
    set(SL_Joy2ButtonBTime,  MUIA_Disabled, TRUE);
    set(SL_Joy2AutoFireRate, MUIA_Disabled, TRUE);
    set(ST_RomPath,          MUIA_Disabled, TRUE);
    set(ST_SamplePath,       MUIA_Disabled, TRUE);
    set(PA_RomPath,          MUIA_Disabled, TRUE);
    set(PA_SamplePath,       MUIA_Disabled, TRUE);
    set(CM_DirtyLines,       MUIA_Disabled, TRUE);
    set(PU_ScreenMode,       MUIA_Disabled, TRUE);
    set(PA_ScreenMode,       MUIA_Disabled, TRUE);
    set(SL_AudioChannel[0],  MUIA_Disabled, TRUE);
    set(SL_AudioChannel[1],  MUIA_Disabled, TRUE);
    set(SL_AudioChannel[2],  MUIA_Disabled, TRUE);
    set(SL_AudioChannel[3],  MUIA_Disabled, TRUE);
#ifdef POWERUP
    set(CM_AsyncPPC,         MUIA_Disabled, TRUE);
#endif
  }
  else
  {
    set(CY_ScreenType,       MUIA_Disabled, FALSE);
    set(CY_DirectMode,       MUIA_Disabled, FALSE);
    set(CM_FlipX,            MUIA_Disabled, FALSE);
    set(CM_FlipY,            MUIA_Disabled, FALSE);
    set(CM_Antialiasing,     MUIA_Disabled, FALSE);
    set(CM_Translucency,     MUIA_Disabled, FALSE);
    set(CM_AutoFrameSkip,    MUIA_Disabled, FALSE);
    set(SL_BeamWidth,        MUIA_Disabled, FALSE);
    set(SL_VectorFlicker,    MUIA_Disabled, FALSE);
    set(SL_FrameSkip,        MUIA_Disabled, FALSE);
    set(ST_Width,            MUIA_Disabled, FALSE);
    set(ST_Height,           MUIA_Disabled, FALSE);
    set(CY_Buffering,        MUIA_Disabled, FALSE);
    set(CY_Rotation,         MUIA_Disabled, FALSE);
    set(CY_Sound,            MUIA_Disabled, FALSE);
    set(SL_MinFreeChip,      MUIA_Disabled, FALSE);
    set(CY_Joy1Type,         MUIA_Disabled, FALSE);
    set(SL_Joy1ButtonBTime,  MUIA_Disabled, FALSE);
    set(SL_Joy1AutoFireRate, MUIA_Disabled, FALSE);
    set(CY_Joy2Type,         MUIA_Disabled, FALSE);
    set(SL_Joy2ButtonBTime,  MUIA_Disabled, FALSE);
    set(SL_Joy2AutoFireRate, MUIA_Disabled, FALSE);
    set(ST_RomPath,          MUIA_Disabled, FALSE);
    set(ST_SamplePath,       MUIA_Disabled, FALSE);
    set(PA_RomPath,          MUIA_Disabled, FALSE);
    set(PA_SamplePath,       MUIA_Disabled, FALSE);
#ifdef POWERUP
    set(CM_AsyncPPC,         MUIA_Disabled, FALSE);
#endif

    get(CY_ScreenType, MUIA_Cycle_Active, &v);
    ScreenTypeNotify(&ScreenTypeNotifyHook, CY_ScreenType, &v);

    get(CY_DirectMode, MUIA_Cycle_Active,  &v);
    DirectModeNotify(&DirectModeNotifyHook, CY_DirectMode, &v);

    get(CY_Sound, MUIA_Cycle_Active, &v);
    SoundNotify(&SoundNotifyHook, CY_Sound, &v);

    drv = GetDriver();

    //0.35 if(!drv || (drv->drv->video_attributes & VIDEO_SUPPORTS_16BIT))
    if(!drv || drv->drv->total_colors<256 || (drv->flags & GAME_REQUIRES_16BIT)==0)
      set(CM_Allow16Bit, MUIA_Disabled, FALSE);
    else
      set(CM_Allow16Bit, MUIA_Disabled, TRUE);
  }

  return(0);
}
#endif
