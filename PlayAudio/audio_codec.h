/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef __AUDIO_CODEC_H_INCLUDED__
#define __AUDIO_CODEC_H_INCLUDED__

#include "pico/audio_i2s.h"
#include "PlayAudio.h"

typedef enum {
    AUDIO_CODEC_NONE = 0,
    AUDIO_CODEC_WAV
} audio_codec_t;

void audio_codec_init();
PlayAudio *getAudioCodec(audio_codec_t audio_codec);
extern "C" {
void i2s_callback_func();
}

#endif // __AUDIO_CODEC_H_INCLUDED__

