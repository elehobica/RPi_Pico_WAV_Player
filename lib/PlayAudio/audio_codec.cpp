/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <cstdio>
#include "pico/stdlib.h"
#include "PlayNone.h"
#include "PlayWav.h"
#include "audio_codec.h"

static PlayAudio *playAudio_ary[2] = {};
static void (*decode_func_ary[2])() = {};
static PlayAudio::audio_codec_t cur_audio_codec = PlayAudio::AUDIO_CODEC_NONE;

void audio_codec_init()
{
    PlayAudio::initialize();
    playAudio_ary[PlayAudio::AUDIO_CODEC_NONE] = (PlayAudio *) new PlayNone();
    playAudio_ary[PlayAudio::AUDIO_CODEC_WAV]  = (PlayAudio *) new PlayWav();
    decode_func_ary[PlayAudio::AUDIO_CODEC_NONE] = PlayNone::decode_func;
    decode_func_ary[PlayAudio::AUDIO_CODEC_WAV]  = PlayWav::decode_func;
    cur_audio_codec = PlayAudio::AUDIO_CODEC_NONE;
}

void audio_codec_deinit()
{
    PlayAudio::finalize();
    delete playAudio_ary[PlayAudio::AUDIO_CODEC_NONE];
    delete playAudio_ary[PlayAudio::AUDIO_CODEC_WAV];
}

PlayAudio *get_audio_codec()
{
    return playAudio_ary[cur_audio_codec];
}

PlayAudio *set_audio_codec(PlayAudio::audio_codec_t audio_codec)
{
    cur_audio_codec = audio_codec;
    return get_audio_codec();
}

// callback from:
//   void __isr __time_critical_func(audio_i2s_dma_irq_handler)()
//   defined at my_pico_audio_i2s/audio_i2s.c
//   where i2s_callback_func() is declared with __attribute__((weak))
void i2s_callback_func()
{
    decode_func_ary[cur_audio_codec]();
}