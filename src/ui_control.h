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

bool ui_get_btn_evt(button_action_t *btn_act);
void ui_clear_btn_evt();
bool ui_is_charging();
void ui_set_power_keep(bool value);
UIMode *getUIMode(ui_mode_enm_t ui_mode_enm);
void ui_init(uint32_t pin_power_keep, ui_mode_enm_t init_dest_ui_mode, stack_t *dir_stack, uint8_t fs_type);
ui_mode_enm_t ui_update();
ui_mode_enm_t ui_force_update(ui_mode_enm_t ui_mode_enm);
uint16_t ui_get_idle_count();

bool uiv_get_low_battery();
void uiv_set_file_idx(uint16_t idx_head, uint16_t idx_column);
void uiv_get_file_idx(uint16_t *idx_head, uint16_t *idx_column);
void uiv_set_play_idx(uint16_t idx_play);
void uiv_get_play_idx(uint16_t *idx_play);
void uiv_set_play_position(size_t fpos, uint32_t samples_played);
void uiv_get_play_position(size_t *fpos, uint32_t *samples_played);
void uiv_get_resume_ui_mode(ui_mode_enm_t *resume_ui_mode);

#ifdef __cplusplus
}
#endif

#endif // _UI_CONTROL_H_