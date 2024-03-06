/*------------------------------------------------------/
/ Copyright (c) 2023, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#pragma once

#include "pico/audio_i2s.h"

static const int SAMPLES_PER_BUFFER = PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH; // Samples / channel
static const int32_t DAC_ZERO = 1; // to avoid pop noise caused by auto-mute function of DAC

typedef enum _i2s_samp_freq_t {
    SAMP_FREQ_NONE = 0,        /**< Not valid */
    SAMP_FREQ_44100 = 44100,   /**< 44.1 KHz */
    SAMP_FREQ_48000 = 48000,   /**< 48.0 KHz */
    SAMP_FREQ_88200 = 88200,   /**< 88.2 KHz */
    SAMP_FREQ_96000 = 96000,   /**< 96.0 KHz */
    SAMP_FREQ_176400 = 176400, /**< 176.4 KHz */
    SAMP_FREQ_192000 = 192000  /**< 192.0 KHz */
} i2s_samp_freq_t;

audio_buffer_pool_t* i2s_setup(i2s_samp_freq_t samp_freq);
audio_buffer_pool_t* i2s_audio_init(uint32_t sample_freq);
void i2s_audio_deinit();
