/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef _POWER_MANAGE_H_
#define _POWER_MANAGE_H_

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

void pm_backlight_init(uint32_t bl_val);
void pm_backlight_update();
void pm_init();
void pm_set_audio_dac_enable(bool value);
void pm_monitor_battery_voltage();
bool pm_is_charging();
void pm_set_power_keep(bool value);
bool pm_get_low_battery();
uint16_t pm_get_battery_voltage();

#ifdef __cplusplus
}
#endif

#endif // _POWER_MANAGE_H_