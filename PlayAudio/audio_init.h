/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AUDIO_INIT_H_INCLUDED__
#define __AUDIO_INIT_H_INCLUDED__

#include "pico/audio_i2s.h"

#define SAMPLES_PER_BUFFER 1152 // Samples / channel

audio_buffer_pool_t *audio_init();

#endif // __AUDIO_INIT_H_INCLUDED__