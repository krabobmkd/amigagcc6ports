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
#ifndef __stdargs
// shut up clangd.
#define __stdargs
#endif

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>


// #include <proto/cybergraphics.h>
extern "C" {
#include <cybergraphx/cybergraphics.h>
}

// mame includes
extern "C" {
    #include "mamecore.h"
    #include "mame.h"
    #include "driver.h"
    #include "osdepend.h"
    #include "unzip.h"
}

#include <stdio.h>
#include <string>
#include <vector>


#include "main.h"
#include "config_moo.h"

// from mame since 0.37:
#include "input.h"


//#include "zlib.h"

#define INTELuint32_t(i) (((i)<<24)|((i)>>24)|(((i)<<8)&0x00ff0000)|(((i)>>8)&0x0000ff00))

mame_bitmap *BitMap=NULL;
LONG ClearBitMap;

static LONG  UserInterface;

extern FILE *errorlog;
extern void *record;
extern void *playback;

static uint8_t Palette[256][3];

static int FrameCounter;
static const int NoFrameSkipCount = 10;

std::string ROMZipName,SampleZipName;

static int  ShowFPS;

static int on_screen_display_timer;

uint8_t *DirtyLines[6];

// patch in mixer.cpp:
int usestereo=0;

int frameskip = 0;
int autoframeskip = 0;
int throttle = 0;
int video_sync  = 0;
unsigned char No_FM   = 1;

static int input_update_counter = 0;

// vector things
//extern "C" {
//    extern int antialias;
//    extern int beam;
//    extern int flicker;
//    extern int translucency;
//}

#ifndef ORIENTATION_DEFAULT
#define ORIENTATION_DEFAULT 0
#endif
void unzip_cache_clear();
void setRomPaths(std::vector<std::string> &extrarompaths,std::vector<std::string> &extrasamplepaths);

inline void initOptions()
{
    // consider everything null by default.
    //note: 0 for brightness treated as 1.
    memset(&options, 0,sizeof(global_options));

   options.cheat=1;
   options.gui_host=1;

   options.samplerate=(Config[CFG_SOUND] == CFGS_NO)?0:22050;
    Machine->sample_rate = options.samplerate;

//re?   options.use_samples=1; //TODO ?

//   options.ror        = (Config[CFG_ROTATION] == CFGR_RIGHT);
//   options.rol        = (Config[CFG_ROTATION] == CFGR_RIGHT);
//   options.flipx      = Config[CFG_FLIPX];
//   options.flipy      = Config[CFG_FLIPY];



}


void StartGame(void)
{
  throttle = 1;

  printf("StartGame1\n");

  initOptions();


  // options.record=NULL;
  // options.playback=NULL;
  // options.language_file=NULL; /* LBO 042400 */

// all are "int"
//   options.mame_debug=0;

 // options.errorlog = NULL;

// O -> originaly commented.
// DONE -> ported up.

/*O  if(Options[OPT_RECORD])
    options.record = osd_fopen( drivers[Options[OPT_GAME]]->name, (const char *) Options[OPT_RECORD],
                  OSD_FILETYPE_INPUTLOG, 1);
  else*/
   // options.record = NULL;

/*O  if(Options[OPT_PLAYBACK])
    options.playback = osd_fopen( drivers[Options[OPT_GAME]]->name, (const char *) Options[OPT_PLAYBACK],
                    OSD_FILETYPE_INPUTLOG, 0);
  else*/
   // options.playback = NULL;
/*DONE
  if(Config[CFG_SOUND] == CFGS_NO)
    options.samplerate  = 0;
  else
    options.samplerate  = 22000;

  Machine->sample_rate = options.samplerate;
*/
//old  options.samplebits = 8;
//done  options.mame_debug = 0;
//done  options.cheat      = 1;
//done  options.norotate   = 0;

 //ok
  frameskip = Config[CFG_FRAMESKIP];
/* vector things removed
  antialias    = Config[CFG_ANTIALIASING];
  translucency = Config[CFG_TRANSLUCENCY];

  beam = Config[CFG_BEAMWIDTH] * 0x00010000;
  if(beam < 0x00010000)
    beam = 0x00010000;
  if(beam > 0x00100000)
    beam = 0x00100000;

  flicker = (int)(Config[CFG_VECTORFLICKER] * 2.55);
  if(flicker < 0)
    flicker = 0;
  if(flicker > 255)
    flicker = 255;
*/
#ifdef MESS
  for(i = 0; i < MAX_ROM; i++)
  {
    options.rom_name[i][0] = 0;

    if((i == 0) && Config[CFG_ROM])
      strcpy(options.rom_name[0], (char *) Config[CFG_ROM]);
  }

  for(i = 0; i < MAX_FLOPPY; i++)
    options.floppy_name[i][0] = 0;

  for(i = 0; i < MAX_HARD; i++)
    options.hard_name[i][0] = 0;

  for(i = 0; i < MAX_CASSETTE; i++)
    options.cassette_name[i][0] = 0;
#endif

  // krb2024: set list of search path for rom
  int path_num=0;
  const char *path;
  std::vector<std::string> rompathlist,samplepathlist;
  for(path_num = 0;
      (path = GetRomPath(Config[CFG_DRIVER], path_num)) != NULL;
      path_num++)
  {
        if(*path != 0)
        {
            rompathlist.push_back(std::string(path));
            printf("pathtotest:%s:\n",path);
        }
  }
  setRomPaths(rompathlist,samplepathlist);


  printf("StartGame2\n");
  /* Clear the zip filename caches. */

  ROMZipName.clear();
  SampleZipName.clear();
  ShowFPS          = 0;
  printf("StartGame2b\n");
  FrameCounter = 0;

  osd_set_mastervolume(0);
  printf("before run_game\n");

  run_game(Config[CFG_DRIVER]);
  printf("after run_game\n");

  unzip_cache_clear();

//todo ?
//  if(options.playback)
//    osd_fclose(options.playback);
  
//  if(options.record)
//    osd_fclose(options.record);
}

int osd_init()
{
  return(0);
}

void osd_exit()
{
}

//mame_bitmap *osd_new_bitmap(int width, int height, int depth)
/*
mame_bitmap *osd_alloc_bitmap(int width,int height,int depth)
{
  mame_bitmap *bitmap;
  unsigned char   *line;
  LONG        safety;
  LONG        i, w;

  TRACE_ENTER("osd_new_bitmap");

//  if(Machine->orientation & ORIENTATION_SWAP_XY)
//  {
//    w   = width;
//    width = height;
//    height  = w;
//  }

  if(width > 32)
    safety = 8;
  else
    safety = 0;

  if(depth != 16)
  {
    depth = 8;
    w = ((width + 2 * safety + 3) >> 2) << 2;
  }
  else
    w = ((2 * (width + 2 * safety) + 3) >> 2) << 2;

  bitmap = (mame_bitmap *) calloc(  sizeof(mame_bitmap)
                      + height*sizeof(unsigned char *)
                      + w*(height+2*safety)*sizeof(unsigned char), 1);

  if(bitmap)
  {
    bitmap->width    = width;
    bitmap->height   = height;
    bitmap->depth    = depth;
//    bitmap->_private = (void *) w;
    bitmap->line     = (unsigned char **) &bitmap[1]; 

    line = ((unsigned char *) &bitmap->line[height]) + safety * w;

    for(i = 0; i < height; i++)
    {
      bitmap->line[i] = line;
      line += w*sizeof(unsigned char);
    }
  }

  TRACE_LEAVE("osd_new_bitmap");

  return(bitmap);
}

void osd_free_bitmap(mame_bitmap *bitmap)
{
  TRACE_ENTER("osd_free_bitmap");

  if(bitmap)
    free(bitmap);

  TRACE_LEAVE("osd_free_bitmap");
}

void osd_clearbitmap(mame_bitmap *bitmap)
{
  int i;

  TRACE_ENTER("osd_clearbitmap");

  for (i = 0;i < bitmap->height;i++)
    memset(bitmap->line[i],0,bitmap->width);

  if(bitmap == BitMap)
  {
#ifdef POWERUP
    ClearBitMap = (ClearBitMap + 1) & 1;
#endif
    if(DirtyLines[0])
      memset(DirtyLines[0], 1, BitMap->height);
  }

  TRACE_LEAVE("osd_clearbitmap");
}

void osd_mark_dirty(int x1, int y1, int x2, int y2, int ui)
{
  if(DirtyLines[0] && (y1 < BitMap->height) && (y2 >= 0))
  {
    if(y1 < 0)
      y1 = 0;
    if(y2 >= BitMap->height)
      y2 = BitMap->height-1;
    memset(&DirtyLines[0][y1], 1, y2 - y1 + 1);
  }

  if(ui)
    UserInterface = TRUE;
}
*/

//void osd_save_snapshot(mame_bitmap *bitmap)
//{

//}

/* called while loading ROMs. It is called a last time with name == 0 to signal */
/* that the ROM loading process is finished. */
/* return non-zero to abort loading */
int osd_display_loading_rom_message(const char *name,rom_load_data *romdata)
{
  return(0);
}
// -  - - - -

/* called when the game is paused/unpaused, so the OS dependant code can do special */
/* things like changing the title bar or darkening the display. */
/* Note that the OS dependant code must NOT stop processing input, since the user */
/* interface is still active while the game is paused. */
void osd_pause(int paused)
{

}



