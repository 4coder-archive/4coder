/* date = November 23rd 2020 1:18 pm */

#ifndef FCODER_AUDIO_H
#define FCODER_AUDIO_H

////////////////////////////////
// NOTE(allen): Default Mixer Types

struct Audio_Control{
    volatile f32 channel_volume[2];
	volatile u32 generation;
	volatile u32 last_played_sample_index;
    volatile b32 loop;
};

struct Audio_Clip{
    i16 *samples;
    Audio_Control *control;
    f32 channel_volume[2];
    
    u32 sample_count;
    u32 at_sample_index;
};

struct Audio_System{
    volatile u32 quit;
    volatile u32 ticket;
    volatile u32 serving;
	volatile u32 generation;
    
    Audio_Clip playing_clips[64];
    
    // NOTE(casey): Requests to play sounds are written to a pending array to avoid long locking
    volatile u32 pending_clip_count;
    Audio_Clip pending_clips[64];
};

////////////////////////////////
// NOTE(allen): Default Mixer

function void def_audio_init(void);
function void def_audio_play_clip(Audio_Clip clip, Audio_Control *control);
function b32  def_audio_is_playing(Audio_Control *control);
function void def_audio_stop(Audio_Control *control);

function void def_audio_mix_sources(void *ctx, f32 *mix_buffer, u32 sample_count);
function void def_audio_mix_destination(i16 *dst, f32 *src, u32 sample_count);

////////////////////////////////
// NOTE(allen): Loading Clip

function Audio_Clip audio_clip_from_wav_data(String_Const_u8 data);
function Audio_Clip audio_clip_from_wav_FILE(Arena *arena, FILE *file);
function Audio_Clip audio_clip_from_wav_file_name(Arena *arena, char *file_name);

#endif //4CODER_AUDIO_H
