/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <cstdio>
#include <cstring>
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

void PlayWav::decode()
{
    if (!playing || paused) {
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

    int32_t *samples = (int32_t *) buffer->buffer->bytes;
    if (rdbuf->getLeft()/4 >= buffer->max_sample_count) {
        buffer->sample_count = buffer->max_sample_count;
    } else {
        buffer->sample_count = rdbuf->getLeft()/4;
    }
    memcpy(buf_s16, rdbuf->buf(), buffer->sample_count*4);
    for (int i = 0; i < buffer->sample_count; i++) {
        samples[i*2+0] = (int32_t) buf_s16[i*2+0] * vol_table[volume] + DAC_ZERO;
        samples[i*2+1] = (int32_t) buf_s16[i*2+1] * vol_table[volume] + DAC_ZERO;
    }
    give_audio_buffer(ap, buffer);
    rdbuf->shift(buffer->sample_count*4);
    if (rdbuf->getLeft() == 0) { stop(); }

    #ifdef DEBUG_PLAYWAV
    {
        uint32_t time = to_ms_since_boot(get_absolute_time());
        printf("WAV::decode end   at %d ms\n", time);
    }
    #endif // DEBUG_PLAYWAV
}