/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef _ADC_UTIL_H_
#define _ADC_UTIL_H_

#include "pico/stdlib.h"
#include "pico/util/queue.h"

#define NUM_BTN_HISTORY 30

typedef enum _button_status_t {
    ButtonOpen = 0,
    ButtonCenter,
    ButtonD,
    ButtonPlus,
    ButtonMinus
} button_status_t;

typedef enum _button_action_t {
    ButtonCenterSingle = 0,
    ButtonCenterDouble,
    ButtonCenterTriple,
    ButtonCenterLong,
    ButtonCenterLongLong,
    ButtonPlusSingle,
    ButtonPlusLong,
    ButtonPlusFwd,
    ButtonMinusSingle,
    ButtonMinusLong,
    ButtonMinusRwd,
    ButtonOthers
} button_action_t;

// using struct as an example, but primitive types can be used too
typedef struct element {
    button_action_t button_action;
} element_t;

int adc_util_init(queue_t *btn_evt_queue);

#endif // _ADC_UTIL_H_