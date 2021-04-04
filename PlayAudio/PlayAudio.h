/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef __PLAY_AUDIO_H_INCLUDED__
#define __PLAY_AUDIO_H_INCLUDED__

#include "pico/audio_i2s.h"
#include "fatfs/ff.h"
#include "audio_init.h"

//=================================
// Interface of PlayAudio Class
//=================================
class PlayAudio
{
public:
    typedef enum {
        AUDIO_CODEC_NONE = 0,
        AUDIO_CODEC_WAV
    } audio_codec_t;
    static void initialize();
    static void volumeUp();
    static void volumeDown();
    static void setVolume(uint8_t value);
    static uint8_t getVolume();
    PlayAudio();
    virtual ~PlayAudio();
    virtual void play(const char *filename);
    void pause(bool flg = true);
    void stop();
    bool isPlaying();
    bool isPaused();
protected:
    static const int32_t DAC_ZERO = 1; // to avoid pop noise caused by auto-mute function of DAC
    static audio_buffer_pool_t *ap;
    static int16_t buf[SAMPLES_PER_BUFFER*2];
    static uint8_t volume;
    static const uint32_t vol_table[101];
    FIL fil;
    bool playing;
    bool paused;
    virtual void decode();
};

#endif // __PLAY_AUDIO_H_INCLUDED__

