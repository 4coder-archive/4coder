#define ___fred_function function
#undef function
#include <alsa/asoundlib.h>
#include <poll.h>
#define function ___fred_function

internal void
linux_default_mix_sources(void *ctx, f32 *mix_buffer, u32 sample_count)
{

}

internal void
linux_default_mix_destination(i16 *dst, f32 *src, u32 sample_count)
{
    u32 opl = sample_count*2;
    for(u32 i = 0; i < sample_count; i += 1){
        dst[i] = (i16)src[i];
    }
}

internal struct alsa_funcs {
#define ALSA_FN(r,n,a) r (*n) a;
#include "alsa_funcs.txt"
#undef ALSA_FN
} snd_pcm;

internal void
linux_submit_audio(snd_pcm_t* pcm, i16* samples, u32 sample_count, f32* mix_buffer)
{
    Audio_Mix_Sources_Function *audio_mix_src;
    Audio_Mix_Destination_Function *audio_mix_dst;

    pthread_mutex_lock(&linuxvars.audio_mutex);
    audio_mix_src = linuxvars.audio_src_func;
    audio_mix_dst = linuxvars.audio_dst_func;
    void* audio_ctx = linuxvars.audio_ctx;
    pthread_mutex_unlock(&linuxvars.audio_mutex);

    if(!audio_mix_src) {
        audio_mix_src = linux_default_mix_sources;
    }

    if(!audio_mix_dst) {
        audio_mix_dst = linux_default_mix_destination;
    }

    audio_mix_src(audio_ctx, mix_buffer, sample_count);
    audio_mix_dst(samples, mix_buffer, sample_count);

    int err = snd_pcm.writei(pcm, samples, sample_count);
    if(err < 0){
	    snd_pcm.recover(pcm, err, 1);
    }
}

#define chk(x) ({\
	int err = (x);\
	if(err < 0){\
		fprintf(stderr, "ALSA ERR: %s: [%d]\n", #x, err);\
	}\
})

internal void
linux_audio_main(void* _unused)
{
    const u32 SamplesPerSecond = 48000;
    const u32 SamplesPerBuffer = 16*SamplesPerSecond/1000;
    const u32 ChannelCount = 2;
    const u32 BytesPerSample = 2; // S16LE
    const u32 BufferSize = SamplesPerBuffer * BytesPerSample;
    const u32 BufferCount = 3;
    const u32 MixBufferSize = (SamplesPerBuffer * ChannelCount * sizeof(f32));
    const u32 SampleBufferSize = (SamplesPerBuffer * ChannelCount * sizeof(i16));

    void* lib = dlopen("libasound.so.2", RTLD_LOCAL | RTLD_LAZY);
    if(!lib) {
        fprintf(stderr, "failed to load libasound.so.2: %s", dlerror());\
        return;
    }

#define ALSA_FN(r,n,a)\
    *((void**)&snd_pcm.n) = (void*)dlsym(lib, stringify(snd_pcm_##n));\
    if(!snd_pcm.n){\
        fprintf(stderr, "failed to load alsa func: %s", #n);\
        return;\
    }
#include "alsa_funcs.txt"
#undef ALSA_FN

    snd_pcm_t* pcm;

	chk( snd_pcm.open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0));

	     snd_pcm_hw_params_t*                hw;
	chk( snd_pcm.hw_params_malloc          (&hw));
	chk( snd_pcm.hw_params_any             (pcm, hw));
	chk( snd_pcm.hw_params_set_access      (pcm, hw, SND_PCM_ACCESS_RW_INTERLEAVED));
	chk( snd_pcm.hw_params_set_format      (pcm, hw, SND_PCM_FORMAT_S16_LE));
	chk( snd_pcm.hw_params_set_channels    (pcm, hw, ChannelCount));
	chk( snd_pcm.hw_params_set_rate        (pcm, hw, SamplesPerSecond, 0));
	chk( snd_pcm.hw_params_set_buffer_size (pcm, hw, BufferSize * BufferCount));
	chk( snd_pcm.hw_params                 (pcm, hw));
	     snd_pcm.hw_params_free            (hw);

	int fd_count = snd_pcm.poll_descriptors_count(pcm);
	struct pollfd* fds = (struct pollfd*)calloc(fd_count, sizeof(struct pollfd));
	snd_pcm.poll_descriptors(pcm, fds, fd_count);

    for(;;) {
		int n = poll(fds, fd_count, -1);
        if(n == -1) {
            perror("poll");
            continue;
        }

        f32* MixBuffer = (f32*)calloc(1, MixBufferSize);
        i16* SampleBuffer = (i16*)calloc(1, SampleBufferSize);

        if(!MixBuffer || !SampleBuffer) {
            perror("calloc");
            continue;
        }

        linux_submit_audio(pcm, SampleBuffer, SamplesPerBuffer, MixBuffer);

        free(MixBuffer);
        free(SampleBuffer);
    }
}

#undef chk
