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
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/alib.h>
#include <proto/intuition.h>
#include <proto/ahi.h>
#include <proto/utility.h>
extern "C" {
    #include "exec/types.h"
}

// from mame:
extern "C" {
    #include "osdepend.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "config_moo.h"
static int MasterVolume=0;
static int Attenuation=0;
struct AChannelArray  *ChannelArray[2]={NULL,NULL};
LONG          CurrentArray  = 0;

struct Audio      *Audio=NULL;


#define DEF_BUFFER_SIZE     32768
#define DEF_MIN_FREE_CHIP   (64*1024)

#define MAX_AHI_CHANNEL_TAGS  9
#define MAX_PAULA_FREQUENCY   28867
#define MAX_PAULA_LENGTH    65536

#define MAKE_PERIOD(f) ((f)?(((ULONG)3579547)/f):(65536))

#define INTELULONG(i) (((i)<<24)|((i)>>24)|(((i)<<8)&0x00ff0000)|(((i)>>8)&0x0000ff00))


extern struct Library *UtilityBase;

static inline APTR memAlloc(ULONG size)
{
  return(AllocVec(size, MEMF_PUBLIC|MEMF_CLEAR));
}

static inline void memFree(APTR mem)
{
  FreeVec(mem);
}


struct AChannelArray *AAllocChannelArray(struct Audio *audio, LONG length)
{
  struct AChannelArray  *ca;
  LONG          size;
  LONG          i;
  BYTE          *buffer;

  size = sizeof(struct AChannelArray) + (audio->Channels * (sizeof(struct AChannel) + length));

  ca = (struct AChannelArray *) memAlloc(size);

  if(ca)
  {
    ca->Audio   = audio;
    ca->Size    = size;
    ca->Length    = length;
    ca->Channels  = (struct AChannel *) &ca[1];

    buffer = (BYTE *) & ca->Channels[audio->Channels];

    for(i = 0; i < audio->Channels; i++)
    {
      ca->Channels[i].Buffer = buffer;

      buffer += length;
    }

#ifdef POWERUP
    PPCCacheClearE(ca, ca->Size, CACRF_ClearD);
#endif
  }

  return(ca);
}

void AFreeChannelArray(struct AChannelArray *ca)
{
  if(ca)
    memFree(ca);
}

struct Audio *AllocAudio(Tag tags,...)
{
  struct Audio  *audio;
  struct TagItem  *tag, *taglist;
  LONG      use_ahi;
  LONG      channels;
  LONG      max_sounds;
  LONG      buffer_size;
  BYTE      map_channels[4];
  ULONG     min_free_chip;
  LONG      size;
  LONG      i;

  taglist = (struct TagItem *) &tags;

  use_ahi     = FALSE;
  channels    = 4;
  max_sounds    = 0;
  min_free_chip = DEF_MIN_FREE_CHIP;
  buffer_size   = DEF_BUFFER_SIZE;
  map_channels[0] = 0;
  map_channels[1] = 1;
  map_channels[2] = 2;
  map_channels[3] = 3;

  while((tag = NextTagItem(&taglist)))
  {
    switch(tag->ti_Tag)
    {
      case AA_UseAHI:
        use_ahi = tag->ti_Data;
        break;
      case AA_Channels:
        channels = tag->ti_Data;
        break;
      case AA_MaxSounds:
        max_sounds = tag->ti_Data;
        break;
      case AA_MapChannel0:
        map_channels[0] = tag->ti_Data;
        break;
      case AA_MapChannel1:
        map_channels[1] = tag->ti_Data;
        break;
      case AA_MapChannel2:
        map_channels[2] = tag->ti_Data;
        break;
      case AA_MapChannel3:
        map_channels[3] = tag->ti_Data;
        break;
      case AA_MinFreeChip:
        min_free_chip = tag->ti_Data;
        break;
    }
  }

  size = sizeof(struct Audio);

  if(use_ahi)
    size += ((channels * MAX_AHI_CHANNEL_TAGS) + 1) * sizeof(struct TagItem);
  else
    size += 9 * sizeof(struct IOAudio);

  audio = (struct Audio *)AllocVec(size, MEMF_CLEAR|MEMF_PUBLIC);

  if(audio)
  {
    NewList((struct List *) &audio->Sounds);

    audio->UseAHI   = use_ahi;
    audio->Channels   = channels;
    audio->BufferSize = ((buffer_size + 3) >> 2) << 2;
    audio->MasterVolume = 100;
    audio->NextSound  = 1;
    audio->MinFreeChip  = min_free_chip;

    for(i = 0; i < 4; i++)
    {
      if(map_channels[i] < channels)
        audio->MapChannels[i] = map_channels[i];
      else
        audio->MapChannels[i] = i;
    }

    audio->ChannelArray = AAllocChannelArray(audio, 0);

    if(audio->ChannelArray)
    {
      audio->MsgPort  = CreateMsgPort();

      if(audio->MsgPort)
      {
        if(use_ahi)
        {
          audio->AHITags    = (struct TagItem *) &audio[1];
          audio->AHIRequest = (struct AHIRequest *) CreateIORequest(audio->MsgPort, sizeof(struct AHIRequest));

          if(audio->AHIRequest)
          {
            audio->AHIRequest->ahir_Version = 4;

            if(!OpenDevice(AHINAME, AHI_NO_UNIT, (struct IORequest *) audio->AHIRequest, 0))
            {
              audio->AHIBase  = (struct Library *) audio->AHIRequest->ahir_Std.io_Device;

              audio->AHIAudioCtrl = AHI_AllocAudio( AHIA_Channels,  channels,
                                  AHIA_Sounds,  max_sounds + 1,
                                  TAG_END);

              if(audio->AHIAudioCtrl)
              {
                audio->AHISampleInfo.ahisi_Type   = AHIST_M8S;
                audio->AHISampleInfo.ahisi_Address  = 0;
                audio->AHISampleInfo.ahisi_Length = 0xffffffff;

                if(!AHI_LoadSound(0, AHIST_DYNAMICSAMPLE, &audio->AHISampleInfo, audio->AHIAudioCtrl))
                {
                  AHI_ControlAudio(audio->AHIAudioCtrl, AHIC_Play, TRUE);

                  audio->AHIEffMasterVolume.ahie_Effect = AHIET_MASTERVOLUME;
                  audio->AHIEffMasterVolume.ahiemv_Volume = (audio->Channels >> 1) * 0x10000;

                  AHI_SetEffect(&audio->AHIEffMasterVolume, audio->AHIAudioCtrl);

                  return(audio);
                }

                AHI_FreeAudio(audio->AHIAudioCtrl);
              }

              CloseDevice((struct IORequest *) audio->AHIRequest);
            }

            DeleteIORequest((struct IORequest *) audio->AHIRequest);
          }
        }
        else
        {
          UBYTE ch;

          audio->AudioRequests = (struct IOAudio *) &audio[1];

          for(i = 0; i < 9; i++)
          {
            audio->AudioRequests[i].ioa_Request.io_Message.mn_ReplyPort = audio->MsgPort;
            audio->AudioRequests[i].ioa_Request.io_Message.mn_Length  = sizeof(struct IOAudio);
          }

          audio->Buffers = (BYTE *)AllocVec(8 * audio->BufferSize, MEMF_PUBLIC|MEMF_CHIP);

          if(audio->Buffers)
          {
            ch = 0xf;

            audio->AudioRequests[0].ioa_Request.io_Command  = ADCMD_ALLOCATE;
            audio->AudioRequests[0].ioa_Request.io_Flags  = ADIOF_NOWAIT|IOF_QUICK;
            audio->AudioRequests[0].ioa_Data        = &ch;
            audio->AudioRequests[0].ioa_Length        = 1;

            if(!OpenDevice("audio.device", 0, (struct IORequest *) audio->AudioRequests, 0))
            {
              for(i = 1; i < 9; i++)
              {
                audio->AudioRequests[i].ioa_Request.io_Device = audio->AudioRequests->ioa_Request.io_Device;
                audio->AudioRequests[i].ioa_AllocKey      = audio->AudioRequests->ioa_AllocKey;
              }

              return(audio);
            }

            FreeVec(audio->Buffers);
          }
        }

        DeleteMsgPort(audio->MsgPort);
      }

      AFreeChannelArray(audio->ChannelArray);
    }

    FreeVec(audio);
  }

  return(NULL);
}


void FreeAudio(struct Audio *audio)
{
  struct MinNode  *node;
  struct IOAudio  *ioa;
  LONG      i;

  node = audio->Sounds.mlh_Head->mln_Succ;

  while(node)
  {
    if(audio->UseAHI)
      AHI_UnloadSound(((struct ASound *) node->mln_Pred)->Sound, audio->AHIAudioCtrl);
    else if(((struct ASound *) node->mln_Pred)->AHISampleInfo.ahisi_Address != &((struct ASound *) node->mln_Pred)[1])
      FreeVec(((struct ASound *) node->mln_Pred)->AHISampleInfo.ahisi_Address);

    memFree(node->mln_Pred);

    node = node->mln_Succ;
  }

  if(audio->UseAHI)
  {
    AHI_UnloadSound(0, audio->AHIAudioCtrl);
    AHI_FreeAudio(audio->AHIAudioCtrl);
    CloseDevice((struct IORequest *) audio->AHIRequest);
    DeleteIORequest((struct IORequest *) audio->AHIRequest);
    DeleteMsgPort(audio->MsgPort);
  }
  else
  {
    for(i = 0; i < 4; i++)
    {
      if(audio->Status[i] == AS_PLAYING1)
        AbortIO((struct IORequest *) &audio->AudioRequests[2*i]);
      else if(audio->Status[i] == AS_PLAYING2)
        AbortIO((struct IORequest *) &audio->AudioRequests[2*i+1]);
    }

    while(audio->Status[0] || audio->Status[1] || audio->Status[2] || audio->Status[3])
    {
      WaitPort(audio->MsgPort);

      ioa = (struct IOAudio *) GetMsg(audio->MsgPort);

      if(ioa)
        audio->Status[(((ULONG) ioa) - ((ULONG) audio->AudioRequests)) / (sizeof(struct IOAudio) << 1)] = AS_IDLE;
    }

    CloseDevice((struct IORequest *) audio->AudioRequests);

    FreeVec(audio->Buffers);
  }

  AFreeChannelArray(audio->ChannelArray);

  FreeVec(audio);
}

static inline void ASetMasterVolume(struct AChannelArray *ca, LONG volume)
{
  ca->MasterVolume = volume;
  ca->Flags        = ACF_SetVolume;
}

static inline void ASetFrequency(struct AChannelArray *ca, LONG channel, LONG frequency)
{
  ca->Channels[channel].Frequency =  frequency;
  ca->Channels[channel].Flags     |= ACF_SetFrequency;
}

static inline void ASetVolume(struct AChannelArray *ca, LONG channel, LONG volume)
{
  ca->Channels[channel].Volume =  volume;
  ca->Channels[channel].Flags  |= ACF_SetVolume;
}

static inline void AStopChannel(struct AChannelArray *ca, LONG channel)
{
  ca->Channels[channel].Flags = ACF_Stop;
}

static inline void ARestartChannel(struct AChannelArray *ca, LONG channel)
{
  ca->Channels[channel].Flags = ACF_Restart;
}

// ---------------
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
    if(Audio)
    {
        osd_stop_audio_stream();
    }
    if(Config[CFG_SOUND] != CFGS_NO)
    {
      Audio = AllocAudio( AA_UseAHI,    (Config[CFG_SOUND] == CFGS_AHI) ? TRUE : FALSE,
                AA_Channels,  AUDIO_CHANNELS,
                AA_MaxSounds, 255,
                AA_MinFreeChip, Config[CFG_MINFREECHIP]*1024,
                TAG_END);

      if(Audio)
      {
        ChannelArray[0] = AAllocChannelArray(Audio, AUDIO_BUFFER_LENGTH);
    #ifdef POWERUP
        ChannelArray[1] = AAllocChannelArray(Audio, AUDIO_BUFFER_LENGTH);
    #endif
      }
    }

    return 0;
}
int osd_update_audio_stream(INT16 *buffer)
{
    return 0;
}
void osd_stop_audio_stream(void)
{
    printf("");
    if(!Audio) return;
    FreeAudio(Audio);
    ChannelArray[0] = NULL;
    ChannelArray[1] = NULL;
    Audio = NULL;
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
