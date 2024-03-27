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
//#include <intuition/intuition.h>
//#include <devices/keyboard.h>
//#include <devices/keymap.h>

extern "C" {
    #include "exec/types.h"
    //#include "input.h"
}

// from mame:
extern "C" {
    #include "osdepend.h"
    //#include "input.h"
}

#include "audio.h"
static int MasterVolume=0;
static int Attenuation=0;
struct AChannelArray  *ChannelArray[2]={NULL,NULL};
LONG          CurrentArray  = 0;

/*
  osd_start_audio_stream() is called at the start of the emulation to initialize
  the output stream, then osd_update_audio_stream() is called every frame to
  feed new data. osd_stop_audio_stream() is called when the emulation is stopped.

  The sample rate is fixed at Machine->sample_rate. Samples are 16-bit, signed.
  When the stream is stereo, left and right samples are alternated in the
  stream.

  osd_start_audio_stream() and osd_update_audio_stream() must return the number
  of samples (or couples of samples, when using stereo) required for next frame.
  This will be around Machine->sample_rate / Machine->drv->frames_per_second,
  the code may adjust it by SMALL AMOUNTS to keep timing accurate and to
  maintain audio and video in sync when using vsync. Note that sound emulation,
  especially when DACs are involved, greatly depends on the number of samples
  per frame to be roughly constant, so the returned value must always stay close
  to the reference value of Machine->sample_rate / Machine->drv->frames_per_second.
  Of course that value is not necessarily an integer so at least a +/- 1
  adjustment is necessary to avoid drifting over time.
*/
int osd_start_audio_stream(int stereo)
{
    return 0;
}
int osd_update_audio_stream(INT16 *buffer)
{
    return 0;
}
void osd_stop_audio_stream(void)
{

}

/*
  control master volume. attenuation is the attenuation in dB (a negative
  number). To convert from dB to a linear volume scale do the following:
    volume = MAX_VOLUME;
    while (attenuation++ < 0)
        volume /= 1.122018454;      //  = (10 ^ (1/20)) = 1dB
*/

void osd_set_mastervolume(int attenuation)
{
//	float volume;

//	Attenuation = attenuation;

// 	volume = 256.0;	/* range is 0-256 */

//	while(attenuation++ < 0)
//		volume /= 1.122018454;	/* = (10 ^ (1/20)) = 1dB */

//  MasterVolume = volume;

//printf("osd_set_mastervolume:%08x\n",(int));
//  if(ChannelArray[0])
//  {
//#ifdef POWERUP
//    if(!ChannelArray[1])
//      return;
//#endif
//    ASetMasterVolume(ChannelArray[CurrentArray], MasterVolume);
//  }
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
