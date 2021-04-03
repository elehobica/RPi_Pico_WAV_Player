/*------------------------------------------------------/
/ Copyright (c) 2020-2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef _UI_CONTROL_H_
#define _UI_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pico/stdlib.h"
#include "stack.h"
#include "LcdCanvas.h"
#include "UIMode.h"

#define NUM_BTN_HISTORY 30

typedef enum _button_status_t {
    ButtonOpen = 0,
    ButtonCenter,
    ButtonD,
    ButtonPlus,
    ButtonMinus
} button_status_t;

// using struct as an example, but primitive types can be used too
typedef struct element {
    button_action_t button_action;
} element_t;

UIMode *getUIMode(ui_mode_enm_t ui_mode_enm);
void ui_init(ui_mode_enm_t init_dest_ui_mode, stack_t *dir_stack, LcdCanvas *lcd, uint8_t fs_type);
void ui_update();
void ui_force_update(ui_mode_enm_t ui_mode_enm);
bool ui_get_btn_evt(button_action_t *btn_act);
void ui_clear_btn_evt();

#ifdef __cplusplus
}
#endif

#endif // _UI_CONTROL_H_