/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <cstdio>
#include "pico/stdlib.h"
#include "PlayWav.h"

//#define DEBUG_PLAYWAV

PlayWav *PlayWav::g_inst = nullptr;

void PlayWav::decode_func()
{
    if (g_inst == nullptr) { return; }
    g_inst->decode();
}

PlayWav::PlayWav() : PlayAudio()
{
    g_inst = this;
}

PlayWav::~PlayWav()
{
}

void PlayWav::play(const char *filename)
{
    PlayAudio::play(filename);
}

void PlayWav::decode()
{
    if (!playing || pausing) {
        PlayAudio::decode();
        return;
    }

    audio_buffer_t *buffer;
    if ((buffer = take_audio_buffer(ap, false)) == nullptr) { return; }

    #ifdef DEBUG_PLAYWAV
    {
        uint32_t time = to_ms_since_boot(get_absolute_time());
        printf("WAV::decode start at %d ms\n", time);
    }
    #endif // DEBUG_PLAYWAV

    FRESULT fr;
    UINT br;
    int32_t *samples = (int32_t *) buffer->buffer->bytes;
    fr = f_read(&fil, buf, buffer->max_sample_count*4, &br);
    buffer->sample_count = br/4;
    for (int i = 0; i < buffer->sample_count; i++) {
        samples[i*2+0] = buf[i*2+0] * 8192 + DAC_ZERO;
        samples[i*2+1] = buf[i*2+1] * 8192 + DAC_ZERO;
    }
    give_audio_buffer(ap, buffer);
    if (f_eof(&fil)) { stop(); }

    #ifdef DEBUG_PLAYWAV
    {
        uint32_t time = to_ms_since_boot(get_absolute_time());
        printf("WAV::decode end   at %d ms\n", time);
    }
    #endif // DEBUG_PLAYWAV
}