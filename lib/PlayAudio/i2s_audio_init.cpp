/*------------------------------------------------------/
/ Copyright (c) 2023, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include "i2s_audio_init.h"

#include <cstdio>

#include "pico/stdlib.h"

static audio_buffer_pool_t* _producer_pool = nullptr;

static audio_format_t audio_format = {
    .sample_freq = 44100,
    .pcm_format = AUDIO_PCM_FORMAT_S32,
    .channel_count = AUDIO_CHANNEL_STEREO
};

static audio_buffer_format_t producer_format = {
    .format = &audio_format,
    .sample_stride = 8
};

static audio_i2s_config_t i2s_config = {
    .data_pin = PICO_AUDIO_I2S_DATA_PIN,
    .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
    .dma_channel0 = 0,
    .dma_channel1 = 1,
    .pio_sm = 0
};

void i2s_setup(uint32_t samp_freq, audio_buffer_pool_t*& ap)
{
    printf("Samp Freq = %d Hz\n", static_cast<int>(samp_freq));
    if (_producer_pool != nullptr) {
        ap = nullptr;
        i2s_audio_deinit(); // less gap noise if deinit() is done when input is stable
    }
    i2s_audio_init(samp_freq);
    ap = _producer_pool;
}

void i2s_audio_init(uint32_t sample_freq)
{
    audio_format.sample_freq = sample_freq;

    _producer_pool = audio_new_producer_pool(&producer_format, 3, SAMPLES_PER_BUFFER);

    bool __unused ok;
    const audio_format_t *output_format;

    output_format = audio_i2s_setup(&audio_format, &audio_format, &i2s_config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(_producer_pool);
    assert(ok);
    { // initial buffer data
        audio_buffer_t* ab = take_audio_buffer(_producer_pool, true);
        int32_t* samples = (int32_t*) ab->buffer->bytes;
        for (uint i = 0; i < ab->max_sample_count; i++) {
            samples[i*2+0] = DAC_ZERO;
            samples[i*2+1] = DAC_ZERO;
        }
        ab->sample_count = ab->max_sample_count;
        give_audio_buffer(_producer_pool, ab);
    }
    audio_i2s_set_enabled(true);
}

void i2s_audio_deinit()
{
    audio_i2s_set_enabled(false);
    audio_i2s_end();

    audio_buffer_t* ab;
    ab = take_audio_buffer(_producer_pool, false);
    while (ab != nullptr) {
        free(ab->buffer->bytes);
        free(ab->buffer);
        ab = take_audio_buffer(_producer_pool, false);
    }
    ab = get_free_audio_buffer(_producer_pool, false);
    while (ab != nullptr) {
        free(ab->buffer->bytes);
        free(ab->buffer);
        ab = get_free_audio_buffer(_producer_pool, false);
    }
    ab = get_full_audio_buffer(_producer_pool, false);
    while (ab != nullptr) {
        free(ab->buffer->bytes);
        free(ab->buffer);
        ab = get_full_audio_buffer(_producer_pool, false);
    }
    free(_producer_pool);
    _producer_pool = nullptr;
}
