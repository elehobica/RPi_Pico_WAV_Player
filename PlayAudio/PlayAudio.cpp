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
uint8_t PlayAudio::volume = 65;
const uint32_t PlayAudio::vol_table[101] = {
    0, 4, 8, 12, 16, 20, 24, 27, 29, 31,
    34, 37, 40, 44, 48, 52, 57, 61, 67, 73,
    79, 86, 94, 102, 111, 120, 131, 142, 155, 168,
    183, 199, 217, 236, 256, 279, 303, 330, 359, 390, // vol_table[34] = 256
    424, 462, 502, 546, 594, 646, 703, 764, 831, 904,
    983, 1069, 1163, 1265, 1376, 1496, 1627, 1770, 1925, 2094,
    2277, 2476, 2693, 2929, 3186, 3465, 3769, 4099, 4458, 4849,
    5274, 5736, 6239, 6785, 7380, 8026, 8730, 9495, 10327, 11232,
    12216, 13286, 14450, 15716, 17093, 18591, 20220, 21992, 23919, 26015,
    28294, 30773, 33470, 36403, 39592, 43061, 46835, 50938, 55402, 60256,
    65536
};

void PlayAudio::initialize()
{
    ap = audio_init();
}
void PlayAudio::volumeUp()
{
    if (volume < 100) { volume++; }
}

void PlayAudio::volumeDown()
{
    if (volume > 0) { volume--; }
}

void PlayAudio::setVolume(uint8_t value)
{
    volume = (value <= 100) ? value : 100;
}

uint8_t PlayAudio::getVolume()
{
    return volume;
}

PlayAudio::PlayAudio() : playing(false), paused(false)
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
    paused = false;
}

void PlayAudio::pause(bool flg)
{
    paused = flg;
}

void PlayAudio::stop()
{
    playing = false;
    paused = false;
    f_close(&fil);
}

bool PlayAudio::isPlaying()
{
    return playing;
}

bool PlayAudio::isPaused()
{
    return paused;
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