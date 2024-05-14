/*------------------------------------------------------/
/ Copyright (c) 2020-2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#pragma once

#include "common.h"

enum class button_status_t : uint32_t {
    Open = 0,
    Center,
    D,
    Plus,
    Minus,
};

enum class button_unit_t : uint32_t {
    PushButtons = 0,
    HpButtons,
};

enum class button_action_t : uint32_t {
    CenterSingle = 0,
    CenterDouble,
    CenterTriple,
    CenterLong,
    CenterLongLong,
    PlusSingle,
    PlusLong,
    PlusFwd,
    MinusSingle,
    MinusLong,
    MinusRwd,
    Others,
};

enum class button_layout_t : uint32_t {
    Horizontal = 0,
    Vertical,
};

// using struct as an example, but primitive types can be used too
typedef struct element {
    button_action_t button_action;
    button_unit_t button_unit;
} element_t;

bool ui_get_btn_evt(button_action_t& btn_act, button_unit_t& btn_unit);
void ui_clear_btn_evt();

void ui_init(const board_type_t& board_type);
ui_mode_enm_t ui_update();
ui_mode_enm_t ui_force_update(const ui_mode_enm_t& ui_mode_enm);
uint16_t ui_get_idle_count();
uint32_t ui_set_center_switch_for_wakeup(const bool& flg);

board_type_t ui_get_board_type();
