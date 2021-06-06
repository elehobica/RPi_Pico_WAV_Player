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
void ui_init(ui_mode_enm_t init_dest_ui_mode, stack_t *dir_stack, uint8_t fs_type);
ui_mode_enm_t ui_update();
ui_mode_enm_t ui_force_update(ui_mode_enm_t ui_mode_enm);
bool ui_get_btn_evt(button_action_t *btn_act);
void ui_clear_btn_evt();

void uiv_set_battery_voltage(uint16_t bat_mv, bool is_low);
void uiv_set_file_idx(uint16_t idx_head, uint16_t idx_column);
void uiv_get_file_idx(uint16_t *idx_head, uint16_t *idx_column);
void uiv_set_play_idx(uint16_t idx_play);
void uiv_get_play_idx(uint16_t *idx_play);

#ifdef __cplusplus
}
#endif

#endif // _UI_CONTROL_H_