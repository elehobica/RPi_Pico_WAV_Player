/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include "power_manage.h"
#include <cstdio>
#include "pico/stdlib.h"
#include "pico/sleep.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "st7735_80x160/lcd.h"
#include "ui_control.h"
#include "ConfigParam.h"
#include "ConfigMenu.h"

static bool low_power_mode = false;
static repeating_timer_t timer;
// ADC Timer frequency
const int TIMER_BATTERY_CHECK_HZ = 20;

static uint16_t _bat_mv = 4200;
//#define NO_BATTERY_VOLTAGE_CHECK

// DC/DC mode selection Pin
static const uint32_t PIN_DCDC_PSM_CTRL = 23;

// USB Charge detect Pin
static const uint32_t PIN_CHARGE_DETECT = 24;

// Power Keep Pin
static const uint32_t PIN_POWER_KEEP = 19;

// Battery Voltage Pin (GPIO28: ADC2)
static const uint32_t PIN_BATT_LVL = 28;
static const uint32_t ADC_PIN_BATT_LVL = 2;

// Battery Check Pin
static const uint32_t PIN_BATT_CHECK = 8;

// Audio DAC Enable Pin
static const uint32_t PIN_AUDIO_DAC_ENABLE = 27;

// Battery monitor interval
const int BATT_CHECK_INTERVAL_SEC = 5;

static bool timer_callback_battery_check(repeating_timer_t *rt) {
    pm_monitor_battery_voltage();
    return true; // keep repeating
}

static int timer_init_battery_check()
{
    // negative timeout means exact delay (rather than delay between callbacks)
    if (!add_repeating_timer_us(-1000000 / TIMER_BATTERY_CHECK_HZ, timer_callback_battery_check, nullptr, &timer)) {
        //printf("Failed to add timer\n");
        return 0;
    }

    return 1;
}

void pm_backlight_init(uint32_t bl_val)
{
    // BackLight PWM (125MHz / 65536 / 4 = 476.84 Hz)
    gpio_set_function(PIN_LCD_BLK, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.f);
    pwm_init(slice_num, &config, true);
    // Square bl_val to make brightness appear more linear
    pwm_set_gpio_level(PIN_LCD_BLK, bl_val * bl_val);
}

void pm_backlight_update()
{
    const int LoopCycleMs = UIMode::UpdateCycleMs; // loop cycle (50 ms)
    const int OneSec = 1000 / LoopCycleMs;
    uint32_t bl_val;
    if (ui_get_idle_count() < GET_CFG_MENU_DISPLAY_TIME_TO_BACKLIGHT_LOW*OneSec) {
        bl_val = GET_CFG_MENU_DISPLAY_BACKLIGHT_HIGH_LEVEL;
    } else {
        bl_val = GET_CFG_MENU_DISPLAY_BACKLIGHT_LOW_LEVEL;
    }
    pwm_set_gpio_level(PIN_LCD_BLK, bl_val * bl_val);
}

void pm_init()
{
    // USB Chard detect Pin (Input)
    gpio_set_dir(PIN_CHARGE_DETECT, GPIO_IN);

    // Power Keep Pin (Output)
    gpio_init(PIN_POWER_KEEP);
    gpio_set_dir(PIN_POWER_KEEP, GPIO_OUT);

    // Audio DAC Disable (Mute On)
    gpio_init(PIN_AUDIO_DAC_ENABLE);
    gpio_set_dir(PIN_AUDIO_DAC_ENABLE, GPIO_OUT);
    gpio_put(PIN_AUDIO_DAC_ENABLE, 0);

    // Battery Check Enable Pin (Output)
    gpio_init(PIN_BATT_CHECK);
    gpio_set_dir(PIN_BATT_CHECK, GPIO_OUT);
    gpio_put(PIN_BATT_CHECK, 0);

    // Battery Level Input (ADC)
    adc_gpio_init(PIN_BATT_LVL);

    // DCDC PSM control
    // 0: PFM mode (best efficiency)
    // 1: PWM mode (improved ripple)
    gpio_init(PIN_DCDC_PSM_CTRL);
    gpio_set_dir(PIN_DCDC_PSM_CTRL, GPIO_OUT);
    gpio_put(PIN_DCDC_PSM_CTRL, 1); // PWM mode for less Audio noise

    // BackLight
    pm_backlight_init(192);

    // Battery Check Timer start
    timer_init_battery_check();
}

void pm_set_audio_dac_enable(bool value)
{
    gpio_put(PIN_AUDIO_DAC_ENABLE, value);
}

void pm_monitor_battery_voltage()
{
    static int count = 0;
    if (count % (TIMER_BATTERY_CHECK_HZ*BATT_CHECK_INTERVAL_SEC) == TIMER_BATTERY_CHECK_HZ*BATT_CHECK_INTERVAL_SEC-2) {
        // Prepare to check battery voltage
        gpio_put(PIN_BATT_CHECK, 1);
    } else if (count % (TIMER_BATTERY_CHECK_HZ*BATT_CHECK_INTERVAL_SEC) == TIMER_BATTERY_CHECK_HZ*BATT_CHECK_INTERVAL_SEC-1) {
        // ADC Calibration Coefficients
        // ADC2 pin is connected to middle point of voltage divider 1.0Kohm + 3.3Kohm
        const int16_t coef_a = 4280;
        const int16_t coef_b = -20;
        adc_select_input(ADC_PIN_BATT_LVL);
        uint16_t result = adc_read();
        gpio_put(PIN_BATT_CHECK, 0);
        int16_t voltage = result * coef_a / (1<<12) + coef_b;
        //printf("Battery Voltage = %d (mV)\n", voltage);
        _bat_mv = voltage;
    }
    count++;
}

bool pm_is_charging()
{
    return gpio_get(PIN_CHARGE_DETECT);
}

void pm_set_power_keep(bool value)
{
    gpio_put(PIN_POWER_KEEP, value);
}

bool pm_get_low_battery()
{
#ifdef NO_BATTERY_VOLTAGE_CHECK
    return false;
#else
    return (_bat_mv < 2900);
#endif
}

uint16_t pm_get_battery_voltage()
{
    return _bat_mv;
}

void pm_set_low_power_mode(bool flag)
{
    if (flag && !low_power_mode) {
        pwm_set_gpio_level(PIN_LCD_BLK, 0);
        gpio_put(PIN_DCDC_PSM_CTRL, 0); // PFM mode for better efficiency
    } else if (!flag && low_power_mode) {
        uint32_t bl_val = GET_CFG_MENU_DISPLAY_BACKLIGHT_LOW_LEVEL;
        pwm_set_gpio_level(PIN_LCD_BLK, bl_val * bl_val);
        gpio_put(PIN_DCDC_PSM_CTRL, 1); // PWM mode for less Audio noise
    }
    low_power_mode = flag;
}