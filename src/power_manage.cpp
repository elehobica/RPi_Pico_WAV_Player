/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/
/* Except for 'recover_from_sleep' part, see comment for copyright */

#include "power_manage.h"
#include <cstdio>
#include "pico/stdlib.h"
#include "pico/sleep.h"
#include "pico/stdio_uart.h"
#include "pico/stdio_usb.h" // use lib/my_pico_stdio_usb/
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/structs/scb.h"
#include "hardware/rosc.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"
#include "st7735_80x160/lcd.h"
#include "ui_control.h"
#include "ConfigParam.h"
#include "ConfigMenu.h"

//#define USE_ACTIVE_BATTERY_CHECK // Additional circuit needed

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

#ifdef USE_ACTIVE_BATTERY_CHECK
// Battery Check Pin
static const uint32_t PIN_BATT_CHECK = 8;
// Battery Voltage Pin (GPIO28: ADC2)
static const uint32_t PIN_BATT_LVL = 28;
static const uint32_t ADC_PIN_BATT_LVL = 2;
#else // USE_ACTIVE_BATTERY_CHECK
// Battery Voltage Pin (GPIO29: ADC3) (Raspberry Pi Pico built-in circuit)
static const uint32_t PIN_BATT_LVL = 29;
static const uint32_t ADC_PIN_BATT_LVL = 3;
#endif // USE_ACTIVE_BATTERY_CHECK

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
    uint32_t slice_num = pwm_gpio_to_slice_num(PIN_LCD_BLK);
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

#ifdef USE_ACTIVE_BATTERY_CHECK
    // Battery Check Enable Pin (Output)
    gpio_init(PIN_BATT_CHECK);
    gpio_set_dir(PIN_BATT_CHECK, GPIO_OUT);
    gpio_put(PIN_BATT_CHECK, 0);
#endif // USE_ACTIVE_BATTERY_CHECK

    // Battery Level Input (ADC)
    adc_gpio_init(PIN_BATT_LVL);

    // DCDC PSM control
    // 0: PFM mode (best efficiency)
    // 1: PWM mode (improved ripple)
    gpio_init(PIN_DCDC_PSM_CTRL);
    gpio_set_dir(PIN_DCDC_PSM_CTRL, GPIO_OUT);
    gpio_put(PIN_DCDC_PSM_CTRL, 1); // PWM mode for less Audio noise

    // BackLight
    pm_backlight_init(GET_CFG_MENU_DISPLAY_BACKLIGHT_HIGH_LEVEL);

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
#ifdef USE_ACTIVE_BATTERY_CHECK
    if (count % (TIMER_BATTERY_CHECK_HZ*BATT_CHECK_INTERVAL_SEC) == TIMER_BATTERY_CHECK_HZ*BATT_CHECK_INTERVAL_SEC-2) {
        // Prepare to check battery voltage
        gpio_put(PIN_BATT_CHECK, 1);
    } else
#endif // USE_ACTIVE_BATTERY_CHECK
    if (count % (TIMER_BATTERY_CHECK_HZ*BATT_CHECK_INTERVAL_SEC) == TIMER_BATTERY_CHECK_HZ*BATT_CHECK_INTERVAL_SEC-1) {
        // ADC Calibration Coefficients
#ifdef USE_ACTIVE_BATTERY_CHECK
        // ADC2 pin is connected to middle point of voltage divider 1.0Kohm + 3.3Kohm
        const int16_t coef_a = 4280;
        const int16_t coef_b = -20;
#else // USE_ACTIVE_BATTERY_CHECK
        // ADC3 pin is connected to middle point of voltage divider 200Kohm + 100Kohm
        const int16_t coef_a = 8860;
        const int16_t coef_b = -20;
#endif // USE_ACTIVE_BATTERY_CHECK
        adc_select_input(ADC_PIN_BATT_LVL);
        uint16_t result = adc_read();
#ifdef USE_ACTIVE_BATTERY_CHECK
        gpio_put(PIN_BATT_CHECK, 0);
#endif // USE_ACTIVE_BATTERY_CHECK
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

// === 'recover_from_sleep' part (start) ===================================
// great reference from 'recover_from_sleep'
// https://github.com/ghubcoder/PicoSleepDemo | https://ghubcoder.github.io/posts/awaking-the-pico/
static void _clock_preserve_and_recovery(bool doPreserve)
{
    static int count = 0;
    static uint32_t scr;
    static uint32_t sleep_en0;
    static uint32_t sleep_en1;

    if (doPreserve && count == 0) { // preserve (push)
        scr = scb_hw->scr;
        sleep_en0 = clocks_hw->sleep_en0;
        sleep_en1 = clocks_hw->sleep_en1;
        count++;
    } else if (!doPreserve && count == 1) { // recover (pop)
        scb_hw->scr = scr;
        clocks_hw->sleep_en0 = sleep_en0;
        clocks_hw->sleep_en1 = sleep_en1;
        count--;
    } else {
        panic("_clock_preserve_and_recovery count error!\n");
    }
}

static void _preserve_clock_before_sleep()
{
    _clock_preserve_and_recovery(true); // preserve
}

static void _recover_clock_after_sleep()
{
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS); //Re-enable ring Oscillator control
    _clock_preserve_and_recovery(false); // recover
    clocks_init(); // reset clocks
}
// === 'recover_from_sleep' part (end) ===================================

void pm_enter_dormant_and_wake()
{
    // === [1] Preparation for dormant ===
    gpio_init(PIN_LCD_BLK);
    gpio_set_dir(PIN_LCD_BLK, GPIO_OUT);
    gpio_put(PIN_LCD_BLK, 0);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    gpio_put(PIN_DCDC_PSM_CTRL, 0); // PFM mode for better efficiency
    stdio_usb_deinit(); // terminate usb cdc

    // === [2] goto dormant then wake up ===
    uint32_t ints = save_and_disable_interrupts(); // (+a)
    uint32_t pin = ui_set_center_switch_for_wakeup(true); // set ADC pin to wake up gpio pin (+b)
    _preserve_clock_before_sleep(); // (+c)
    //--
    sleep_run_from_xosc();
    sleep_goto_dormant_until_pin(pin, true, false); // dormant until fall edge detected
    //--
    _recover_clock_after_sleep(); // (-c)
    ui_set_center_switch_for_wakeup(false); // wake up gpio pin recover to ADC pin function (-b)
    restore_interrupts(ints); // (-a)

    // === [3] treatments after wake up ===
    pw_set_pll_usb_96MHz();
    gpio_put(PIN_DCDC_PSM_CTRL, 1); // PWM mode for less Audio noise
    pm_backlight_init(GET_CFG_MENU_DISPLAY_BACKLIGHT_HIGH_LEVEL);
    pm_backlight_update();

    // wake up alert
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    sleep_ms(500);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

void pw_set_pll_usb_96MHz()
{
    // Set PLL_USB 96MHz
    pll_init(pll_usb, 1, 1536 * MHZ, 4, 4);
    clock_configure(clk_usb,
        0,
        CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
        96 * MHZ,
        48 * MHZ);
    // Change clk_sys to be 96MHz.
    clock_configure(clk_sys,
        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
        96 * MHZ,
        96 * MHZ);
    // CLK peri is clocked from clk_sys so need to change clk_peri's freq
    clock_configure(clk_peri,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
        96 * MHZ,
        96 * MHZ);
    // Reinit uart/usb_cdc now that clk_peri has changed
    stdio_uart_init();
    stdio_usb_init(); // don't call multiple times without stdio_usb_deinit because of duplicated IRQ calls
}