/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hardware/gpio.h"
#include "hardware/gpio_ex.h"
#include "hardware/sync.h"

#include "hardware/structs/iobank0.h"
#include "hardware/irq.h"

#include "pico/binary_info.h"

/// \tag::gpio_set_drive_strength[]
// Select drive strength for this GPIO
void gpio_set_drive_strength(uint gpio, uint value) {
    invalid_params_if(GPIO, gpio >= NUM_BANK0_GPIOS);
    invalid_params_if(GPIO, value << PADS_BANK0_GPIO0_DRIVE_LSB & ~PADS_BANK0_GPIO0_DRIVE_BITS);
    hw_write_masked(&padsbank0_hw->io[gpio],
                   value << PADS_BANK0_GPIO0_DRIVE_LSB,
                   PADS_BANK0_GPIO0_DRIVE_BITS
    );
}
/// \end::gpio_set_drive_strength[]

/// \tag::gpio_set_schmitt[]
// Select schmitt trigger for this GPIO
void gpio_set_schmitt(uint gpio, uint value) {
    invalid_params_if(GPIO, gpio >= NUM_BANK0_GPIOS);
    invalid_params_if(GPIO, value << PADS_BANK0_GPIO1_SCHMITT_LSB & ~PADS_BANK0_GPIO1_SCHMITT_BITS);
    hw_write_masked(&padsbank0_hw->io[gpio],
                   value << PADS_BANK0_GPIO1_SCHMITT_LSB,
                   PADS_BANK0_GPIO1_SCHMITT_BITS
    );
}
/// \end::gpio_set_schmitt[]

/// \tag::gpio_set_slew_rate[]
// Select slew rate for this GPIO
void gpio_set_slew_rate(uint gpio, uint value) {
    invalid_params_if(GPIO, gpio >= NUM_BANK0_GPIOS);
    invalid_params_if(GPIO, value << PADS_BANK0_GPIO0_SLEWFAST_LSB & ~PADS_BANK0_GPIO0_SLEWFAST_BITS);
    hw_write_masked(&padsbank0_hw->io[gpio],
                   value << PADS_BANK0_GPIO0_SLEWFAST_LSB,
                   PADS_BANK0_GPIO0_SLEWFAST_BITS
    );
}
/// \end::gpio_set_slew_rate[]
