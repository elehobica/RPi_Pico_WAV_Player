/*------------------------------------------------------/
/ Copyright (c) 2023, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#pragma once

#include "pico/audio_i2s.h"

static constexpr int SAMPLES_PER_BUFFER = PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH; // Samples / channel
static constexpr int32_t DAC_ZERO = 1; // to avoid pop noise caused by auto-mute function of DAC

void i2s_setup(uint32_t samp_freq, audio_buffer_pool_t*& ap);
void i2s_audio_init(uint32_t sample_freq);
void i2s_audio_deinit();
