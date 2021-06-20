/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef __AUDIO_CODEC_H_INCLUDED__
#define __AUDIO_CODEC_H_INCLUDED__

#include "pico/audio_i2s.h"
#include "PlayAudio.h"

void audio_codec_init();
PlayAudio *get_audio_codec();
PlayAudio *set_audio_codec(PlayAudio::audio_codec_t audio_codec);
void audio_codec_deinit();
extern "C" {
void i2s_callback_func();
}

#endif // __AUDIO_CODEC_H_INCLUDED__