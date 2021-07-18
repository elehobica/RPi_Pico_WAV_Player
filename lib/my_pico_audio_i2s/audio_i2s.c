/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Modified by Elehobica, 2021

#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "hardware/structs/dma.h"
#include "hardware/regs/dreq.h"

#include "audio_i2s.pio.h"
#include "pico/audio_i2s.h"

//#define CORE1_PROCESS_I2S_CALLBACK  // Multi-Core Processing Mode (Experimentally Single-Core seems better)
//#define WATCH_DMA_TRANSFER_INTERVAL // Activate only for analysis because of watch overhead
//#define WATCH_PIO_SM_TX_FIFO_LEVEL  // Activate only for analysis because of watch overhead

CU_REGISTER_DEBUG_PINS(audio_timing)

// ---- select at most one ---
//CU_SELECT_DEBUG_PINS(audio_timing)


#define audio_pio __CONCAT(pio, PICO_AUDIO_I2S_PIO)
#define GPIO_FUNC_PIOx __CONCAT(GPIO_FUNC_PIO, PICO_AUDIO_I2S_PIO)
#define DREQ_PIOx_TX0 __CONCAT(__CONCAT(DREQ_PIO, PICO_AUDIO_I2S_PIO), _TX0)

#define dma_intsx __CONCAT(dma_hw->ints, PICO_AUDIO_I2S_DMA_IRQ)
#define dma_channel_set_irqx_enabled __CONCAT(__CONCAT(dma_channel_set_irq, PICO_AUDIO_I2S_DMA_IRQ),_enabled)
#define DMA_IRQ_x __CONCAT(DMA_IRQ_, PICO_AUDIO_I2S_DMA_IRQ)

static uint loaded_offset = 0;
static const audio_format_t *_i2s_input_audio_format;
static const audio_format_t *_i2s_output_audio_format;
struct {
    audio_buffer_t *playing_buffer;
    uint32_t freq;
    uint8_t pio_sm;
    uint8_t dma_channel;
} shared_state;

audio_format_t pio_i2s_consumer_format;
audio_buffer_format_t pio_i2s_consumer_buffer_format = {
        .format = &pio_i2s_consumer_format,
};

static audio_buffer_pool_t *audio_i2s_consumer;
static audio_buffer_t silence_buffer;

static void __isr __time_critical_func(audio_i2s_dma_irq_handler)();

#ifdef WATCH_PIO_SM_TX_FIFO_LEVEL
static inline uint32_t _millis(void)
{
	return to_ms_since_boot(get_absolute_time());
}
#endif // WATCH_PIO_SM_TX_FIFO_LEVEL

#ifdef WATCH_DMA_TRANSFER_INTERVAL
static inline uint32_t _micros(void)
{
	return to_us_since_boot(get_absolute_time());
}
#endif // WATCH_DMA_TRANSFER_INTERVAL

// i2s callback function to be defined at external
__attribute__((weak))
void i2s_callback_func()
{
    /*
	uint32_t time = to_ms_since_boot(get_absolute_time());
    printf("i2s_callback_func %d\n", time);
    */
    return;
}

#ifdef CORE1_PROCESS_I2S_CALLBACK

enum FifoMessage {
    RESPONSE_CORE1_THREAD_STARTED = 0,
    RESPONSE_CORE1_THREAD_TERMINATED = 0,
    EVENT_I2S_DMA_TRANSFER_STARTED,
    NOTIFY_I2S_DISABLED
};

static const uint64_t FIFO_TIMEOUT = 10 * 1000; // us

void i2s_callback_loop()
{
    multicore_fifo_push_blocking(RESPONSE_CORE1_THREAD_STARTED);
#ifndef NDEBUG
    printf("i2s_callback_loop started (on core %d)\n", get_core_num());
#endif // NDEBUG
    while (true) {
        uint32_t msg = multicore_fifo_pop_blocking();
        if (msg == EVENT_I2S_DMA_TRANSFER_STARTED) {
            i2s_callback_func();
        } else if (msg == NOTIFY_I2S_DISABLED) {
            break;
        } else {
            panic("Unexpected message from Core 0\n");
        }
        tight_loop_contents();
    }
    multicore_fifo_push_blocking(RESPONSE_CORE1_THREAD_TERMINATED);
#ifndef NDEBUG
    printf("i2s_callback_loop terminated (on core %d)\n", get_core_num());
#endif // NDEBUG

    while (true) { tight_loop_contents(); } // infinite loop
    return;
}
#endif // CORE1_PROCESS_I2S_CALLBACK

//void audio_i2s_end(const audio_i2s_config_t *config) {
void audio_i2s_end() {
    audio_buffer_t *ab = shared_state.playing_buffer;
    queue_free_audio_buffer(audio_i2s_consumer, ab);
    free(silence_buffer.buffer);
    shared_state.playing_buffer = NULL;
    uint8_t sm = shared_state.pio_sm;
    pio_sm_drain_tx_fifo(audio_pio, sm);
    pio_sm_unclaim(audio_pio, sm);
    pio_remove_program(audio_pio, &audio_i2s_program, loaded_offset);
    pio_clear_instruction_memory(audio_pio);
    uint8_t dma_channel = shared_state.dma_channel;
    dma_channel_unclaim(dma_channel);
    irq_remove_handler(DMA_IRQ_x, audio_i2s_dma_irq_handler);
    dma_channel_set_irqx_enabled(dma_channel, 0);
}

const audio_format_t *audio_i2s_setup(const audio_format_t *i2s_input_audio_format, const audio_format_t *i2s_output_audio_format,
                                               const audio_i2s_config_t *config) {
    _i2s_input_audio_format = i2s_input_audio_format;
    _i2s_output_audio_format = i2s_output_audio_format;
    uint func = GPIO_FUNC_PIOx;
    gpio_set_function(config->data_pin, func);
    gpio_set_function(config->clock_pin_base, func);
    gpio_set_function(config->clock_pin_base + 1, func);

    uint8_t sm = shared_state.pio_sm = config->pio_sm;
    pio_sm_claim(audio_pio, sm);

    loaded_offset = pio_add_program(audio_pio, &audio_i2s_program);

    assert(_i2s_output_audio_format->channel_count == AUDIO_CHANNEL_STEREO);
    assert(_i2s_output_audio_format->pcm_format == AUDIO_PCM_FORMAT_S16 || _i2s_output_audio_format->pcm_format == AUDIO_PCM_FORMAT_S32);
    uint res_bits = (_i2s_output_audio_format->pcm_format == AUDIO_PCM_FORMAT_S32) ? 32 : 16;
    audio_i2s_program_init(audio_pio, sm, loaded_offset, config->data_pin, config->clock_pin_base, res_bits);

    silence_buffer.buffer = pico_buffer_alloc(PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH * 4);
    silence_buffer.sample_count = PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH;
    silence_buffer.format = &pio_i2s_consumer_buffer_format;

    __mem_fence_release();
    uint8_t dma_channel = config->dma_channel;
    dma_channel_claim(dma_channel);

    shared_state.dma_channel = dma_channel;

    dma_channel_config dma_config = dma_channel_get_default_config(dma_channel);

    channel_config_set_dreq(&dma_config,
                            DREQ_PIOx_TX0 + sm
    );
    enum dma_channel_transfer_size i2s_dma_configure_size;
    if (_i2s_output_audio_format->channel_count == AUDIO_CHANNEL_MONO) {
        switch (_i2s_output_audio_format->pcm_format) {
            case AUDIO_PCM_FORMAT_S8:
            case AUDIO_PCM_FORMAT_U8:
                i2s_dma_configure_size = DMA_SIZE_8;
                assert(false);
                break;
            case AUDIO_PCM_FORMAT_S16:
            case AUDIO_PCM_FORMAT_U16:
                i2s_dma_configure_size = DMA_SIZE_16;
                assert(false);
                break;
            case AUDIO_PCM_FORMAT_S32:
            case AUDIO_PCM_FORMAT_U32:
                i2s_dma_configure_size = DMA_SIZE_32;
                assert(false);
                break;
            default:
                assert(false);
                break;
        }
    } else {
        switch (_i2s_output_audio_format->pcm_format) {
            case AUDIO_PCM_FORMAT_S8:
            case AUDIO_PCM_FORMAT_U8:
                i2s_dma_configure_size = DMA_SIZE_16;
                break;
            case AUDIO_PCM_FORMAT_S16:
            case AUDIO_PCM_FORMAT_U16:
                i2s_dma_configure_size = DMA_SIZE_32;
                break;
            case AUDIO_PCM_FORMAT_S32:
            case AUDIO_PCM_FORMAT_U32:
                i2s_dma_configure_size = DMA_SIZE_32; // Need after-treatment because of no 64bit transfer
                break;
            default:
                assert(false);
                break;
        }

    }
    channel_config_set_transfer_data_size(&dma_config, i2s_dma_configure_size);
    dma_channel_configure(dma_channel,
                          &dma_config,
                          &audio_pio->txf[sm],  // dest
                          NULL, // src
                          0, // count
                          false // trigger
    );

    irq_add_shared_handler(DMA_IRQ_x, audio_i2s_dma_irq_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    dma_channel_set_irqx_enabled(dma_channel, 1);
    return _i2s_output_audio_format;
}

static void update_pio_frequency(uint32_t sample_freq, audio_pcm_format_t pcm_format, audio_channel_t channel_count) {
    printf("setting PIO freq for target sampling freq = %d Hz\n", (int) sample_freq);
    uint32_t system_clock_frequency = clock_get_hz(clk_sys);
    assert(system_clock_frequency < 0x40000000);
    //uint32_t divider = system_clock_frequency * 4 / sample_freq; // avoid arithmetic overflow
    uint32_t divider;
    uint32_t bits;
    switch (pcm_format) {
        case AUDIO_PCM_FORMAT_S8:
        case AUDIO_PCM_FORMAT_U8:
            divider = system_clock_frequency * 4 * channel_count / sample_freq;
            bits = 8;
            break;
        case AUDIO_PCM_FORMAT_S16:
        case AUDIO_PCM_FORMAT_U16:
            divider = system_clock_frequency * 2 * channel_count / sample_freq;
            bits = 16;
            break;
        case AUDIO_PCM_FORMAT_S32:
        case AUDIO_PCM_FORMAT_U32:
            divider = system_clock_frequency * 1 * channel_count / sample_freq;
            bits = 32;
            break;
        default:
            divider = system_clock_frequency * 2 * channel_count / sample_freq;
            bits = 16;
            assert(false);
            break;
    }
    assert(divider < 0x1000000);
#if 0 // PIO_CLK_DIV_FRAC
    float pio_freq = (float) system_clock_frequency * 256 / divider; // frac
    printf("System clock at %u Hz, I2S clock divider %d/256: PIO freq %7.4f Hz\n", (uint) system_clock_frequency, (uint) divider, pio_freq);
    pio_sm_set_clkdiv_int_frac(audio_pio, shared_state.pio_sm, divider >> 8u, divider & 0xffu); // This scheme includes clock Jitter
#else
    divider >>= 8u;
    float pio_freq = (float) system_clock_frequency / divider; // no frac
    float samp_freq = pio_freq / ((float) bits * 2.0 * 2.0);
    printf("System clock at %u Hz, I2S clock divider %d: PIO freq %7.4f Hz: sampling freq %7.4f Hz\n", (uint) system_clock_frequency, (uint) divider, pio_freq, samp_freq);
    pio_sm_set_clkdiv(audio_pio, shared_state.pio_sm, divider); // No Jitter. but clock freq accuracy depends on PIO source clock freq
#endif

    shared_state.freq = sample_freq;
}

static audio_buffer_t *wrap_consumer_take(audio_connection_t *connection, bool block) {
    // support dynamic frequency shifting
    if (connection->producer_pool->format->sample_freq != shared_state.freq) {
        update_pio_frequency(connection->producer_pool->format->sample_freq, connection->producer_pool->format->pcm_format, connection->producer_pool->format->channel_count);
    }
    if (_i2s_input_audio_format->pcm_format == _i2s_output_audio_format->pcm_format) {
        if (_i2s_input_audio_format->channel_count == AUDIO_CHANNEL_MONO && _i2s_input_audio_format->channel_count == AUDIO_CHANNEL_MONO) {
            return mono_to_mono_consumer_take(connection, block);
        } else if (_i2s_input_audio_format->channel_count == AUDIO_CHANNEL_MONO && _i2s_input_audio_format->channel_count == AUDIO_CHANNEL_STEREO) {
            return mono_to_stereo_consumer_take(connection, block);
        } else if (_i2s_input_audio_format->channel_count == AUDIO_CHANNEL_STEREO && _i2s_input_audio_format->channel_count == AUDIO_CHANNEL_STEREO) {
            switch (_i2s_input_audio_format->pcm_format) {
                case AUDIO_PCM_FORMAT_S16:
                    return stereo_s16_to_stereo_s16_consumer_take(connection, block);
                    break;
                case AUDIO_PCM_FORMAT_S32:
                    return stereo_s32_to_stereo_s32_consumer_take(connection, block);
                    break;
                default:
                assert(false);
            }
        } else {
            assert(false); // unsupported
        }
    } else {
        assert(false); // unsupported
    }
}

static void wrap_producer_give(audio_connection_t *connection, audio_buffer_t *buffer) {
    // support dynamic frequency shifting
    if (connection->producer_pool->format->sample_freq != shared_state.freq) {
        update_pio_frequency(connection->producer_pool->format->sample_freq, connection->producer_pool->format->pcm_format, connection->producer_pool->format->channel_count);
    }
    if (_i2s_input_audio_format->pcm_format == _i2s_output_audio_format->pcm_format) {
        if (_i2s_input_audio_format->channel_count == AUDIO_CHANNEL_MONO && _i2s_input_audio_format->channel_count == AUDIO_CHANNEL_MONO) {
            assert(false);
            //return mono_to_mono_producer_give(connection, block);
        } else if (_i2s_input_audio_format->channel_count == AUDIO_CHANNEL_MONO && _i2s_input_audio_format->channel_count == AUDIO_CHANNEL_STEREO) {
            assert(false);
            //return mono_to_stereo_producer_give(connection, buffer);
        } else if (_i2s_input_audio_format->channel_count == AUDIO_CHANNEL_STEREO && _i2s_input_audio_format->channel_count == AUDIO_CHANNEL_STEREO) {
            switch (_i2s_input_audio_format->pcm_format) {
                case AUDIO_PCM_FORMAT_S16:
                    return stereo_s16_to_stereo_s16_producer_give(connection, buffer);
                    break;
                case AUDIO_PCM_FORMAT_S32:
                    return stereo_s32_to_stereo_s32_producer_give(connection, buffer);
                    break;
                default:
                assert(false);
            }
        } else {
            assert(false); // unsupported
        }
    } else {
        assert(false); // unsupported
    }
}

static struct buffer_copying_on_consumer_take_connection m2s_audio_i2s_ct_connection = {
        .core = {
                .consumer_pool_take = wrap_consumer_take,
                .consumer_pool_give = consumer_pool_give_buffer_default,
                .producer_pool_take = producer_pool_take_buffer_default,
                .producer_pool_give = producer_pool_give_buffer_default,
        }
};

static struct producer_pool_blocking_give_connection m2s_audio_i2s_pg_connection = {
        .core = {
                .consumer_pool_take = consumer_pool_take_buffer_default,
                .consumer_pool_give = consumer_pool_give_buffer_default,
                .producer_pool_take = producer_pool_take_buffer_default,
                .producer_pool_give = wrap_producer_give,
        }
};

bool audio_i2s_connect_thru(audio_buffer_pool_t *producer, audio_connection_t *connection) {
    return audio_i2s_connect_extra(producer, false, 2, 256, connection);
}

bool audio_i2s_connect(audio_buffer_pool_t *producer) {
    return audio_i2s_connect_thru(producer, NULL);
}

bool audio_i2s_connect_extra(audio_buffer_pool_t *producer, bool buffer_on_give, uint buffer_count,
                                 uint samples_per_buffer, audio_connection_t *connection) {
    printf("Connecting PIO I2S audio\n");

    // todo we need to pick a connection based on the frequency - e.g. 22050 can be more simply upsampled to 44100
    assert(producer->format->pcm_format == AUDIO_PCM_FORMAT_S16 || producer->format->pcm_format == AUDIO_PCM_FORMAT_S32);
    pio_i2s_consumer_format.pcm_format = _i2s_output_audio_format->pcm_format;
    // todo we could do mono
    // todo we can't match exact, so we should return what we can do
    pio_i2s_consumer_format.sample_freq = producer->format->sample_freq;
    pio_i2s_consumer_format.channel_count = _i2s_output_audio_format->channel_count;
    switch (_i2s_output_audio_format->pcm_format) {
        case AUDIO_PCM_FORMAT_S8:
        case AUDIO_PCM_FORMAT_U8:
            pio_i2s_consumer_buffer_format.sample_stride = 1 * pio_i2s_consumer_format.channel_count;
            break;
        case AUDIO_PCM_FORMAT_S16:
        case AUDIO_PCM_FORMAT_U16:
            pio_i2s_consumer_buffer_format.sample_stride = 2 * pio_i2s_consumer_format.channel_count;
            break;
        case AUDIO_PCM_FORMAT_S32:
        case AUDIO_PCM_FORMAT_U32:
            pio_i2s_consumer_buffer_format.sample_stride = 4 * pio_i2s_consumer_format.channel_count;
            break;
        default:
            assert(false);
            break;
    }

    audio_i2s_consumer = audio_new_consumer_pool(&pio_i2s_consumer_buffer_format, buffer_count, samples_per_buffer);

    update_pio_frequency(producer->format->sample_freq, producer->format->pcm_format, producer->format->channel_count);

    // todo cleanup threading
    __mem_fence_release();

    if (!connection) {
        if (producer->format->channel_count == AUDIO_CHANNEL_STEREO) {
            if (_i2s_input_audio_format->channel_count == AUDIO_CHANNEL_MONO) {
                panic("need to merge channels down\n");
            } else if (_i2s_output_audio_format->channel_count == AUDIO_CHANNEL_MONO) {
                panic("trying to play stereo thru mono not yet supported");
            } else {
                printf("Copying stereo to stereo at %d Hz\n", (int) producer->format->sample_freq);
            }
            // todo we should support pass thru option anyway
            //printf("TODO... not completing stereo audio connection properly!\n");
        } else {
            if (_i2s_output_audio_format->channel_count == AUDIO_CHANNEL_MONO) {
                printf("Copying mono to mono at %d Hz\n", (int) producer->format->sample_freq);
            } else {
                printf("Converting mono to stereo at %d Hz\n", (int) producer->format->sample_freq);
            }
        }
        connection = buffer_on_give ? &m2s_audio_i2s_pg_connection.core : &m2s_audio_i2s_ct_connection.core;
    }
    audio_complete_connection(connection, producer, audio_i2s_consumer);
    return true;
}

static struct buffer_copying_on_consumer_take_connection m2s_audio_i2s_connection_s8_mono = {
        .core = {
                .consumer_pool_take = mono_s8_to_mono_consumer_take,
                .consumer_pool_give = consumer_pool_give_buffer_default,
                .producer_pool_take = producer_pool_take_buffer_default,
                .producer_pool_give = producer_pool_give_buffer_default,
        }
};

static struct buffer_copying_on_consumer_take_connection m2s_audio_i2s_connection_s8_stereo = {
        .core = {
                .consumer_pool_take = mono_s8_to_stereo_consumer_take,
                .consumer_pool_give = consumer_pool_give_buffer_default,
                .producer_pool_take = producer_pool_take_buffer_default,
                .producer_pool_give = producer_pool_give_buffer_default,
        }
};

bool audio_i2s_connect_s8(audio_buffer_pool_t *producer) {
    printf("Connecting PIO I2S audio (U8)\n");

    // todo we need to pick a connection based on the frequency - e.g. 22050 can be more simply upsampled to 44100
    assert(producer->format->pcm_format == AUDIO_PCM_FORMAT_S8);
    pio_i2s_consumer_format.pcm_format = AUDIO_PCM_FORMAT_S16;
    // todo we could do mono
    // todo we can't match exact, so we should return what we can do
    pio_i2s_consumer_format.sample_freq = producer->format->sample_freq;
    pio_i2s_consumer_format.channel_count = _i2s_output_audio_format->channel_count;
    switch (_i2s_output_audio_format->pcm_format) {
        case AUDIO_PCM_FORMAT_S8:
        case AUDIO_PCM_FORMAT_U8:
            pio_i2s_consumer_buffer_format.sample_stride = 1 * pio_i2s_consumer_format.channel_count;
            break;
        case AUDIO_PCM_FORMAT_S16:
        case AUDIO_PCM_FORMAT_U16:
            pio_i2s_consumer_buffer_format.sample_stride = 2 * pio_i2s_consumer_format.channel_count;
            break;
        case AUDIO_PCM_FORMAT_S32:
        case AUDIO_PCM_FORMAT_U32:
            pio_i2s_consumer_buffer_format.sample_stride = 4 * pio_i2s_consumer_format.channel_count;
            break;
        default:
            assert(false);
            break;
    }

    // we do this on take so should do it quickly...
    uint samples_per_buffer = 256;
    // todo with take we really only need 1 buffer
    audio_i2s_consumer = audio_new_consumer_pool(&pio_i2s_consumer_buffer_format, 2, samples_per_buffer);

    // todo we need a method to calculate this in clocks
    uint32_t system_clock_frequency = 48000000;
    //uint32_t divider = system_clock_frequency * 4 / producer->format->sample_freq; // avoid arithmetic overflow
    //uint32_t divider = system_clock_frequency * 256 / producer->format->sample_freq * 16 * 4;
    uint32_t divider;
    switch (producer->format->pcm_format) {
        case AUDIO_PCM_FORMAT_S8:
        case AUDIO_PCM_FORMAT_U8:
            divider = system_clock_frequency * 4 * producer->format->channel_count * 2 / producer->format->sample_freq;
            break;
        case AUDIO_PCM_FORMAT_S16:
        case AUDIO_PCM_FORMAT_U16:
            divider = system_clock_frequency * 2 * producer->format->channel_count * 2 / producer->format->sample_freq;
            break;
        case AUDIO_PCM_FORMAT_S32:
        case AUDIO_PCM_FORMAT_U32:
            divider = system_clock_frequency * 1 * producer->format->channel_count * 2 / producer->format->sample_freq;
            break;
        default:
            divider = system_clock_frequency * 2 * producer->format->channel_count * 2 / producer->format->sample_freq;
            assert(false);
            break;
    }
    pio_sm_set_clkdiv_int_frac(audio_pio, shared_state.pio_sm, divider >> 8u, divider & 0xffu);

    // todo cleanup threading
    __mem_fence_release();

    audio_connection_t *connection;
    if (producer->format->channel_count == AUDIO_CHANNEL_STEREO) {
        if (_i2s_output_audio_format->channel_count == AUDIO_CHANNEL_MONO) {
            panic("trying to play stereo thru mono not yet supported");
        }
        // todo we should support pass thru option anyway
        printf("TODO... not completing stereo audio connection properly!\n");
        connection = &m2s_audio_i2s_connection_s8_stereo.core;
    } else {
        if (_i2s_output_audio_format->channel_count == AUDIO_CHANNEL_MONO) {
            printf("Copying mono to mono at %d Hz\n", (int) producer->format->sample_freq);
            connection = &m2s_audio_i2s_connection_s8_mono.core;
        } else {
            printf("Converting mono to stereo at %d Hz\n", (int) producer->format->sample_freq);
            connection = &m2s_audio_i2s_connection_s8_stereo.core;
        }
    }
    audio_complete_connection(connection, producer, audio_i2s_consumer);
    return true;
}

static inline void audio_start_dma_transfer() {
    assert(!shared_state.playing_buffer);

    #ifdef WATCH_DMA_TRANSFER_INTERVAL
    static uint32_t latest = 0;
    static uint32_t max_interval = 0;
    uint32_t now = _micros();
    uint32_t interval = now - latest;
    if (latest != 0 && max_interval < interval) {
        printf("dma_transfer interval %d\n", interval);
        max_interval = interval;
    }
    latest = now;
    #endif // WATCH_DMA_TRANSFER_INTERVAL
    #ifdef WATCH_PIO_SM_TX_FIFO_LEVEL
    uint tx_fifo_level = pio_sm_get_tx_fifo_level(audio_pio, shared_state.pio_sm);
    if (tx_fifo_level < 4) {
        printf("PIO TX FIFO too low: %d at %d ms\n", (int) tx_fifo_level, (int) _millis());
    }
    #endif // WATCH_PIO_SM_TX_FIFO_LEVEL

    audio_buffer_t *ab = take_audio_buffer(audio_i2s_consumer, false);

    shared_state.playing_buffer = ab;
    if (!ab) {
        DEBUG_PINS_XOR(audio_timing, 1);
        DEBUG_PINS_XOR(audio_timing, 2);
        DEBUG_PINS_XOR(audio_timing, 1);
        //DEBUG_PINS_XOR(audio_timing, 2);
        // just play some silence
        ab = &silence_buffer;
    }
    assert(ab->sample_count);
    // todo better naming of format->format->format!!
    assert(ab->format->format->pcm_format == AUDIO_PCM_FORMAT_S16 || ab->format->format->pcm_format == AUDIO_PCM_FORMAT_S32);
    if (_i2s_output_audio_format->channel_count == AUDIO_CHANNEL_MONO) {
        assert(ab->format->format->channel_count == AUDIO_CHANNEL_MONO);
        //assert(ab->format->sample_stride == 2);
    } else {
        assert(ab->format->format->channel_count == AUDIO_CHANNEL_STEREO);
        //assert(ab->format->sample_stride == 4);
    }
    if (ab->format->format->pcm_format == AUDIO_PCM_FORMAT_S32 && ab->format->format->channel_count == AUDIO_CHANNEL_STEREO) {
        dma_channel_transfer_from_buffer_now(shared_state.dma_channel, ab->buffer->bytes, ab->sample_count*2); // DMA_SIZE_32 * 2 times;
    } else {
        dma_channel_transfer_from_buffer_now(shared_state.dma_channel, ab->buffer->bytes, ab->sample_count);
    }
}

// irq handler for DMA
void __isr __time_critical_func(audio_i2s_dma_irq_handler)() {
#if PICO_AUDIO_I2S_NOOP
    assert(false);
#else
    uint dma_channel = shared_state.dma_channel;
    if (dma_intsx & (1u << dma_channel)) {
        dma_intsx = 1u << dma_channel;
        DEBUG_PINS_SET(audio_timing, 4);
        // free the buffer we just finished
        if (shared_state.playing_buffer) {
            give_audio_buffer(audio_i2s_consumer, shared_state.playing_buffer);
#ifndef NDEBUG
            shared_state.playing_buffer = NULL;
#endif
        }
        audio_start_dma_transfer();
        DEBUG_PINS_CLR(audio_timing, 4);
#ifdef CORE1_PROCESS_I2S_CALLBACK
        bool flg = multicore_fifo_push_timeout_us(EVENT_I2S_DMA_TRANSFER_STARTED, FIFO_TIMEOUT);
        if (!flg) { printf("Core0 -> Core1 FIFO Full\n"); }
        #else
        i2s_callback_func();
#endif // CORE1_PROCESS_I2S_CALLBACK
    }
#endif
}

static bool audio_enabled;

void audio_i2s_set_enabled(bool enabled) {
    if (enabled != audio_enabled) {
#ifndef NDEBUG
        if (enabled) {
            printf("Enabling PIO I2S audio (on core %d)\n", get_core_num());
        } else {
            printf("Disabling PIO I2S audio (on core %d)\n", get_core_num());
        }
#endif
        if (enabled) { // Clear pending before enabled
            uint dma_channel = shared_state.dma_channel;
            dma_intsx = 1u << dma_channel;
        }
        irq_set_enabled(DMA_IRQ_x, enabled);
        if (enabled) {
            audio_start_dma_transfer();
        }
#ifdef CORE1_PROCESS_I2S_CALLBACK
        bool flg;
        uint32_t msg;
        if (enabled) {
            multicore_reset_core1();
            multicore_launch_core1(i2s_callback_loop);
            flg = multicore_fifo_pop_timeout_us(FIFO_TIMEOUT, &msg);
            if (!flg || msg != RESPONSE_CORE1_THREAD_STARTED) {
                panic("Core1 is not respond\n");
            }
        } else {
            flg = multicore_fifo_push_timeout_us(NOTIFY_I2S_DISABLED, FIFO_TIMEOUT);
            if (!flg) { printf("Core0 -> Core1 FIFO Full\n"); }
            flg = multicore_fifo_pop_timeout_us(FIFO_TIMEOUT, &msg);
            if (!flg || msg != RESPONSE_CORE1_THREAD_TERMINATED) {
                panic("Core1 is not respond\n");
            }
        }
#endif // CORE1_PROCESS_I2S_CALLBACK

        pio_sm_set_enabled(audio_pio, shared_state.pio_sm, enabled);

        audio_enabled = enabled;
    }
}
