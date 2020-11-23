////////////////////////////////
// NOTE(allen): Win32 Audio Functions

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

function void
win32_play_clip(Audio_Clip clip, Audio_Control *control){
    clip.control = control;
    Audio_System *Crunky = &win32vars.audio_system;
    win32_begin_ticket_mutex(Crunky);
    if (Crunky->pending_clip_count < ArrayCount(Crunky->pending_clips))
    {
        Crunky->pending_clips[Crunky->pending_clip_count++] = clip;
    }
    win32_end_ticket_mutex(Crunky);
}

function void
win32_mix_audio(Audio_System *Crunky, u32 SampleCount, i16 *Dest){
    // NOTE(casey): Move pending sounds into the playing list
    win32_begin_ticket_mutex(Crunky);
	++Crunky->generation;
    for(u32 PendIndex = 0;
        PendIndex < Crunky->pending_clip_count;
        ++PendIndex)
    {
        u32 DestIndex = Crunky->next_playing_clip_index++ % ArrayCount(Crunky->playing_clips);
        Crunky->playing_clips[DestIndex] = Crunky->pending_clips[PendIndex];
    }
	Crunky->pending_clip_count = 0;
    win32_end_ticket_mutex(Crunky);
    
    memset(Dest, 0, SampleCount*4);
    for(u32 SoundIndex = 0;
        SoundIndex < ArrayCount(Crunky->playing_clips);
        ++SoundIndex)
    {
        Audio_Clip Sound = Crunky->playing_clips[SoundIndex];
        u32 SamplesToMix = Min((Sound.sample_count - Sound.at_sample_index), SampleCount);
        Crunky->playing_clips[SoundIndex].at_sample_index += SamplesToMix;
        
		i32 LeftVol = AUDIO_PRODUCER_KNOB_ONE;
		i32 RightVol = AUDIO_PRODUCER_KNOB_ONE;
		if(SamplesToMix && Sound.control)
		{
			LeftVol = Sound.control->left_volume_knob;
			RightVol = Sound.control->right_volume_knob;
			Sound.control->generation = Crunky->generation;
			Sound.control->last_played_sample_index = Crunky->playing_clips[SoundIndex].at_sample_index;
		}
		
        
        for(u32 SampleIndex = 0;
            SampleIndex < SamplesToMix;
            ++SampleIndex)
        {
            u32 src_base_indx = 2*(Sound.at_sample_index + SampleIndex);
            u32 dst_base_indx = 2*SampleIndex;
            Dest[dst_base_indx + 0] += (i16)(LeftVol*(i32)Sound.samples[src_base_indx + 0]/AUDIO_PRODUCER_KNOB_ONE);
            Dest[dst_base_indx + 1] += (i16)(RightVol*(i32)Sound.samples[src_base_indx + 1]/AUDIO_PRODUCER_KNOB_ONE);
        }
    }
}

function b32
win32_submit_audio(Audio_System *Crunky, HWAVEOUT WaveOut, WAVEHDR *Header){
    b32 Result = false;
    
    u32 SampleCount = Header->dwBufferLength/4;
    i16 *Samples = (i16 *)Header->lpData;
    win32_mix_audio(Crunky, SampleCount, Samples);
    
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
    u32 TotalAudioMemorySize = BufferCount*TotalBufferSize;
    
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
    
    if(waveOutOpen(&WaveOut, WAVE_MAPPER, &Format, GetCurrentThreadId(), 0, CALLBACK_THREAD) == MMSYSERR_NOERROR)
    {
        void *AudioBufferMemory = VirtualAlloc(0, TotalAudioMemorySize, MEM_COMMIT, PAGE_READWRITE);
        if(AudioBufferMemory)
        {
            u8 *At = (u8 *)AudioBufferMemory;
            for(u32 BufferIndex = 0;
                BufferIndex < BufferCount;
                ++BufferIndex)
            {
                WAVEHDR *Header = (WAVEHDR *)At;
                At += sizeof(WAVEHDR);
                Header->lpData = (char *)At;
                Header->dwBufferLength = TotalBufferSize;
                
                At += TotalBufferSize;
                
                win32_submit_audio(Crunky, WaveOut, Header);
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
    // SetTimer(0, 0, 100, 0);
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
            
            win32_submit_audio(Crunky, WaveOut, Header);
        }
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
