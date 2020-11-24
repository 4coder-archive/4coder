////////////////////////////////
// NOTE(allen): Win32 Audio Helpers

function u32
AtomicAddU32AndReturnOriginal(u32 volatile *Value, u32 Addend)
{
    // NOTE(casey): Returns the original value _prior_ to adding
    u32 Result = _InterlockedExchangeAdd(Value, Addend);
    return(Result);
}

function void
win32_begin_ticket_mutex(Audio_System *Crunky)
{
    u32 Ticket = AtomicAddU32AndReturnOriginal(&Crunky->ticket, 1);
    while(Ticket != Crunky->serving) {_mm_pause();}
}

function void
win32_end_ticket_mutex(Audio_System *Crunky)
{
    AtomicAddU32AndReturnOriginal(&Crunky->serving, 1);
}

////////////////////////////////
// NOTE(allen): Win32 Audio System API

internal
system_play_clip_sig(){
    clip.control = control;
    Audio_System *Crunky = &win32vars.audio_system;
    win32_begin_ticket_mutex(Crunky);
    if (Crunky->pending_clip_count < ArrayCount(Crunky->pending_clips))
    {
        Crunky->pending_clips[Crunky->pending_clip_count++] = clip;
    }
    win32_end_ticket_mutex(Crunky);
}

internal
system_audio_is_playing_sig(){
    Audio_System *Crunky = (Audio_System *)&win32vars.audio_system;
    b32 result = (Crunky->generation - control->generation < 2);
    return(result);
}

internal
system_audio_stop_sig(){
    Audio_System *Crunky = (Audio_System *)&win32vars.audio_system;
    win32_begin_ticket_mutex(Crunky);
    
    Audio_Clip *clip = Crunky->playing_clips;
    for(u32 i = 0;
        i < ArrayCount(Crunky->playing_clips);
        i += 1, clip += 1){
        if (clip->control == control){
            clip->at_sample_index = clip->sample_count;
        }
    }
	control->loop = false;
    
    win32_end_ticket_mutex(Crunky);
}

////////////////////////////////
// NOTE(allen): Win32 Audio Loop

function void
win32_mix_audio(Audio_System *Crunky, u32 SampleCount, i16 *Dest, void *mix_buffer_memory){
    // NOTE(casey): Clear the output buffer
    u32 MixBufferSize = SampleCount*2*sizeof(f32);
    f32 *MixBuffer = (f32 *)mix_buffer_memory;
    memset(MixBuffer, 0, MixBufferSize);
    
    win32_begin_ticket_mutex(Crunky);
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
    win32_end_ticket_mutex(Crunky);
    
    // NOTE(casey): Mix all sounds into the output buffer
    {
        Audio_Clip *clip = Crunky->playing_clips;
        for(u32 SoundIndex = 0;
            SoundIndex < ArrayCount(Crunky->playing_clips);
            SoundIndex += 1, clip += 1)
        {
            Audio_Control *control = clip->control;
            if (control != 0 && control->loop && clip->at_sample_index == clip->sample_count){
                clip->at_sample_index = 0;
            }
            
            // NOTE(casey): Determine how many samples are left to play in this
            // sound (possible none)
            u32 SamplesToMix = Min((clip->sample_count - clip->at_sample_index), SampleCount);
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
                u32 src_index = 2*(clip->at_sample_index + SampleIndex);
                f32 Left = LeftVol*(f32)clip->samples[src_index + 0];
                f32 Right = RightVol*(f32)clip->samples[src_index + 1];
                
                u32 dst_index = 2*SampleIndex;
                MixBuffer[dst_index + 0] += Left;
                MixBuffer[dst_index + 1] += Right;
            }
        }
        
        for(u32 SampleIndex = 0;
            SampleIndex < SampleCount;
            ++SampleIndex)
        {
            f32 Left = MixBuffer[2*SampleIndex + 0];
            f32 Right = MixBuffer[2*SampleIndex + 1];
            
            Dest[2*SampleIndex + 0] = (i16)Left;
            Dest[2*SampleIndex + 1] = (i16)Right;
        }
    }
}

function b32
win32_submit_audio(Audio_System *Crunky, HWAVEOUT WaveOut, WAVEHDR *Header, u32 SampleCount, void *mix_buffer){
    b32 Result = false;
    
    i16 *Samples = (i16 *)Header->lpData;
    win32_mix_audio(Crunky, SampleCount, Samples, mix_buffer);
    
    DWORD Error = waveOutPrepareHeader(WaveOut, Header, sizeof(*Header));
    if(Error == MMSYSERR_NOERROR)
    {
        Error = waveOutWrite(WaveOut, Header, sizeof(*Header));
        if(Error == MMSYSERR_NOERROR)
        {
            // NOTE(casey): Success
            Result = true;
        }
    }
    
    return(Result);
}

function DWORD WINAPI
win32_audio_loop(void *Passthrough){
    Audio_System *Crunky = (Audio_System *)Passthrough;
    
    //
    // NOTE(casey): Set up our audio output buffer
    //
    u32 SamplesPerSecond = 48000;
    u32 SamplesPerBuffer = 16*SamplesPerSecond/1000;
    u32 ChannelCount = 2;
    u32 BytesPerChannelValue = 2;
    u32 BytesPerSample = ChannelCount*BytesPerChannelValue;
    
    u32 BufferCount = 3;
    u32 BufferSize = SamplesPerBuffer*BytesPerSample;
    u32 HeaderSize = sizeof(WAVEHDR);
    u32 TotalBufferSize = (BufferSize+HeaderSize);
    u32 MixBufferSize = (SamplesPerBuffer*ChannelCount*sizeof(f32));
    u32 TotalAudioMemorySize = BufferCount*TotalBufferSize + MixBufferSize;
    
    //
    // NOTE(casey): Initialize audio out
    //
    HWAVEOUT WaveOut = {};
    
    WAVEFORMATEX Format = {};
    Format.wFormatTag = WAVE_FORMAT_PCM;
    Format.nChannels = (WORD)ChannelCount;
    Format.wBitsPerSample = (WORD)(8*BytesPerChannelValue);
    Format.nSamplesPerSec = SamplesPerSecond;
    Format.nBlockAlign = (Format.nChannels*Format.wBitsPerSample)/8;
    Format.nAvgBytesPerSec = Format.nBlockAlign * Format.nSamplesPerSec;
    
    void *MixBuffer = 0;
    void *AudioBufferMemory = 0;
    if(waveOutOpen(&WaveOut, WAVE_MAPPER, &Format, GetCurrentThreadId(), 0, CALLBACK_THREAD) == MMSYSERR_NOERROR)
    {
        AudioBufferMemory = VirtualAlloc(0, TotalAudioMemorySize, MEM_COMMIT, PAGE_READWRITE);
        if(AudioBufferMemory)
        {
            u8 *At = (u8 *)AudioBufferMemory;
			MixBuffer = At;
            At += MixBufferSize;
            
            for(u32 BufferIndex = 0;
                BufferIndex < BufferCount;
                ++BufferIndex)
            {
                WAVEHDR *Header = (WAVEHDR *)At;
                Header->lpData = (char *)(Header + 1);
                Header->dwBufferLength = BufferSize;
                
                At += TotalBufferSize;
                
                win32_submit_audio(Crunky, WaveOut, Header, SamplesPerBuffer, MixBuffer);
            }
        }
        else
        {
            Crunky->quit = true;
        }
    }
    else
    {
        Crunky->quit = true;
    }
    
    //
    // NOTE(casey): Serve audio forever (until we are told to stop)
    //
    
    SetTimer(0, 0, 100, 0);
    while(!Crunky->quit)
    {
        MSG Message = {};
        GetMessage(&Message, 0, 0, 0);
        if(Message.message == MM_WOM_DONE)
        {
            WAVEHDR *Header = (WAVEHDR *)Message.lParam;
            if(Header->dwFlags & WHDR_DONE)
            {
                Header->dwFlags &= ~WHDR_DONE;
            }
            
            waveOutUnprepareHeader(WaveOut, Header, sizeof(*Header));
            
            win32_submit_audio(Crunky, WaveOut, Header, SamplesPerBuffer, MixBuffer);
        }
    }
    
    if(WaveOut)
    {
        waveOutClose(WaveOut);
    }
    
    if(AudioBufferMemory)
    {
        VirtualFree(AudioBufferMemory, 0, MEM_RELEASE);
    }
    
    return(0);
}

function DWORD
win32_audio_init(Audio_System *audio_system){
    DWORD thread_id = 0;
    HANDLE handle = CreateThread(0, 0, win32_audio_loop, audio_system, 0, &thread_id);
    CloseHandle(handle);
    return(thread_id);
}
