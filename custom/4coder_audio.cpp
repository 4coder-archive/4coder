////////////////////////////////
// NOTE(allen): Default Mixer Helpers

// TODO(allen): intrinsics wrappers
#if OS_LINUX
#include <immintrin.h>
#define _InterlockedExchangeAdd __sync_fetch_and_add
#elif OS_MAC
#include <immintrin.h>
#define _InterlockedExchangeAdd __sync_fetch_and_add
#else
#include <intrin.h>
#endif

function u32
AtomicAddU32AndReturnOriginal(u32 volatile *Value, u32 Addend)
{
 // NOTE(casey): Returns the original value _prior_ to adding
 u32 Result = _InterlockedExchangeAdd((long volatile*)Value, (long)Addend);
 return(Result);
}

function void
def_audio_begin_ticket_mutex(Audio_System *Crunky)
{
 u32 Ticket = AtomicAddU32AndReturnOriginal(&Crunky->ticket, 1);
 while(Ticket != Crunky->serving) {_mm_pause();}
}

function void
def_audio_end_ticket_mutex(Audio_System *Crunky)
{
 AtomicAddU32AndReturnOriginal(&Crunky->serving, 1);
}


////////////////////////////////
// NOTE(allen): Default Mixer

global Audio_System def_audio_system = {};

function void
def_audio_init(void){
 block_zero_struct(&def_audio_system);
 system_set_source_mixer(&def_audio_system, def_audio_mix_sources);
 system_set_destination_mixer(def_audio_mix_destination);
}

function void
def_audio_play_clip(Audio_Clip clip, Audio_Control *control){
 clip.control = control;
 Audio_System *Crunky = &def_audio_system;
 def_audio_begin_ticket_mutex(Crunky);
 if (Crunky->pending_clip_count < ArrayCount(Crunky->pending_clips))
 {
  Crunky->pending_clips[Crunky->pending_clip_count++] = clip;
 }
 def_audio_end_ticket_mutex(Crunky);
}

internal b32
def_audio_is_playing(Audio_Control *control){
 Audio_System *Crunky = &def_audio_system;
 b32 result = (Crunky->generation - control->generation < 2);
 return(result);
}

internal void
def_audio_stop(Audio_Control *control){
 Audio_System *Crunky = &def_audio_system;
 def_audio_begin_ticket_mutex(Crunky);
 
 Audio_Clip *clip = Crunky->playing_clips;
 for(u32 i = 0;
     i < ArrayCount(Crunky->playing_clips);
     i += 1, clip += 1){
  if (clip->control == control){
   clip->at_sample_index = clip->sample_count;
   clip->control = 0;
  }
 }
 control->loop = false;
 
 def_audio_end_ticket_mutex(Crunky);
}

function void
def_audio_mix_sources(void *ctx, f32 *mix_buffer, u32 sample_count){
 Audio_System *Crunky = (Audio_System*)ctx;
 def_audio_begin_ticket_mutex(Crunky);
 // NOTE(casey): Move pending sounds into the playing list
 {
  Crunky->generation += 1;
  u32 PendIndex = 0;
  Audio_Clip *clip = Crunky->playing_clips;
  for(u32 DestIndex = 0;
      (DestIndex < ArrayCount(Crunky->playing_clips)) && (PendIndex < Crunky->pending_clip_count);
      DestIndex += 1, clip += 1)
  {
   if (clip->at_sample_index == clip->sample_count)
   {
    Audio_Control *control = clip->control;
    if (control == 0 || !control->loop){
     *clip = Crunky->pending_clips[PendIndex++];
    }
   }
  }
  Crunky->pending_clip_count = 0;
 }
 def_audio_end_ticket_mutex(Crunky);
 
 // NOTE(casey): Mix all sounds into the output buffer
 {
  Audio_Clip *clip = Crunky->playing_clips;
  for(u32 SoundIndex = 0;
      SoundIndex < ArrayCount(Crunky->playing_clips);
      SoundIndex += 1, clip += 1)
  {
   // NOTE(allen): Determine starting point
   Audio_Control *control = clip->control;
   if (control != 0 && control->loop && clip->at_sample_index == clip->sample_count){
    clip->at_sample_index = 0;
   }
   u32 base_sample_index = clip->at_sample_index;
   
   // NOTE(casey): Determine how many samples are left to play in this
   // sound (possible none)
   u32 SamplesToMix = clamp_top((clip->sample_count - clip->at_sample_index), sample_count);
   clip->at_sample_index += SamplesToMix;
   
   // NOTE(casey): Load the volume out of the control if there is one,
   // and if there is, update the generation and sample index so
   // external controllers can take action
   f32 LeftVol = clip->channel_volume[0];
   f32 RightVol = clip->channel_volume[1];
   if(SamplesToMix && control != 0)
   {
    LeftVol *= control->channel_volume[0];
    RightVol *= control->channel_volume[1];
    control->generation = Crunky->generation;
    control->last_played_sample_index = clip->at_sample_index;
   }
   
   // NOTE(casey): Mix samples
   for(u32 SampleIndex = 0;
       SampleIndex < SamplesToMix;
       ++SampleIndex)
   {
    u32 src_index = 2*(base_sample_index + SampleIndex);
    f32 Left  = LeftVol *(f32)clip->samples[src_index + 0];
    f32 Right = RightVol*(f32)clip->samples[src_index + 1];
    
    u32 dst_index = 2*SampleIndex;
    mix_buffer[dst_index + 0] += Left;
    mix_buffer[dst_index + 1] += Right;
   }
  }
 }
}

function void
def_audio_mix_destination(i16 *dst, f32 *src, u32 sample_count){
 u32 opl = sample_count*2;
 for(u32 i = 0; i < opl; i += 1){
  f32 sample = src[i];
  f32 sat_sample = clamp(-32768.f, sample, 32767.f);
  dst[i] = (i16)sat_sample;
 }
}


////////////////////////////////
// NOTE(allen): Loading Clip

#if !defined(FCODER_SKIP_WAV)
#define FCODER_SKIP_WAV
#pragma pack(push, 1)
struct wave_fmt_data
{
 u16 wFormatTag;
 u16 wChannels;
 u32 dwSamplesPerSec;
 u32 dwAvgBytesPerSec;
 u16 wBlockAlign;
 u16 wBitsPerSample;
};

struct riff_header
{
 u32 ID;
 u32 DataSize;
};
#pragma pack(pop)
#endif

function Audio_Clip
audio_clip_from_wav_data(String_Const_u8 data){
 Audio_Clip Result = {};
 
 if (data.size >= 4 && *(u32 *)data.str == *(u32 *)"RIFF"){
  // NOTE(casey): This ROM is in WAV format
  
  riff_header *RootHeader = (riff_header *)data.str;
  
  wave_fmt_data *Format = 0;
  u32 SampleDataSize = 0;
  i16 *Samples = 0;
  
  u32 At = sizeof(riff_header);
  u32 LastAt = At + ((RootHeader->DataSize + 1) & ~1);
  if ((*(u32 *)(data.str + At) == *(u32 *)"WAVE") &&
      (LastAt <= data.size)){
   At += sizeof(u32);
   while (At < LastAt){
    riff_header *Header = (riff_header *)(data.str + At);
    u32 DataAt = At + sizeof(riff_header);
    u32 EndAt = DataAt + ((Header->DataSize + 1) & ~1);
    if(EndAt <= data.size)
    {
     void *Data = (data.str + DataAt);
     if(Header->ID == *(u32 *)"fmt ")
     {
      Format = (wave_fmt_data *)Data;
     }
     else if(Header->ID == *(u32 *)"data")
     {
      SampleDataSize = Header->DataSize;
      Samples = (i16 *)Data;
     }
    }
    
    At = EndAt;
   }
  }
  
  if (Format &&
      Samples &&
      (Format->wFormatTag == 1) &&
      (Format->wChannels == 2) &&
      (Format->wBitsPerSample == 16) &&
      (Format->dwSamplesPerSec == 48000)){
   for (u32 i = 0; i < 2; i += 1){
    Result.channel_volume[i] = 1.f;
   }
   Result.sample_count = SampleDataSize / (Format->wChannels*Format->wBitsPerSample/8);
   Result.samples = (i16 *)Samples;
  }
  else{
   // TODO(casey): This is where you would output an error - to 4coder somehow?
  }
 }
 else{
  // TODO(casey): This is where you would output an error - to 4coder somehow?
 }
 
 return(Result);
}

function Audio_Clip
audio_clip_from_wav_FILE(Arena *arena, FILE *file){
 String_Const_u8 data = data_from_file(arena, file);
 Audio_Clip result = audio_clip_from_wav_data(data);
 return(result);
}

function Audio_Clip
audio_clip_from_wav_file_name(Arena *arena, char *file_name){
 Audio_Clip result = {};
 String_Const_u8 data = {};
 FILE *file = fopen(file_name, "rb");
 if (file != 0){
  result = audio_clip_from_wav_FILE(arena, file);
  fclose(file);
 }
 return(result);
}
