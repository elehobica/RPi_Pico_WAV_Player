/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HARDWARE_GPIO_EX_H_
#define _HARDWARE_GPIO_EX_H_

#include "hardware/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// PICO_CONFIG: PARAM_ASSERTIONS_ENABLED_GPIO, Enable/disable assertions in the GPIO module, type=bool, default=0, group=hardware_gpio
#ifndef PARAM_ASSERTIONS_ENABLED_GPIO
#define PARAM_ASSERTIONS_ENABLED_GPIO 0
#endif

/** \file gpio_ex.h
 *  \defgroup hardware_gpio_ex hardware_gpio_ex
 *
 * General Purpose Input/Output (GPIO) Extra API
 *
 */

/*! \brief Set GPIO output drive strength
 *  \ingroup hardware_gpio_ex
 *
 * \param gpio GPIO number
 * \param value See \ref gpio_set_drive_strength
 */
void gpio_set_drive_strength(uint gpio, uint value);

/*! \brief Set GPIO input schmitt trigger
 *  \ingroup hardware_gpio_ex
 *
 * \param gpio GPIO number
 * \param value See \ref gpio_set_schmitt
 */
void gpio_set_schmitt(uint gpio, uint value);

/*! \brief Set GPIO output slew rate
 *  \ingroup hardware_gpio_ex
 *
 * \param gpio GPIO number
 * \param value See \ref gpio_set_slew_rate
 */
void gpio_set_slew_rate(uint gpio, uint value);

#endif // _HARDWARE_GPIO_EX_H_
