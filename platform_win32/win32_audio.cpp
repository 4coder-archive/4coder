////////////////////////////////
// NOTE(allen): Quick Mutex

// TODO(allen): intrinsics wrappers
#include <intrin.h>

function u32
AtomicAddU32AndReturnOriginal(u32 volatile *Value, u32 Addend)
{
    // NOTE(casey): Returns the original value _prior_ to adding
    u32 Result = _InterlockedExchangeAdd((long volatile*)Value, (long)Addend);
    return(Result);
}

global volatile u32 win32_audio_ticket = 0;
global volatile u32 win32_audio_serving = 0;

function void
win32_audio_begin_ticket_mutex(void)
{
    u32 Ticket = AtomicAddU32AndReturnOriginal(&win32_audio_ticket, 1);
    while(Ticket != win32_audio_serving) {_mm_pause();}
}
function void
win32_audio_end_ticket_mutex(void)
{
    AtomicAddU32AndReturnOriginal(&win32_audio_serving, 1);
}

////////////////////////////////
// NOTE(allen): Fallback Mixers

function void
win32_default_mix_sources(void *ctx, f32 *mix_buffer, u32 sample_count){
}

function void
win32_default_mix_destination(i16 *dst, f32 *src, u32 sample_count){
    u32 opl = sample_count*2;
    for(u32 i = 0; i < sample_count; i += 1){
        dst[i] = (i16)src[i];
    }
}

////////////////////////////////
// NOTE(allen): Win32 Audio System API

internal
system_set_source_mixer_sig(){
    win32_audio_begin_ticket_mutex();
    win32vars.audio_mix_ctx = ctx;
    win32vars.audio_mix_sources = mix_func;
    win32_audio_end_ticket_mutex();
}

internal
system_set_destination_mixer_sig(){
    win32_audio_begin_ticket_mutex();
    win32vars.audio_mix_destination = mix_func;
    win32_audio_end_ticket_mutex();
}

////////////////////////////////
// NOTE(allen): Win32 Audio Loop

function b32
win32_submit_audio(HWAVEOUT WaveOut, WAVEHDR *Header, u32 SampleCount, f32 *mix_buffer){
    b32 Result = false;
    
    // NOTE(allen): prep buffers
    u32 mix_buffer_size = SampleCount*2*sizeof(f32);
    memset(mix_buffer, 0, mix_buffer_size);
    i16 *Samples = (i16 *)Header->lpData;
    
    // NOTE(allen): prep mixer pointers
    win32_audio_begin_ticket_mutex();
    void *audio_mix_ctx = win32vars.audio_mix_ctx;
    Audio_Mix_Sources_Function *audio_mix_sources = win32vars.audio_mix_sources;
    Audio_Mix_Destination_Function *audio_mix_destination = win32vars.audio_mix_destination;
    win32_audio_end_ticket_mutex();
    
    if (audio_mix_sources == 0){
        audio_mix_sources = win32_default_mix_sources;
    }
    if (audio_mix_destination == 0){
        audio_mix_destination = win32_default_mix_destination;
    }
    
    // NOTE(allen): mix
    audio_mix_sources(audio_mix_ctx, mix_buffer, SampleCount);
    audio_mix_destination(Samples, mix_buffer, SampleCount);
    
    // NOTE(allen): send final samples to win32
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
    
    b32 quit = false;
    
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
                
                win32_submit_audio(WaveOut, Header, SamplesPerBuffer, (f32*)MixBuffer);
            }
        }
        else
        {
            // TODO(allen): audio error
            quit = true;
        }
    }
    else
    {
        // TODO(allen): audio error
        quit = true;
    }
    
    //
    // NOTE(casey): Serve audio forever (until we are told to stop)
    //
    
    SetTimer(0, 0, 100, 0);
    for (;!quit;)
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
            
            win32_submit_audio(WaveOut, Header, SamplesPerBuffer, (f32*)MixBuffer);
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
win32_audio_init(void){
    DWORD thread_id = 0;
    HANDLE handle = CreateThread(0, 0, win32_audio_loop, 0, 0, &thread_id);
    CloseHandle(handle);
    return(thread_id);
}
