/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <cstdio>
#include "pico/stdlib.h"
#include "PlayAudio.h"

//#define DEBUG_PLAYAUDIO

spin_lock_t* PlayAudio::spin_lock = nullptr;
audio_buffer_pool_t* PlayAudio::ap = nullptr;
ReadBuffer* PlayAudio::rdbuf = nullptr;
int16_t PlayAudio::buf_s16[SAMPLES_PER_BUFFER*2];
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
    spin_lock = spin_lock_init(spin_lock_claim_unused(true));
    ap = i2s_setup(SAMP_FREQ_44100);
    rdbuf = new ReadBuffer(RDBUF_SIZE, RDBUF_SIZE/4); // auto fill if left is lower than RDBUF_SIZE/4
}

void PlayAudio::finalize()
{
    spin_lock_unclaim(spin_lock_get_num(spin_lock));
    i2s_audio_deinit();
    delete rdbuf;
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

PlayAudio::PlayAudio() : playing(false), paused(false), 
    channels(2), sampRateHz(44100), bitRateKbps(44100*16*2/1000), bitsPerSample(16),
    samplesPlayed(0), levelL(0.0), levelR(0.0)
{
}

PlayAudio::~PlayAudio()
{
}

void PlayAudio::setBufPos(size_t fpos)
{
    if (fpos > 0) {
        rdbuf->seek(fpos);
    }
}

void PlayAudio::play(const char* filename, size_t fpos, uint32_t samplesPlayed)
{
    FRESULT fr;
	fr = f_open(&fil, (TCHAR *) filename, FA_READ);
    rdbuf->bind(&fil);

    setBufPos(fpos);
    setSamplesPlayed(samplesPlayed);

    // Don't manipulate rdbuf after playing = true because decode callback handles it
    playing = true;
    paused = false;
}

void PlayAudio::pause(bool flg)
{
    paused = flg;
}

void PlayAudio::stop()
{
    if (playing) { f_close(&fil); }

    playing = false;
    paused = false;
}

bool PlayAudio::isPlaying()
{
    return playing;
}

bool PlayAudio::isPaused()
{
    return paused;
}

uint16_t PlayAudio::getU16LE(const char* ptr)
{
    return ((uint16_t) ptr[1] << 8) + ((uint16_t) ptr[0]);
}

uint32_t PlayAudio::getU32LE(const char* ptr)
{
    return ((uint32_t) ptr[3] << 24) + ((uint32_t) ptr[2] << 16) + ((uint32_t) ptr[1] << 8) + ((uint32_t) ptr[0]);
}

uint32_t PlayAudio::getU28BE(const char* ptr)
{
    return (((uint32_t) ptr[0] & 0x7f) << 21) + (((uint32_t) ptr[1] & 0x7f) << 14) + (((uint32_t) ptr[2] & 0x7f) << 7) + (((uint32_t) ptr[3] & 0x7f));
}

float PlayAudio::convLevelCurve(uint32_t levelInt) // assume 0 <= level <= 32768
{
    int i;
    for (i = 0; i < 101; i++) {
        if (levelInt*2 < vol_table[i]) { break; }
    }
    return (float) i / 100.0;
}

void PlayAudio::setSamplesPlayed(uint32_t value)
{
    uint32_t save = spin_lock_blocking(spin_lock);
    samplesPlayed = value;
    spin_unlock(spin_lock, save);
}

void PlayAudio::incSamplesPlayed(uint32_t inc)
{
    uint32_t save = spin_lock_blocking(spin_lock);
    samplesPlayed += inc;
    spin_unlock(spin_lock, save);
}

uint32_t PlayAudio::getSamplesPlayed()
{
    uint32_t save = spin_lock_blocking(spin_lock);
    uint32_t value = samplesPlayed;
    spin_unlock(spin_lock, save);
    return value;
}

void PlayAudio::setLevelInt(uint32_t levelIntL, uint32_t levelIntR)
{
    uint32_t save;
    // Level conversion with slow level down
    const float MaxLevelDown = 0.02;

    float levelL_nxt = convLevelCurve(levelIntL);
    save = spin_lock_blocking(spin_lock);
    if (levelL - MaxLevelDown > levelL_nxt) {
        levelL -= MaxLevelDown;
    } else {
        levelL = levelL_nxt;
    }
    spin_unlock(spin_lock, save);

    float levelR_nxt = convLevelCurve(levelIntR);
    save = spin_lock_blocking(spin_lock);
    if (levelR - MaxLevelDown > levelR_nxt) {
        levelR -= MaxLevelDown;
    } else {
        levelR = levelR_nxt;
    }
    spin_unlock(spin_lock, save);
}

void PlayAudio::decode()
{
    // Performs Audio Mute

    audio_buffer_t* buffer;
    if ((buffer = take_audio_buffer(ap, false)) == nullptr) { return; }

    #ifdef DEBUG_PLAYAUDIO
    {
        uint32_t time = to_ms_since_boot(get_absolute_time());
        printf("AUDIO::decode start at %d ms\n", time);
    }
    #endif // DEBUG_PLAYAUDIO

    int32_t* samples = (int32_t *) buffer->buffer->bytes;
    buffer->sample_count = buffer->max_sample_count;
    for (int i = 0; i < buffer->sample_count; i++) {
        samples[i*2+0] = DAC_ZERO;
        samples[i*2+1] = DAC_ZERO;
    }
    give_audio_buffer(ap, buffer);
    levelL = 0.0;
    levelR = 0.0;

    #ifdef DEBUG_PLAYAUDIO
    {
        uint32_t time = to_ms_since_boot(get_absolute_time());
        printf("AUDIO::decode end   at %d ms\n", time);
    }
    #endif // DEBUG_PLAYAUDIO
}

uint32_t PlayAudio::elapsedMillis()
{
    return (uint32_t) ((uint64_t) getSamplesPlayed() * 1000 / sampRateHz);
}

void PlayAudio::getCurrentPosition(size_t* fpos, uint32_t* samplesPlayed)
{
    if (playing) {
        *fpos = rdbuf->tell();
        *samplesPlayed = getSamplesPlayed();
    } else {
        *fpos = 0;
        *samplesPlayed = 0;
    }
}

void PlayAudio::getLevel(float* levelL, float* levelR)
{
    uint32_t save = spin_lock_blocking(spin_lock);
    *levelL = this->levelL;
    *levelR = this->levelR;
    spin_unlock(spin_lock, save);
}