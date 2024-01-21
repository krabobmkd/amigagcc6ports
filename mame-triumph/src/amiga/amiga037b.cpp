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
#include <string>

extern "C" {
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <cybergraphx/cybergraphics.h>
}
#include <stdio.h>

#include "mame.h"
#include "driver.h"
#include "osdepend.h"

#include "main.h"
#include "config.h"
#include "file.h"
#include "audio.h"
// from mame since 0.37:
#include "input.h"

#include "zlib.h"

#define INTELuint32_t(i) (((i)<<24)|((i)>>24)|(((i)<<8)&0x00ff0000)|(((i)>>8)&0x0000ff00))

#ifdef POWERUP
extern LONG FrameSkip;
extern LONG FPS;

#define VGetFrameSkip(video) FrameSkip
#define VGetFPS(video)       FPS

#define VSetFrameSkip(video,frameskip)
#define VSetLimitSpeed(video,limitspeed)
#endif

struct osd_bitmap *BitMap=NULL;
LONG ClearBitMap;

static LONG  UserInterface;

extern FILE *errorlog;
extern void *record;
extern void *playback;

static uint8_t Palette[256][3];

static int MasterVolume;
static int Attenuation;

static int FrameCounter;
static const int NoFrameSkipCount = 10;

//static char ROMZipName[256];
//static char SampleZipName[256];
std::string ROMZipName,SampleZipName;

static int  ShowFPS;

static int on_screen_display_timer;

uint8_t *DirtyLines[6];

int frameskip = 0;
int autoframeskip = 0;
int throttle = 0;
int video_sync  = 0;
unsigned char No_FM   = 1;

static int input_update_counter = 0;

extern int antialias;
extern int beam;
extern int flicker;
extern int translucency;

int load_zipped_file(const char *zipfile, const char *filename, unsigned char **buf, unsigned int *length);

//unsigned int crc32(unsigned int crc, const unsigned char *buf, unsigned int len);

#ifndef ORIENTATION_DEFAULT
#define ORIENTATION_DEFAULT 0
#endif

void StartGame(void)
{
#ifdef MESS
  int i;
#endif

  throttle = 1;

    // consider evrything null by default.
    memset(&options, 0,sizeof(struct GameOptions));

  // options.record=NULL;
  // options.playback=NULL;
  // options.language_file=NULL; /* LBO 042400 */

// all are "int"
//   options.mame_debug=0;
   options.cheat=1;
   options.gui_host=0; //? not done.

   options.samplerate=(Config[CFG_SOUND] == CFGS_NO)?0:22050;
    Machine->sample_rate = options.samplerate;

   options.use_samples=1; //TODO ?
   options.use_emulated_ym3812=0;

   options.color_depth=0;	/* 8 or 16, any other value means auto */
   options.vector_width=0;	/* requested width for vector games=0; 0 means default (640) */
   options.vector_height=0;	/* requested height for vector games=0; 0 means default (480) */
   options.norotate=0; //OK

   options.ror        = (Config[CFG_ROTATION] == CFGR_RIGHT);
   options.rol        = (Config[CFG_ROTATION] == CFGR_RIGHT);
   options.flipx      = Config[CFG_FLIPX];
   options.flipy      = Config[CFG_FLIPY];


   options.beam=0;
   options.flicker=0;
   options.translucency=0;
   options.antialias=0;
   options.use_artwork=0;

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

  /* Clear the zip filename caches. */

  ROMZipName.clear();
  SampleZipName.clear();
  ShowFPS          = 0;

  FrameCounter = 0;

  osd_set_mastervolume(0);

  run_game(Config[CFG_DRIVER]);

  if(options.playback)
    osd_fclose(options.playback);
  
  if(options.record)
    osd_fclose(options.record);
}

int osd_init()
{
  return(0);
}

void osd_exit()
{
}

struct osd_bitmap *osd_new_bitmap(int width, int height, int depth)
{
  struct osd_bitmap *bitmap;
  unsigned char   *line;
  LONG        safety;
  LONG        i, w;

  TRACE_ENTER("osd_new_bitmap");

  if(Machine->orientation & ORIENTATION_SWAP_XY)
  {
    w   = width;
    width = height;
    height  = w;
  }

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

  bitmap = (struct osd_bitmap *) calloc(  sizeof(struct osd_bitmap)
                      + height*sizeof(unsigned char *)
                      + w*(height+2*safety)*sizeof(unsigned char), 1);

  if(bitmap)
  {
    bitmap->width    = width;
    bitmap->height   = height;
    bitmap->depth    = depth;
    bitmap->_private = (void *) w;
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

void osd_free_bitmap(struct osd_bitmap *bitmap)
{
  TRACE_ENTER("osd_free_bitmap");

  if(bitmap)
    free(bitmap);

  TRACE_LEAVE("osd_free_bitmap");
}

void osd_clearbitmap(struct osd_bitmap *bitmap)
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

/* palette is an array of 'totalcolors' R,G,B triplets. The function returns */
/* in *pens the pen values corresponding to the requested colors. */
/* If 'totalcolors' is 32768, 'palette' is ignored and the *pens array is filled */
/* with pen values corresponding to a 5-5-5 15-bit palette */

static inline short makecol(int r, int g, int b)
{
  switch(PixelArray[0]->PixelFormat)
  {
    case PIXFMT_RGB15:
      return((r&0xf8)<<7)|((g&0xf8)<<2)|(b>>3);
      break;

    case PIXFMT_BGR15:
      return((b&0xf8)<<7)|((g&0xf8)<<2)|(r>>3);
      break;

    case PIXFMT_RGB15PC:
      return((r&0xf8)>>1)|(g>>6)|((g&0x38)<<10)|((b&0xf8)<<5);
      break;

    case PIXFMT_BGR15PC:
      return((b&0xf8)>>1)|(g>>6)|((g&0x38)<<10)|((r&0xf8)<<5);
      break;

    case PIXFMT_BGR16:
      return((b&0xf8)<<8)|((g&0xfc)<<3)|(r>>3);
      break;

    case PIXFMT_RGB16PC:
      return(r&0xf8)|(g>>5)|((g&0x1c)<<11)|((b&0xf8)<<5);
      break;

    case PIXFMT_BGR16PC:
      return(b&0xf8)|(g>>5)|((g&0x1c)<<11)|((r&0xf8)<<5);
      break;

    case PIXFMT_RGB16:
    default:
      return((r&0xf8)<<8)|((g&0xfc)<<3)|(b>>3);
      break;
  }
}


/*
  osd_allocate_colors() is called after osd_create_display(), to create and initialize
  the palette.
  palette is an array of 'totalcolors' R,G,B triplets. The function returns
  in *pens the pen values corresponding to the requested colors.
  When modifiable is not 0, the palette will be modified later via calls to
  osd_modify_pen(). Otherwise, the code can assume that the palette will not change,
  and activate special optimizations (e.g. direct copy for a 16-bit display).
  The function must also initialize Machine->uifont->colortable[] to get proper
  white-on-black and black-on-white text.
  Return 0 for success.
*/
int osd_allocate_colors(unsigned int total_colors,const unsigned char *palette,unsigned short *pens,int modifiable)
//olde void osd_allocate_colors(unsigned int total_colors,const unsigned char *palette,unsigned short *pens)
{
  TRACE_ENTER("osd_allocate_colors");

  if(total_colors == 32768)
  {
    int r,g,b;

    for (r = 0; r < 32; r++)
    {
      for (g = 0; g < 32; g++)
      {
        for (b = 0; b < 32; b++)
        {
					*pens++ = makecol(r<<3,g<<3,b<<3);
        }
      }
    }

		Machine->uifont->colortable[0] = makecol(0x00,0x00,0x00);
		Machine->uifont->colortable[1] = makecol(0xff,0xff,0xff);
		Machine->uifont->colortable[2] = makecol(0xff,0xff,0xff);
		Machine->uifont->colortable[3] = makecol(0x00,0x00,0x00);
  }
  else
  {
    int i;
    int best_black;
    int best_white;
    int best_black_score;
    int best_white_score;

    best_black       = 0;
    best_white       = 0;
    best_black_score = 3*255*255;
    best_white_score = 0;

    for(i = 0; i < total_colors; i++)
    {
      int r,g,b,score;

      pens[i] = i;

      r = palette[3*i];
      g = palette[3*i+1];
      b = palette[3*i+2];

      score = r*r + g*g + b*b;

      if(score < best_black_score)
      {
        best_black       = i;
        best_black_score = score;
      }

      if(score > best_white_score)
      {
        best_white       = i;
        best_white_score = score;
      }
    }

    Machine->uifont->colortable[0] = pens[best_black];
    Machine->uifont->colortable[1] = pens[best_white];
    Machine->uifont->colortable[2] = pens[best_white];
    Machine->uifont->colortable[3] = pens[best_black];

    if(DirectArray)
    {
      for(i = 0; i < total_colors; i++)
      {
        DirectArray->Palette[pens[i]][1]  = palette[3*i];
        DirectArray->Palette[pens[i]][2]  = palette[3*i+1];
        DirectArray->Palette[pens[i]][3]  = palette[3*i+2];

        Palette[pens[i]][0] = palette[3*i];
        Palette[pens[i]][1] = palette[3*i+1];
        Palette[pens[i]][2] = palette[3*i+2];
      }
    }
    else
    {
      for(i = 0; i < total_colors; i++)
      {
        PixelArray[0]->Palette[pens[i]][1]  = palette[3*i];
        PixelArray[0]->Palette[pens[i]][2]  = palette[3*i+1];
        PixelArray[0]->Palette[pens[i]][3]  = palette[3*i+2];

        Palette[pens[i]][0] = palette[3*i];
        Palette[pens[i]][1] = palette[3*i+1];
        Palette[pens[i]][2] = palette[3*i+2];
      }
    }
  }

  TRACE_LEAVE("osd_allocate_colors");
  return 0;
}
//krbtest
//struct osd_bitmap *_globalBitmap = NULL;
/* 0.37:
  Create a display screen, or window, of the given dimensions (or larger). It is
  acceptable to create a smaller display if necessary, in that case the user must
  have a way to move the visibility window around.
  Attributes are the ones defined in driver.h, they can be used to perform
  optimizations, e.g. dirty rectangle handling if the game supports it, or faster
  blitting routines with fixed palette if the game doesn't change the palette at
  run time. The VIDEO_PIXEL_ASPECT_RATIO flags should be honored to produce a
  display of correct proportions.
  Orientation is the screen orientation (as defined in driver.h) which will be done
  by the core. This can be used to select thinner screen modes for vertical games
  (ORIENTATION_SWAP_XY set), or even to ask the user to rotate the monitor if it's
  a pivot model. Note that the OS dependant code must NOT perform any rotation,
  this is done entirely in the core.
  Returns 0 on success.
*/

int osd_create_display(int width,int height,int depth,int fps,int attributes,int orientation)
//old struct osd_bitmap *osd_create_display(int width, int height, int attributes)
{
  unsigned char *line;
  
  LONG left, top, right, bottom;
  LONG i, t;

  TRACE_ENTER("osd_create_display");

  if(Machine->orientation & ORIENTATION_SWAP_XY)
  {
    t      = width;
    width  = height;
    height = t;
    
    left   = Machine->drv->default_visible_area.min_y;
    top    = Machine->drv->default_visible_area.min_x;
    right  = Machine->drv->default_visible_area.max_y;
    bottom = Machine->drv->default_visible_area.max_x;
  }
  else
  {
    left   = Machine->drv->default_visible_area.min_x;
    top    = Machine->drv->default_visible_area.min_y;
    right  = Machine->drv->default_visible_area.max_x;
    bottom = Machine->drv->default_visible_area.max_y;
  }

  if(attributes & VIDEO_TYPE_VECTOR)
  {
    if(Config[CFG_WIDTH] > width)
      width = Config[CFG_WIDTH];

    if(Config[CFG_HEIGHT] > height)
      height = Config[CFG_HEIGHT];
      
    left   = 0;
    top    = 0;
    right  = width - 1;
    bottom = height - 1;
  }
  else
  {
    if(Machine->orientation & ORIENTATION_FLIP_X)
    {
      t     = width - left - 1;
      left  = width - right - 1;
      right = t;
    }
    
    if(Machine->orientation & ORIENTATION_FLIP_Y)
    {
      t      = height - top - 1;
      top    = height - bottom - 1;
      bottom = t;
    }
  }

  Machine->uiwidth  = right - left + 1;
  Machine->uiheight = bottom - top + 1;
  Machine->uixmin   = left;
  Machine->uiymin   = top;

  if(VideoOpen(width, height, left, top, right, bottom, attributes & VIDEO_SUPPORTS_DIRTY))
  {
    DirtyLines[0] = NULL;

    if(DirectArray)
    {
      BitMap = osd_new_bitmap(width, height, 8);
      
      if((Config[CFG_DIRECTMODE] == CFGDM_COPY) && Config[CFG_DIRTYLINES] && !Config[CFG_BUFFERING])
      {
        DirtyLines[0] = (uint8_t *)calloc(2 * (1 + Config[CFG_BUFFERING]) * height, 1);
      
        if(DirtyLines[0])
        {
          for(i = 0; i < (2 * (Config[CFG_BUFFERING] + 1)); i++)
            DirtyLines[i+1] = DirtyLines[i] + height;
        }
      }
    }
    else
    {
      if(PixelArray[0]->DirtyLines)
      {
        DirtyLines[0] = (uint8_t *)calloc(2 * (1 + Config[CFG_BUFFERING]) * height, 1);
      
        if(DirtyLines[0])
        {
          for(i = 0; i < (2 * (Config[CFG_BUFFERING] + 1)); i++)
            DirtyLines[i+1] = DirtyLines[i] + height;
        }
      }
#ifdef POWERUP
      BitMap  = (struct osd_bitmap *) malloc( sizeof(struct osd_bitmap) + 2*height*sizeof(unsigned char *));
#else
      BitMap  = (struct osd_bitmap *) malloc( sizeof(struct osd_bitmap) + height*sizeof(unsigned char *));
#endif
    }
    
    if(BitMap)
    {
      ClearBitMap = 0;

      if(DirectArray)
      {     
        if(Config[CFG_DIRECTMODE] == CFGDM_DRAW)
        {
          line = DirectArray->Pixels + 8 + (8 * DirectArray->BytesPerRow);

          for(i = 0; i < height; i++)
          {
            BitMap->line[i] = line;

            line += DirectArray->BytesPerRow;
          }
        }
      }
      else
      {
        BitMap->width  = width;
        BitMap->height = height;
        BitMap->line   = (unsigned char **) &BitMap[1];

        if(PixelArray[0]->PixelFormat)
          BitMap->depth = 16;
        else
          BitMap->depth = 8;
  

        line = (unsigned char *) VGetPixelArrayPointAddr(PixelArray[0], 8, 8);

        for(i = 0; i < height; i++)
        {
          BitMap->line[i] = line;

          line += PixelArray[0]->BytesPerRow;
        }
#ifdef POWERUP
        line  = (unsigned char *) VGetPixelArrayPointAddr(PixelArray[1], 8, 8);

        for(; i < (height << 1); i++)
        {
          BitMap->line[i] = line;

          line += PixelArray[1]->BytesPerRow;
        }

        for(i = 0; i < 256; i++)
        {
          PixelArray[1]->Palette[i][0]  = 0;
          PixelArray[1]->Palette[i][1]  = 0;
          PixelArray[1]->Palette[i][2]  = 0;
          PixelArray[1]->Palette[i][3]  = 0;
        }
#endif
        for(i = 0; i < 256; i++)
        {
          PixelArray[0]->Palette[i][0]  = 1;
          PixelArray[0]->Palette[i][1]  = 0;
          PixelArray[0]->Palette[i][2]  = 0;
          PixelArray[0]->Palette[i][3]  = 0;
        }
      }

      TRACE_LEAVE("osd_create_display");
      //return(BitMap);
      //krbnote: it's in global var BitMap. -> TODO rename.
      return 0; // Returns 0 on success.
    }

    VideoClose();
  }

  TRACE_LEAVE("osd_create_display");

  return(1); // Returns 0 on success.
}

void osd_close_display(void)
{
  LONG i;

  TRACE_ENTER("osd_close_display");
  
  if(DirtyLines[0])
  {
    for(i = 1; i < (2 * (Config[CFG_BUFFERING] + 1)); i++)
    {
      if(DirtyLines[0] > DirtyLines[i])
        DirtyLines[0] = DirtyLines[i];
    }
  
    free(DirtyLines[0]);
  }

  if(BitMap)
    free(BitMap);

  TRACE_LEAVE("osd_close_display");

  VideoClose();
}
void osd_update_video_and_audio(struct osd_bitmap *bitmap)
//old void osd_update_video_and_audio(void)
{
  unsigned char *line;
  int       trueorientation;
  int       fps;
  LONG      i, j, w, h, l;
  uint32_t     bpr;
  uint32_t     mod;
  uint32_t     srcmod;
  uint32_t     *src;
  uint32_t     *pix;
  uint8_t     **lines;
  uint8_t     *dst;
  uint8_t     *newb, *new2, *new3;
  uint8_t     *old, *old2, *old3;
  char      buf[30];

  static int showfpstemp;
  int need_to_clear_bitmap = 0;

  TRACE_ENTER("osd_update_video_and_audio");

  /* Do not skip frames on the initial information screens. */

  if(FrameCounter >= NoFrameSkipCount)
  {
    if(FrameCounter < (NoFrameSkipCount + frameskip))
    {
      FrameCounter++;
      
      return;
    }

    FrameCounter = NoFrameSkipCount;
  }
  else
    FrameCounter++;

  if(ChannelArray[0])
  {
#ifdef POWERUP
    if(!ChannelArray[1])
      return;
#endif
    ASetChannelFrame(ChannelArray[CurrentArray]);
  }
/*re
  if(osd_key_pressed_memory(OSD_KEY_THROTTLE))
    throttle ^= 1;

  if(osd_key_pressed_memory(OSD_KEY_FRAMESKIP_INC))
  {
    frameskip   = (frameskip + 1) % 4;
    showfpstemp = 50;
  }

  if(osd_key_pressed_memory(OSD_KEY_SHOW_FPS))
  {
    ShowFPS ^= 1;

    if(!ShowFPS)
      need_to_clear_bitmap = 1;
  }
*/
  if(showfpstemp)
  {
    showfpstemp--;

    if(!ShowFPS && !showfpstemp)
      need_to_clear_bitmap = 1;
  }

  if(ShowFPS || showfpstemp)
  {
    trueorientation = Machine->orientation;
    Machine->orientation = ORIENTATION_DEFAULT;

    fps = VGetFPS(Video);
    sprintf(buf," %3d%%(%3d/%d fps)",(int)(100*fps/Machine->drv->frames_per_second),fps,
            (int)(Machine->drv->frames_per_second));
    l = strlen(buf);

//krb, verify
#ifndef DT_COLOR_WHITE
#define DT_COLOR_WHITE 0
#endif
    for (i = 0;i < l;i++)
    {
      drawgfx(Machine->scrbitmap,Machine->uifont,buf[i], DT_COLOR_WHITE, 0, 0,
              Machine->uixmin + Machine->uiwidth - (l-i)*Machine->uifont->width,
              Machine->uiymin, 0, TRANSPARENCY_NONE, 0);
    }

    Machine->orientation = trueorientation;
  }

  if(on_screen_display_timer > 0)
  {
    on_screen_display_timer -= (frameskip+1);

    if(on_screen_display_timer <= 0)
    {
      on_screen_display_timer = 0;
      need_to_clear_bitmap    = 1;
    }
  }

  VSetFrameSkip(Video, frameskip);
  VSetLimitSpeed(Video, throttle);

  if(DirectArray)
  {
    VSetDirectFrame(DirectArray);

    if(Config[CFG_DIRECTMODE] == CFGDM_DRAW)
    {
      if(DirectArray->Pixels)
      {
        h   = /*BitMap*/bitmap->height;
        bpr   = DirectArray->BytesPerRow;
        line  = DirectArray->Pixels + 8 + (8 * bpr);
        lines = /*BitMap*/bitmap->line;
  
        for(i = 0; i < h; i++)
        {
          lines[i]    = line;
          line      += bpr;
        }
      }
      else
        puts("PANIC! Didn't get direct pointer");
    }
    else
    {
      if(DirectArray->Pixels)
      {
        if(DirtyLines[0])
        {
          lines = bitmap->line;
          h   = bitmap->height;
          w   = bitmap->width >> 2;
          pix   = (uint32_t *) DirectArray->Pixels;
          bpr   = DirectArray->BytesPerRow >> 2;
          mod   = bpr - w;
          newb   = DirtyLines[0];
          old   = DirtyLines[1];

          switch(Config[CFG_BUFFERING])
          {
            case CFGB_SINGLE:           
              DirtyLines[0] = old;
              DirtyLines[1] = newb;

              for(i = 0; i < h; i++)
              {
                if((*newb++ | *old++))
                {
                  src = (uint32_t *) lines[i];
              
                  for(j = 0; j < w; j++)
                    *pix++ = *src++;
          
                  pix += mod;
                }
                else
                  pix += bpr;
              }
              
              break;

            case CFGB_DOUBLE:             
              new2 = DirtyLines[2];
              old2 = DirtyLines[3];
              
              DirtyLines[0] = old2;
              DirtyLines[1] = new2;
              DirtyLines[2] = newb;
              DirtyLines[3] = old;
              
              for(i = 0; i < h; i++)
              {
                if((*newb++ | *new2++ | *old++ | *old2++))
                {
                  src = (uint32_t *) lines[i];
              
                  for(j = 0; j < w; j++)
                    *pix++ = *src++;
          
                  pix += mod;
                }
                else
                  pix += bpr;
              }
              
              break;

            case CFGB_TRIPLE:
              new2 = DirtyLines[2];
              old2 = DirtyLines[3];
              new3 = DirtyLines[4];
              old3 = DirtyLines[5];
              
              DirtyLines[0] = old3;
              DirtyLines[1] = new3;
              DirtyLines[2] = newb;
              DirtyLines[3] = old;
              DirtyLines[4] = new2;
              DirtyLines[5] = old2;
              
              for(i = 0; i < h; i++)
              {
                if((*newb++ | *new2++ | *new3++ | *old++ | *old2++ | *old3++))
                {
                  src = (uint32_t *) lines[i];
              
                  for(j = 0; j < w; j++)
                    *pix++ = *src++;
          
                  pix += mod;
                }
                else
                  pix += bpr;
              }
              
              break;
          }

          memset(DirtyLines[0], 0, h);
        }
        else
        {
          lines = bitmap->line;
          h   = bitmap->height;
          w   = bitmap->width >> 2;
          pix   = (uint32_t *) DirectArray->Pixels;
          bpr   = DirectArray->BytesPerRow >> 2;
          mod   = bpr - w;
          src   = (uint32_t *) lines[0];
          srcmod  = (((uint32_t) bitmap->_private) >> 2) - w;

          for(i = 0; i < h; i++)
          {
            for(j = 0; j < w; j++)
                *pix++ = *src++;
              
            pix += mod;
            src += srcmod;
          }
        }
      }
    }
  }
  else
  {
    PixelArray[CurrentArray]->BackgroundPen = Machine->pens[0];

    if(DirtyLines[0])
    {
      h = bitmap->height;
      dst = PixelArray[CurrentArray]->DirtyLines + 8;
      newb = DirtyLines[0];
      old = DirtyLines[1];

      switch(Config[CFG_BUFFERING])
      {
        case CFGB_SINGLE:           
          DirtyLines[0] = old;
          DirtyLines[1] = newb;

          for(i = 0; i < h; i++)
            *dst++ = *newb++ | *old++;
          
          break;

        case CFGB_DOUBLE:             
          new2 = DirtyLines[2];
          old2 = DirtyLines[3];
          
          DirtyLines[0] = old2;
          DirtyLines[1] = new2;
          DirtyLines[2] = newb;
          DirtyLines[3] = old;
          
          for(i = 0; i < h; i++)
            *dst++ = *newb++ | *new2++ | *old++ | *old2++;
          
          break;

        case CFGB_TRIPLE:
          new2 = DirtyLines[2];
          old2 = DirtyLines[3];
          new3 = DirtyLines[4];
          old3 = DirtyLines[5];
          
          DirtyLines[0] = old3;
          DirtyLines[1] = new3;
          DirtyLines[2] = newb;
          DirtyLines[3] = old;
          DirtyLines[4] = new2;
          DirtyLines[5] = old2;
          
          for(i = 0; i < h; i++)
            *dst++ = *newb++ | *new2++ | *new3++ | *old++ | *old2++ | *old3++;
          
          break;
      }

      VSetPixelFrame(PixelArray[CurrentArray]);

      memset(DirtyLines[0], 0, h);
    }
    else
      VSetPixelFrame(PixelArray[CurrentArray]);
  }

  frameskip = VGetFrameSkip(Video);

  if(need_to_clear_bitmap)
    osd_clearbitmap(bitmap);

  input_update_counter = 0;
  InputUpdate(FALSE);

  TRACE_LEAVE("osd_update_video_and_audio");
}

void osd_modify_pen(int pen, unsigned char red, unsigned char green, unsigned char blue)
{
  TRACE_ENTER("osd_modify_pen");

  if(DirectArray)
  {
    DirectArray->Palette[pen][0]  = 1;
    DirectArray->Palette[pen][1]  = red;
    DirectArray->Palette[pen][2]  = green;
    DirectArray->Palette[pen][3]  = blue;
  }
  else
  {
    PixelArray[CurrentArray]->Palette[pen][0] = 1;
    PixelArray[CurrentArray]->Palette[pen][1] = red;
    PixelArray[CurrentArray]->Palette[pen][2] = green;
    PixelArray[CurrentArray]->Palette[pen][3] = blue;
  }

  Palette[pen][0] = red;
  Palette[pen][1] = green;
  Palette[pen][2] = blue;

  TRACE_LEAVE("osd_modify_pen");
}

void osd_get_pen(int pen, unsigned char *r, unsigned char *g, unsigned char *b)
{
  TRACE_ENTER("osd_get_pen");

  *r  = Palette[pen][0];
  *g  = Palette[pen][1];
  *b  = Palette[pen][2];

  TRACE_LEAVE("osd_get_pen");
}

int osd_faccess (const char *filename, int filetype)
{
  return(1);
}

int osd_fchecksum (const char *game, const char *filename, unsigned int *length, unsigned int *sum)
{
  return(0);
}

void *osd_fopen(const char *gamename,const char *filename,int filetype,int write)
{
  struct File *file;
  std::string *zip_name=NULL;

  file = NULL;

  if(!write)
  {
    if(filetype == OSD_FILETYPE_ROM)
      zip_name = &ROMZipName;
    else if(filetype == OSD_FILETYPE_SAMPLE)
      zip_name = &SampleZipName;

    if(zip_name && !zip_name->empty() )
    {
      file = (struct File *)calloc(sizeof(struct File), 1);
          
      if(file)
      {
        if(!load_zipped_file(zip_name->c_str(), filename, &file->Data, &file->Length))
        {
          file->Type  = FILETYPE_CUSTOM;
          file->CRC = crc32(0, file->Data, file->Length);
        }
        else
        {
          free(file);
          file = NULL;
        }
      }
    }
  }

  if(!file)
  {
    file = OpenFileType(gamename, filename, write ? MODE_NEWFILE : MODE_OLDFILE, filetype);
    
    if(file && (file->Type == FILETYPE_ZIP))
    {
      if(filetype == OSD_FILETYPE_ROM)
        zip_name = &ROMZipName;
      else if(filetype == OSD_FILETYPE_SAMPLE)
        zip_name = &SampleZipName;
      else
        zip_name= NULL;

      /* Cache the zip filename. */
      
      if(zip_name) (*zip_name) = file->Name;

      if(load_zipped_file(zip_name->c_str(), filename, &file->Data, &file->Length))
      {
        file->Data = NULL;
        osd_fclose(file);

        return(NULL);
      }

      file->CRC = crc32(0, file->Data, file->Length);
    }
  }

  return((void *) file);
}

int osd_fread(void *file_handle, void *buffer, int length)
{
  struct File *file;
  LONG len;

  file = (struct File *) file_handle;

  switch(file->Type)
  {
    case FILETYPE_ZIP:
    case FILETYPE_CUSTOM:
      if(file->Data)
      {
        len = file->Length - file->Offset;
        
        if(len > length)
          len = length;
    
        memcpy(buffer, &file->Data[file->Offset], len);
    
        file->Offset += len;
        
        return(len);
      }
       
      break;

    case FILETYPE_NORMAL:
    case FILETYPE_TMP:
      len = ReadFile(file->File, buffer, length);

      return(len);
  }

  return(0);
}

int osd_fread_scatter(void *void_file_p, void *buffer_p, int length, int increment)
{
  struct File *file_p;
  uint8_t buf[4096];
  uint8_t *dst_p;
  uint8_t *src_p;
  int   remaining_len;
  int   len;

  file_p = (struct File *) void_file_p;
  dst_p  = (uint8_t*)buffer_p;

  switch(file_p->Type)
  {
    case FILETYPE_ZIP:
    case FILETYPE_CUSTOM:
      if(file_p->Data)
      {
        len = file_p->Length - file_p->Offset;

        if(len > length)
          len = length;

        length = len;
            
        src_p = &file_p->Data[file_p->Offset];

        while(len--)
        {
          *dst_p = *src_p++;
          
          dst_p += increment;
        }
    
        file_p->Offset += length;

        return(length);
      }
       
      break;

    case FILETYPE_NORMAL:
    case FILETYPE_TMP:
      remaining_len = length;
    
      while(remaining_len)
      {      
        if(remaining_len < sizeof(buf))
          len = remaining_len;
        else
          len = sizeof(buf);

        len = ReadFile(file_p->File, buf, len);

        if(len == 0)
          break;

        remaining_len -= len;

        src_p = buf;
        
        while(len--)
        {
          *dst_p = *src_p++;
          
          dst_p += increment;
        }
      }
      
      length = length - remaining_len;
      
      return(length);
      
      break;
  }

  return(0);
}

int osd_fread_swap(void *file_handle, void *buffer, int length)
{
	int i;
	uint8_t *buf;
	uint8_t temp;
	int res;

	res = osd_fread(file_handle,buffer,length);

	buf = (uint8_t*)buffer;
	for (i = 0;i < length;i+=2)
	{
		temp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = temp;
	}

	return res;
}

int osd_fwrite(void *void_file_p, const void *buffer_p, int length)
{
  struct File *file_p;
  LONG rc;

  file_p = (struct File *) void_file_p;

  switch(file_p->Type)
  {
    case FILETYPE_ZIP:
    case FILETYPE_CUSTOM:
      return(-1);

    case FILETYPE_NORMAL:
    case FILETYPE_TMP:
      rc = WriteFile(file_p->File, (void *) buffer_p, length);

      if(rc > 0)
        return(rc);
  }

  return(0);
}

int osd_fwrite_swap(void *file,const void *buffer,int length)
{
	int i;
	unsigned char *buf;
	unsigned char temp;
	int res;


	buf = (unsigned char *)buffer;
	for (i = 0;i < length;i+=2)
	{
		temp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = temp;
	}

	res = osd_fwrite(file,buffer,length);

	for (i = 0;i < length;i+=2)
	{
		temp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = temp;
	}

	return res;
}

int osd_fseek(void *file, int position, int mode)
{
  LONG  rc = 0;

  if((((struct File *) file)->Type == FILETYPE_ZIP) || (((struct File *) file)->Type == FILETYPE_CUSTOM))
  {
    switch(mode)
    {
      case SEEK_SET:
        ((struct File *) file)->Offset = position;
        break;
      case SEEK_CUR:
        ((struct File *) file)->Offset += position;
        break;
      case SEEK_END:
        ((struct File *) file)->Offset = ((struct File *) file)->Length + position;
        break;
      default:
        return(-1);
    }
  }
  else
  {
    switch(mode)
    {
      case SEEK_SET:
        rc = SeekFile(((struct File *) file)->File, position, OFFSET_BEGINNING);
        break;
      case SEEK_CUR:
        rc = SeekFile(((struct File *) file)->File, position, OFFSET_CURRENT);
        break;
      case SEEK_END:
        rc = SeekFile(((struct File *) file)->File, position, OFFSET_END);
        break;
      default:
        return(-1);
    }

#ifdef POWERUP
    return(rc);
#else
    if(DOSBase->dl_lib.lib_Version > 37)
    {
      if(rc == -1)
        return(-1);
      else
        return(0);
    }
    else
    {
      if(IoErr())
        return(-1);
      else
        return(0);
    }
#endif
  }

  return(rc);
}

void osd_fclose(void *file)
{
  if(((((struct File *) file)->Type == FILETYPE_ZIP) || (((struct File *) file)->Type == FILETYPE_CUSTOM)) && ((struct File *) file)->Data)
    free(((struct File *) file)->Data);
  
  if(((struct File *) file)->Type == FILETYPE_CUSTOM)
    free(file);
  else
    CloseFile((struct File *) file);
}

void osd_led_w(int led,int on)
{
}
/*
static int map_key(int key)
{
  switch(key)
  {
    case OSD_KEY_CANCEL:
      return(OSD_KEY_ESC);
    case OSD_KEY_RESET_MACHINE:
      return(OSD_KEY_F3);

    case OSD_KEY_SHOW_GFX:
      return(OSD_KEY_F4);

    case OSD_KEY_CHEAT_TOGGLE:
      return(OSD_KEY_F5);

    case OSD_KEY_SHOW_FPS:
      return(OSD_KEY_F7);

    case OSD_KEY_FRAMESKIP_INC:
      return(OSD_KEY_F9);

    case OSD_KEY_THROTTLE:
      return(OSD_KEY_F10);

    case OSD_KEY_CONFIGURE:
      return(OSD_KEY_TAB);

    case OSD_KEY_ON_SCREEN_DISPLAY:
      return(OSD_KEY_TILDE);

    case OSD_KEY_PAUSE:
    case OSD_KEY_UNPAUSE:
      return(OSD_KEY_P);

    case OSD_KEY_UI_SELECT:
      return(OSD_KEY_ENTER);

    case OSD_KEY_UI_LEFT:
      return(OSD_KEY_LEFT);

    case OSD_KEY_UI_RIGHT:
      return(OSD_KEY_RIGHT);

    case OSD_KEY_UI_UP:
      return(OSD_KEY_UP);

    case OSD_KEY_UI_DOWN:
      return(OSD_KEY_DOWN);

    default:
      return(key);
  }
}

int osd_key_invalid(int keycode)
{
    switch(keycode)
    {
        case KEYCODE_ESC:
        case KEYCODE_F3:
        case KEYCODE_F4:
        case KEYCODE_F7:
        case KEYCODE_F8:
        case KEYCODE_F10:
        case KEYCODE_TAB:
        case KEYCODE_TILDE:
        case KEYCODE_P:
      return(1);

    default:
      return(0);
  }
}

const char *osd_key_name(int keycode)
{
/*re
	static const char *keynames[] =
	{
		"ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "MINUS", "EQUAL", "BKSPACE",
		"TAB", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "OPBRACE", "CLBRACE", "ENTER",
		"LCTRL", "A", "S", "D", "F", "G", "H", "J", "K", "L", "COLON", "QUOTE", "TILDE",
		"LSHIFT", "Error", "Z", "X", "C", "V", "B", "N", "M", "COMMA", ".", "SLASH", "RSHIFT",
		"*", "ALT", "SPACE", "CAPSLOCK", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
		"NUMLOCK", "SCRLOCK", "HOME", "UP", "PGUP", "MINUS PAD",
		"LEFT", "5 PAD", "RIGHT", "PLUS PAD", "END", "DOWN",
		"PGDN", "INS", "DEL", "RCTRL", "ALTGR", "Error",
		"F11", "F12", "Error", "Error",
		"Error", "Error", "Error", "Error", "Error",
		"Error", "Error", "Error", "Error", "Error",
		"1 PAD", "2 PAD", "3 PAD", "4 PAD", "Error",
		"6 PAD", "7 PAD", "8 PAD", "9 PAD", "0 PAD",
		". PAD", "= PAD", "/ PAD", "* PAD", "ENTER PAD",
		"Error", "Error", "Error", "Error", "Error",
    "Error", "Error", "PAUSE",
  };

	if(keycode && keycode <= OSD_MAX_KEY)
    return(keynames[keycode-1]);
	else

    return("None");
}

int osd_key_pressed(int key)
{
  if(!Keys)
    return(0);
/*todo
#ifndef POWERUP
  // To prevent the m68k version from being stuck in a waiting for
  // key loop.

  input_update_counter++;
  
  if(input_update_counter > 1000)
  {
    input_update_counter = 0;
    InputUpdate(FALSE);
  }
#endif

  if(key == OSD_KEY_ANY)
    return(osd_read_key_immediate() != OSD_KEY_NONE);

  key = map_key(key);

  if((key < OSD_MAX_KEY) && Keys[key])
    return(1);

  return(0);
}

int osd_key_pressed_memory(int key)
{

#ifndef POWERUP
  // To prevent the m68k version from being stuck in a waiting for
  // key loop.

  input_update_counter++;
  
  if(input_update_counter > 1000)
  {
    input_update_counter = 0;
    InputUpdate(FALSE);
  }
#endif

  if(key == OSD_KEY_ANY)
    return(osd_read_key_immediate() != OSD_KEY_NONE);

  key = map_key(key);

  if(key < OSD_MAX_KEY)
  {
    if(Keys[key] == 1)
    {
      Keys[key] = 2;

      return(1);
    }
  }
  return(0);
}

int osd_key_pressed_memory_repeat(int key, int speed)
{
  static int counter;
  static int keydelay;

#ifndef POWERUP
  // To prevent the m68k version from being stuck in a waiting for
  // key loop.

  input_update_counter++;
  
  if(input_update_counter > 1000)
  {
    input_update_counter = 0;
    InputUpdate(FALSE);
  }
#endif

  if(key == OSD_KEY_ANY)
    return(osd_read_key_immediate() != OSD_KEY_NONE);

  key = map_key(key);

  if(key < OSD_MAX_KEY)
  {
    if(Keys[key] == 1)
    {
      Keys[key] = 2;

            keydelay = 3;
      counter  = 0;

      return(1);
    }

    if((Keys[key] == 2)
    && (++counter > keydelay * speed * Machine->drv->frames_per_second / 60))
    {
            keydelay = 1;
      counter  = 0;

      return(1);
    }   
  }
  return(0);

}

int osd_read_key_immediate(void)
{
  int key;

#ifndef POWERUP
  / To prevent the m68k version from being stuck in a waiting for
  // key loop.

  input_update_counter++;
  
  if(input_update_counter > 1000)
  {
    input_update_counter = 0;
    InputUpdate(FALSE);
  }
#endif

  for(key = OSD_MAX_KEY; (key > OSD_KEY_NONE) && !Keys[key]; key--);

  if((key > OSD_KEY_NONE) && (Keys[key] == 1))
  {
    Keys[key] = 2;

    return(key);
  }

  return(OSD_KEY_NONE);
}

int osd_read_keyrepeat(void)
{
  int key;

  while((key = osd_read_key_immediate()) == OSD_KEY_NONE)
    InputUpdate(TRUE);

  return(key);
}
*/
/*old
const char *osd_joy_name(int joycode)
{

	static const char *joynames[] = {
		"Left", "Right", "Up", "Down", "Button A",
		"Button B", "Button C", "Button D", "Button E", "Button F",
		"Button G", "Button H", "Button I", "Button J", "Any Button",
		"J2 Left", "J2 Right", "J2 Up", "J2 Down", "J2 Button A",
		"J2 Button B", "J2 Button C", "J2 Button D", "J2 Button E", "J2 Button F",
		"J2 Button G", "J2 Button H", "J2 Button I", "J2 Button J", "J2 Any Button",
		"J3 Left", "J3 Right", "J3 Up", "J3 Down", "J3 Button A",
		"J3 Button B", "J3 Button C", "J3 Button D", "J3 Button E", "J3 Button F",
		"J3 Button G", "J3 Button H", "J3 Button I", "J3 Button J", "J3 Any Button",
		"J4 Left", "J4 Right", "J4 Up", "J4 Down", "J4 Button A",
		"J4 Button B", "J4 Button C", "J4 Button D", "J4 Button E", "J4 Button F",
		"J4 Button G", "J4 Button H", "J4 Button I", "J4 Button J", "J4 Any Button"
	};

	if(joycode == 0)
    return("None");
	else if(joycode <= OSD_MAX_JOY)
    return(joynames[joycode-1]);
	else
    return("Unknown");
}

void osd_poll_joysticks(void)
{
}

int osd_joy_pressed(int joycode)
{
  if(Keys)
  {
    if(joycode < OSD_JOY2_LEFT)
    {
      switch(joycode)
      {
        case OSD_JOY_FIRE1:
          if(Port2->Type == IPT_MOUSE)
            return(Port2->Red);

          return(Port1->Red);

        case OSD_JOY_FIRE2:
          if(Port2->Type == IPT_MOUSE)
            return(Port2->Blue);

          return(Port1->Blue);

        case OSD_JOY_FIRE3:
          if(Port2->Type == IPT_MOUSE)
            return(Port2->Green);

          return(Port1->Green);

        case OSD_JOY_FIRE4:
          return(Port1->Yellow);

        case OSD_JOY_FIRE5:
          return(Port1->Forward);

        case OSD_JOY_FIRE6:
          return(Port1->Reverse);

        case OSD_JOY_FIRE7:
          return(Port1->Play);

        case OSD_JOY_FIRE:
          if(Port2->Type == IPT_MOUSE)
            return(Port2->Red | Port2->Blue | Port2->Green |
                   Port1->Red | Port1->Blue | Port1->Green | Port1->Yellow |
                   Port1->Forward | Port1->Reverse | Port1->Play);

          return(Port1->Red | Port1->Blue | Port1->Green | Port1->Yellow |
                 Port1->Forward | Port1->Reverse | Port1->Play);
        default:
          if((Port1->Type == IPT_JOYSTICK) || (Port1->Type == IPT_JOYPAD))
          {
            switch(joycode)
            {
              case OSD_JOY_LEFT:
                return(Port1->Move.Joystick.Left);

              case OSD_JOY_RIGHT:
                return(Port1->Move.Joystick.Right);

              case OSD_JOY_UP:
                return(Port1->Move.Joystick.Up);

              case OSD_JOY_DOWN:
                return(Port1->Move.Joystick.Down);
            }
          }
      }
    }
    else
    {
      switch(joycode)
      {
        case OSD_JOY2_FIRE1:
          return(Port2->Red);

        case OSD_JOY2_FIRE2:
          return(Port2->Blue);

        case OSD_JOY2_FIRE3:
          return(Port2->Green);

        case OSD_JOY2_FIRE4:
          return(Port2->Yellow);

        case OSD_JOY2_FIRE5:
          return(Port2->Forward);

        case OSD_JOY2_FIRE6:
          return(Port2->Reverse);

        case OSD_JOY2_FIRE7:
          return(Port2->Play);

        case OSD_JOY2_FIRE:
          return( Port2->Red | Port2->Blue | Port2->Green | Port2->Yellow
              | Port2->Forward | Port2->Reverse | Port2->Play);

        default:
          if((Port2->Type == IPT_JOYSTICK) || (Port2->Type == IPT_JOYPAD))
          {
            switch(joycode)
            {
              case OSD_JOY2_LEFT:
                return(Port2->Move.Joystick.Left);

              case OSD_JOY2_RIGHT:
                return(Port2->Move.Joystick.Right);

              case OSD_JOY2_UP:
                return(Port2->Move.Joystick.Up);

              case OSD_JOY2_DOWN:
                return(Port2->Move.Joystick.Down);
            }
          }
      }
    }
  }

  return(0);
}
*/

void osd_play_sample(int channel,signed char *data,int len,int freq,int volume,int loop)
{
  if(ChannelArray[0] && (channel < AUDIO_CHANNELS))
  {
#ifdef POWERUP
    if(!ChannelArray[1])
      return;
#endif
    if(len > 0)
    {
#ifdef POWERUP
      if(len > AUDIO_BUFFER_LENGTH)
      {
        len = AUDIO_BUFFER_LENGTH;
        printf("audio clipping\n");
      }

      memcpy(ChannelArray[CurrentArray]->Channels[channel].Buffer, data, len);

      data = ChannelArray[CurrentArray]->Channels[channel].Buffer;
#endif
      APlaySample(ChannelArray[CurrentArray], channel, freq, volume * 100 / 255, len, (UBYTE*)data, loop);
    }
    else if(len < 0)
    {
      APlaySound(ChannelArray[CurrentArray], channel, freq, volume * 100 / 255, -len, loop);
    }
  }
}

void osd_play_sample_16(int channel,signed short *data,int len,int freq,int volume,int loop)
{
}

void osd_play_streamed_sample(int channel,signed char *data,int len,int freq,int volume, int pan)
{
  osd_play_sample(channel, data, len, freq, volume, TRUE);
}

void osd_play_streamed_sample_16(int channel,signed short *data,int len,int freq,int volume, int pan)
{
}

void osd_adjust_sample(int channel,int freq,int volume)
{
  if(ChannelArray[0] && (channel < AUDIO_CHANNELS))
  {
#ifdef POWERUP
    if(!ChannelArray[1])
      return;
#endif
    ASetFrequency(ChannelArray[CurrentArray], channel, freq);
    ASetVolume(ChannelArray[CurrentArray], channel, volume * 100 / 255);
  }
}

void osd_stop_sample(int channel)
{
  if(ChannelArray[0])
  {
#ifdef POWERUP
    if(!ChannelArray[1])
      return;
#endif
    AStopChannel(ChannelArray[CurrentArray], channel);
  }
}

void osd_restart_sample(int channel)
{
  if(ChannelArray[0])
  {
#ifdef POWERUP
    if(!ChannelArray[1])
      return;
#endif
    ARestartChannel(ChannelArray[CurrentArray], channel);
  }
}

int osd_get_sample_status(int channel)
{
  return(-1);
}

void osd_ym2203_write(int n, int r, int v)
{
}

void osd_ym2203_update(void)
{
}

int osd_ym3812_status(void)
{
  return(0);
}

int osd_ym3812_read(void)
{
  return(0);
}

void osd_ym3812_control(int reg)
{
}

void osd_ym3812_write(int data)
{
}

void osd_set_mastervolume(int attenuation)
{
	float volume;

	Attenuation = attenuation;

 	volume = 256.0;	/* range is 0-256 */

	while(attenuation++ < 0)
		volume /= 1.122018454;	/* = (10 ^ (1/20)) = 1dB */

  MasterVolume = volume;

  if(ChannelArray[0])
  {
#ifdef POWERUP
    if(!ChannelArray[1])
      return;
#endif
    ASetMasterVolume(ChannelArray[CurrentArray], MasterVolume);
  }
}

int osd_get_mastervolume(void)
{
  return(Attenuation);
}

void osd_sound_enable(int enable)
{
#ifdef POWERUP
  if(ChannelArray[0] && ChannelArray[1])
#else
  if(ChannelArray[0])
#endif
  {
    if(enable)
    {
      ASetMasterVolume(ChannelArray[CurrentArray], MasterVolume);
    }
    else
    {
      ASetMasterVolume(ChannelArray[CurrentArray], 0);
    }
  }
}

struct GameSamples *readsamples(const char **samplenames,const char *basename)
{
  struct MameSample *mame_sample;
  struct GameSamples  *samples;
  struct ASound   *sound;
  int        skipfirst;
  int        i;

#ifdef POWERUP
    if(!ChannelArray[1])
      return(NULL);
#endif

  if(ChannelArray[0])
  {
    skipfirst = 0;

    if(samplenames && samplenames[0])
    {
      if(samplenames[0][0] == '*')
        skipfirst = 1;

      i = 0;

      while(samplenames[i+skipfirst])
        i++;

      if((samples = (struct GameSamples *)malloc(sizeof(struct GameSamples) + (i-1)*sizeof(struct GameSample))))
      {
        samples->total = i;

        for(i = 0; i < samples->total; i++)
          samples->sample[i] = 0;

        for(i = 0; i < samples->total; i++)
        {
          struct File *file;

          if(samplenames[i+skipfirst][0])
          {
            file = (struct File *)
                    osd_fopen(basename,samplenames[i+skipfirst],OSD_FILETYPE_SAMPLE,0);
            
            if(!file && skipfirst)
              file = (struct File *)
                      osd_fopen(samplenames[0]+1,samplenames[i+skipfirst],OSD_FILETYPE_SAMPLE,0);

            if(file)
            {
              if((file->Type == FILETYPE_ZIP) || (file->Type == FILETYPE_CUSTOM))
              {
                mame_sample = (struct MameSample *) file->Data;
                
                if((mame_sample->ID == MAKE_ID('M','A','M','E')) && mame_sample->Length)
                {
                  sound = ALoadSound(Audio, mame_sample->Data,
                                mame_sample->Resolution,
                                INTELuint32_t(mame_sample->Length),
                                INTELuint32_t(mame_sample->Frequency),
                                mame_sample->Volume);
                }
                else
                  sound = NULL;
              }
              else
                sound = AReadSound(Audio, file->File);

              if(sound)
              {
                samples->sample[i] = (struct GameSample *)malloc(sizeof(struct GameSample));
                
                if(samples->sample[i])
                {
                  samples->sample[i]->length    = -sound->Sound;
           //krb: no volume       samples->sample[i]->Volume    = sound->Volume;
                  samples->sample[i]->smpfreq   = sound->Frequency;
                  samples->sample[i]->resolution  = 8;
                }
/*
struct GameSample
{
	int length;
	int smpfreq;
	int resolution;
	signed char data[1];	// extendable
};
*/
              }

              osd_fclose(file);
            }
          }
        }

        return(samples);
      }
    }
  }
  
  return(NULL);
}

int osd_skip_this_frame(void)
{
  if(FrameCounter >= NoFrameSkipCount)
  {
    if(FrameCounter < (NoFrameSkipCount + frameskip))
      return(1);
  }

  return(0);
}

#ifdef MESS
#if 0
extern int CurrentVolume;

int osd_handle_event(void)
{
static  int showvoltemp = 0;

        /* if the user pressed ESC, stop the emulation */
        if (osd_key_pressed(UI_KEY_ESCAPE))
                return 1;

        if (osd_key_pressed(UI_KEY_DEC_VOLUME) && osd_key_pressed(OSD_KEY_LSHIFT) == 0)
        {
                /* decrease volume */
                if (CurrentVolume > 0) CurrentVolume--;
                osd_set_mastervolume(CurrentVolume);
                showvoltemp = 50;
        }

        if (osd_key_pressed(UI_KEY_INC_VOLUME) && osd_key_pressed(OSD_KEY_LSHIFT) == 0)
        {
                /* increase volume */
                if (CurrentVolume < 100) CurrentVolume++;
                osd_set_mastervolume(CurrentVolume);
                showvoltemp = 50;
        }                                          /* MAURY_END: new options */

        if (osd_key_pressed(UI_KEY_PAUSE)) /* pause the game */
        {
                struct DisplayText dt[2];
                int count = 0;


                dt[0].text = "PAUSED";
                dt[0].color = DT_COLOR_RED;
                dt[0].x = (Machine->uiwidth - Machine->uifont->width * strlen(dt[0].text)) / 2;
                dt[0].y = (Machine->uiheight - Machine->uifont->height) / 2;
                dt[1].text = 0;

                osd_set_mastervolume(0);

                while (osd_key_pressed(UI_KEY_PAUSE))
                        osd_update_audio();     /* give time to the sound hardware to apply the volume change */

                while (osd_key_pressed(UI_KEY_PAUSE) == 0 && osd_key_pressed(UI_KEY_ESCAPE) == 0)
                {
                        if (osd_key_pressed(UI_KEY_MENU)) setup_menu(); /* call the configuration menu */

                        osd_clearbitmap(Machine->scrbitmap);

                        (*Machine->drv->vh_update)(Machine->scrbitmap, 1);  /* redraw screen */

                        if (count < Machine->drv->frames_per_second / 2)
                                displaytext(dt,0);      /* make PAUSED blink */
                        else
                                osd_update_display();

                        count = (count + 1) % (Machine->drv->frames_per_second / 1);
                }

                while (osd_key_pressed(UI_KEY_ESCAPE));   /* wait for jey release */
                while (osd_key_pressed(UI_KEY_PAUSE));     /* ditto */

                osd_set_mastervolume(CurrentVolume);
                osd_clearbitmap(Machine->scrbitmap);
        }

        /* if the user pressed TAB, go to the setup menu */
        if (osd_key_pressed(UI_KEY_MENU))
        {
                osd_set_mastervolume(0);

                while (osd_key_pressed(UI_KEY_MENU))
                        osd_update_audio();     /* give time to the sound hardware to apply the volume change */

                if (setup_menu()) return 1;

                osd_set_mastervolume(CurrentVolume);
        }

        /* if the user pressed F4, show the character set */
        if (osd_key_pressed(UI_KEY_CHARSET))
        {
                osd_set_mastervolume(0);

                while (osd_key_pressed(UI_KEY_CHARSET))
                        osd_update_audio();     /* give time to the sound hardware to apply the volume change */

                if (showcharset()) return 1;

                osd_set_mastervolume(CurrentVolume);
        }

        if (showvoltemp)
        {
                showvoltemp--;
                if (!showvoltemp)
                {
                        osd_clearbitmap(Machine->scrbitmap);
                }
                else
                {                     /* volume-meter */
                int trueorientation;
                int i,x;
                char volstr[25];
                        trueorientation = Machine->orientation;
                        Machine->orientation = ORIENTATION_DEFAULT;

                        x = (Machine->uiwidth - 24*Machine->uifont->width)/2;
                        strcpy(volstr,"                      ");
                        for (i = 0;i < (CurrentVolume/5);i++) volstr[i+1] = '\x15';

                        drawgfx(Machine->scrbitmap,Machine->uifont,16,DT_COLOR_RED,0,0,x,Machine->drv->screen_height/2,0,TRANSPARENCY_NONE,0);
                        drawgfx(Machine->scrbitmap,Machine->uifont,17,DT_COLOR_RED,0,0,x+23*Machine->uifont->width,Machine->drv->screen_height/2,0,TRANSPARENCY_NONE,0);
                        for (i = 0;i < 22;i++)
                            drawgfx(Machine->scrbitmap,Machine->uifont,(unsigned int)volstr[i],DT_COLOR_WHITE,
                                        0,0,x+(i+1)*Machine->uifont->width+Machine->uixmin,Machine->uiheight/2+Machine->uiymin,0,TRANSPARENCY_NONE,0);

                        Machine->orientation = trueorientation;
                }
        }

        return 0;
}
#endif

int osd_fdc_init(void)
{
  return(1);
}

void osd_fdc_exit(void)
{
}

void osd_fdc_motors(unsigned char unit)
{
}

void osd_fdc_density(unsigned char unit, unsigned char density, unsigned char tracks, unsigned char spt, unsigned char eot, unsigned char secl)
{
}

void osd_fdc_interrupt(int param)
{
}

unsigned char osd_fdc_recal(unsigned char *track)
{
  return(0);
}

unsigned char osd_fdc_seek(unsigned char t, unsigned char *track)
{
  return(0);
}

unsigned char osd_fdc_step(int dir, unsigned char *track)
{
  return(0);
}

unsigned char osd_fdc_format(unsigned char t, unsigned char h, unsigned char spt, unsigned char *fmt)
{
  return(0);
}

unsigned char osd_fdc_put_sector(unsigned char track, unsigned char side, unsigned char head, unsigned char sector, unsigned char *buff, unsigned char ddam)
{
  return(0);
}

unsigned char osd_fdc_get_sector(unsigned char track, unsigned char side, unsigned char head, unsigned char sector, unsigned char *buff)
{
  return(0);
}
#endif

void osd_on_screen_display(const char *text,int percentage)
{
#if 0
  int i,j,start,avail,cutoff;


  on_screen_display_timer = 2*Machine->drv->frames_per_second;

  strcpy(on_screen_display_text,text);
  strcat(on_screen_display_text," ");

  start = strlen(on_screen_display_text);
  avail = (gfx_display_columns / Machine->uifont->width) * 9 / 10;
  avail -= start;
  cutoff = 8 * avail * percentage / 100;
  for (i = 0;i < avail;i++)
  {
    if (8 * i >= cutoff)
      on_screen_display_text[start + i] = ' ';
    else if (8 * (i + 1) <= cutoff)
      on_screen_display_text[start + i] = 8;
    else
      on_screen_display_text[start + i] = cutoff - 8 * i;
  }
  on_screen_display_text[start + avail] = 0;  /* nul terminate */
#endif
}

int osd_fsize(void *file)
{
  if((((struct File *) file)->Type == FILETYPE_ZIP) || (((struct File *) file)->Type == FILETYPE_CUSTOM))
    return(((struct File *) file)->Length);

  return(0);
}

unsigned int osd_fcrc(void *file)
{
  if((((struct File *) file)->Type == FILETYPE_ZIP) || (((struct File *) file)->Type == FILETYPE_CUSTOM))
    return(((struct File *) file)->CRC);

  return(0);
}

void osd_set_gamma(float gamma)
{
}

float osd_get_gamma(void)
{
  return(1.0);
}

void osd_set_brightness(int brightness)
{
}

int osd_get_brightness(void)
{
  return(100);
}

void osd_profiler(int type)
{
}
void osd_save_snapshot(struct osd_bitmap *bitmap)
{

}

int osd_display_loading_rom_message(const char *name,int current,int total)
{
  return(0);
}
