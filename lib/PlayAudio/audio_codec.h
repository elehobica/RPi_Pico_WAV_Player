/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#pragma once

#include "pico/audio_i2s.h"
#include "PlayAudio.h"

void audio_codec_init();
void audio_codec_deinit();
void audio_codec_set_dac_enable_func(void (*func)(bool flag));
void audio_codec_dac_enable(bool flag);
PlayAudio* get_audio_codec();
PlayAudio* set_audio_codec(PlayAudio::audio_codec_t audio_codec);
extern "C" {
void i2s_callback_func();
}
