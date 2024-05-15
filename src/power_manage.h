/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#pragma once

#include <cstdint>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

void pm_backlight_update();
void pm_init(board_type_t board_type);
bool pm_get_active_battery_check();
void pm_set_audio_dac_enable(bool flag);
void pm_monitor_battery_voltage();
bool pm_usb_power_detected();
void pm_set_power_keep(bool flag);
void pm_enable_button_control(bool flag);
bool pm_get_low_battery();
float pm_get_battery_voltage();
void pm_enter_dormant_and_wake();
void pw_set_pll_usb_96MHz();
void pm_reboot();
bool pm_is_caused_reboot();

#ifdef __cplusplus
}
#endif
