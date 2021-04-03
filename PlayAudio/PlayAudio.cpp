/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <cstdio>
#include "pico/stdlib.h"
#include "PlayAudio.h"

//#define DEBUG_PLAYAUDIO

audio_buffer_pool_t *PlayAudio::ap = nullptr;
int16_t PlayAudio::buf[SAMPLES_PER_BUFFER*2];

void PlayAudio::initialize()
{
    ap = audio_init();
}

PlayAudio::PlayAudio() : playing(false), pausing(false)
{
}

PlayAudio::~PlayAudio()
{
}

void PlayAudio::play(const char *filename)
{
    FRESULT fr;
	fr = f_open(&fil, (TCHAR *) filename, FA_READ);

    playing = true;
    pausing = false;
}

void PlayAudio::pause(bool flg)
{
    pausing = flg;
}

void PlayAudio::stop()
{
    playing = false;
    pausing = false;
    f_close(&fil);
}

bool PlayAudio::is_playing()
{
    return playing;
}

bool PlayAudio::is_pausing()
{
    return pausing;
}

void PlayAudio::decode()
{
    // Performs Audio Mute

    audio_buffer_t *buffer;
    if ((buffer = take_audio_buffer(ap, false)) == nullptr) { return; }

    #ifdef DEBUG_PLAYAUDIO
    {
        uint32_t time = to_ms_since_boot(get_absolute_time());
        printf("AUDIO::decode start at %d ms\n", time);
    }
    #endif // DEBUG_PLAYAUDIO

    int32_t *samples = (int32_t *) buffer->buffer->bytes;
    buffer->sample_count = buffer->max_sample_count;
    for (int i = 0; i < buffer->sample_count; i++) {
        samples[i*2+0] = DAC_ZERO;
        samples[i*2+1] = DAC_ZERO;
    }
    give_audio_buffer(ap, buffer);

    #ifdef DEBUG_PLAYAUDIO
    {
        uint32_t time = to_ms_since_boot(get_absolute_time());
        printf("AUDIO::decode end   at %d ms\n", time);
    }
    #endif // DEBUG_PLAYAUDIO
}