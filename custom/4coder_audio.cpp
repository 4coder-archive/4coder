////////////////////////////////
// NOTE(allen): Load Clip

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

#include <stdlib.h>

function Audio_Clip
audio_clip_from_wav_file_name(char *file_name){
    String_Const_u8 data = {};
    FILE *file = fopen(file_name, "rb");
    if (file != 0){
        fseek(file, 0, SEEK_END);
        data.size = ftell(file);
        data.str = (u8*)malloc(data.size);
        if (data.str != 0 && data.size > 0){
            fseek(file, 0, SEEK_SET);
            fread(data.str, data.size, 1, file);
        }
        fclose(file);
    }
    
    Audio_Clip result = audio_clip_from_wav_data(data);
    return(result);
}
