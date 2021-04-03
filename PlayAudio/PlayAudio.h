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
    static void initialize();
    PlayAudio();
    virtual ~PlayAudio();
    virtual void play(const char *filename);
    void pause(bool flg = true);
    void stop();
    bool is_playing();
    bool is_pausing();
protected:
    static const int32_t DAC_ZERO = 1; // to avoid pop noise caused by auto-mute function of DAC
    static audio_buffer_pool_t *ap;
    static int16_t buf[SAMPLES_PER_BUFFER*2];
    FIL fil;
    bool playing;
    bool pausing;
    virtual void decode();
};

#endif // __PLAY_AUDIO_H_INCLUDED__

