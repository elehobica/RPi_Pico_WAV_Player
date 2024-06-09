/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/
/* Except for 'recover_from_sleep' part, see comment for copyright */

#include "power_manage.h"

//#include <cstdio>

#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/structs/scb.h"
#include "hardware/rosc.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/sync.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include "pico/sleep.h"
#include "pico/stdio_uart.h"
#include "pico/stdio_usb.h" // use lib/pico_stdio_usb_revised/

#include "ConfigParam.h"
#include "ConfigMenu.h"
#include "lcd_extra.h"
#include "ui_control.h"
#include "UIMode.h"

//#define NO_BATTERY_VOLTAGE_CHECK

static board_type_t _board_type;

// === Pin Settings for power management ===
// DC/DC mode selection Pin
static constexpr uint32_t PIN_DCDC_PSM_CTRL = 23;
// USB Charge detect Pin
static constexpr uint32_t PIN_USB_POWER_DETECT = 24;
// Conditional pullup for PIN_HP_BUTTON
static constexpr uint32_t PIN_COND_PU_BUTTONS = 21;
// Power Keep Pin
static constexpr uint32_t PIN_POWER_KEEP = 19;
// Audio DAC Enable Pin
static constexpr uint32_t PIN_AUDIO_DAC_ENABLE = 27;
// Battery Check Pin
static constexpr uint32_t PIN_ACTIVE_BATT_CHECK = 8;
// Battery Voltage Pin (GPIO28: ADC2)
static constexpr uint32_t PIN_ACTIVE_BATT_LVL = 28;
static constexpr uint32_t ADC_PIN_ACTIVE_BATT_LVL = 2;
// ADC2 pin is connected to middle point of voltage divider 2.2Kohm + 3.3Kohm (active)
static constexpr float COEF_A_ACTIVE_BATT_CHECK = 5.48;
static constexpr float COEF_B_ACTIVE_BATT_CHECK = -0.02;
// Battery Voltage Pin (GPIO29: ADC3) (Raspberry Pi Pico built-in circuit)
static constexpr uint32_t PIN_STATIC_BATT_LVL = 29;
static constexpr uint32_t ADC_PIN_STATIC_BATT_LVL = 3;
// ADC3 pin is connected to middle point of voltage divider 200Kohm + 100Kohm (static)
static constexpr float COEF_A_STATIC_BATT_CHECK = 9.875;
static constexpr float COEF_B_STATIC_BATT_CHECK = -0.02;

// flag for active battery check
static bool _use_active_batt_check = false;

// ADC Timer & frequency
static repeating_timer_t timer;
const int TIMER_BATTERY_CHECK_HZ = 20;

// Battery monitor interval
constexpr int BATT_CHECK_INTERVAL_SEC = 5;
constexpr float LOW_BATT_LVL = 2.9;
static float _battery_voltage = 4.2;

// for preserving clock configuration
static uint32_t _scr;
static uint32_t _sleep_en0;
static uint32_t _sleep_en1;

static bool _timer_callback_battery_check(repeating_timer_t *rt) {
    pm_monitor_battery_voltage();
    return true; // keep repeating
}

static int _timer_init_battery_check()
{
    // negative timeout means exact delay (rather than delay between callbacks)
    if (!add_repeating_timer_us(-1000000 / TIMER_BATTERY_CHECK_HZ, _timer_callback_battery_check, nullptr, &timer)) {
        //printf("Failed to add timer\r\n");
        return 0;
    }
    return 1;
}

static float _get_battery_voltage(bool use_active_batt_check)
{
    uint32_t adc_pin_batt_lvl = (use_active_batt_check) ? ADC_PIN_ACTIVE_BATT_LVL : ADC_PIN_STATIC_BATT_LVL;
    adc_select_input(adc_pin_batt_lvl);
    uint16_t result = adc_read();
    // ADC Calibration Coefficients
    const float coef_a = (use_active_batt_check) ? COEF_A_ACTIVE_BATT_CHECK : COEF_A_STATIC_BATT_CHECK;
    const float coef_b = (use_active_batt_check) ? COEF_B_ACTIVE_BATT_CHECK : COEF_B_STATIC_BATT_CHECK;
    float voltage = static_cast<float>(result) * coef_a / 4095 + coef_b;
    //printf("Battery Voltage = %7.4f V (%dd)\r\n", voltage, result);
    return voltage;
}

void pm_backlight_update()
{
    const int LoopCycleMs = UIMode::UpdateCycleMs; // loop cycle (50 ms)
    const int OneSec = 1000 / LoopCycleMs;
    uint32_t bl_val;
    ConfigMenu& cfg = ConfigMenu::instance();
    if (ui_get_idle_count() < cfg.get(ConfigMenuId::DISPLAY_TIME_TO_BACKLIGHT_LOW)*OneSec) {
        bl_val = cfg.get(ConfigMenuId::DISPLAY_BACKLIGHT_HIGH_LEVEL);
    } else {
        bl_val = cfg.get(ConfigMenuId::DISPLAY_BACKLIGHT_LOW_LEVEL);
    }
    OLED_BLK_Set_PWM(bl_val);
}

void pm_init(board_type_t board_type)
{
    // Board type
    _board_type = board_type;

    // USB Power detect Pin = Charge detect (Input)
    gpio_set_dir(PIN_USB_POWER_DETECT, GPIO_IN);

    // Power Keep Pin (Output)
    gpio_init(PIN_POWER_KEEP);
    gpio_set_dir(PIN_POWER_KEEP, GPIO_OUT);

    // Conditional pullup for HP buttons Pin (Output)
    gpio_init(PIN_COND_PU_BUTTONS);
    gpio_set_dir(PIN_COND_PU_BUTTONS, GPIO_OUT);

    // Audio DAC Disable (Mute On)
    gpio_init(PIN_AUDIO_DAC_ENABLE);
    gpio_set_dir(PIN_AUDIO_DAC_ENABLE, GPIO_OUT);
    gpio_put(PIN_AUDIO_DAC_ENABLE, 0);

    // Judge whether active battery check circuit is populated or not
    _use_active_batt_check = false;
    do {
        // If active battery check circuit is populated,
        // battery voltage measured by ADC pin should react depending on Battery Check Enable Pin's level and
        // the level when enable pin is high should be at least higher than LOW_BATT_LVL.
        gpio_init(PIN_ACTIVE_BATT_CHECK);
        gpio_set_dir(PIN_ACTIVE_BATT_CHECK, GPIO_OUT);
        gpio_disable_pulls(PIN_ACTIVE_BATT_LVL);
        adc_gpio_init(PIN_ACTIVE_BATT_LVL);
        adc_select_input(ADC_PIN_ACTIVE_BATT_LVL);
        // check if the voltage is low enough when enable control pin = low
        gpio_put(PIN_ACTIVE_BATT_CHECK, 0);
        sleep_ms(10);
        float voltage = _get_battery_voltage(true);
        if (voltage > LOW_BATT_LVL/10.0) { break; }
        // check if the voltage is high enough when enable control pin = high
        gpio_put(PIN_ACTIVE_BATT_CHECK, 1);
        sleep_ms(10);
        voltage = _get_battery_voltage(true);
        gpio_put(PIN_ACTIVE_BATT_CHECK, 0);
        if (voltage < LOW_BATT_LVL) { break; }
        // active battery check circuit is populated if reached here
        _use_active_batt_check = true;
    } while(false);

    if (_use_active_batt_check) {
        // Battery Level Input (ADC)
        adc_gpio_init(PIN_ACTIVE_BATT_LVL);
        // reset unused pins' configuration
        gpio_init(PIN_STATIC_BATT_LVL);
        gpio_set_dir(PIN_STATIC_BATT_LVL, GPIO_IN);
    } else {
        // Battery Level Input (ADC)
        adc_gpio_init(PIN_STATIC_BATT_LVL);
        // reset unused pins' configuration
        gpio_init(PIN_ACTIVE_BATT_CHECK);
        gpio_set_dir(PIN_ACTIVE_BATT_CHECK, GPIO_IN);
        gpio_init(PIN_ACTIVE_BATT_LVL);
        gpio_set_dir(PIN_ACTIVE_BATT_LVL, GPIO_IN);
    }

    // For Raspberry Pi Pico: DCDC PSM control of RT6150B-33GQW
    //   0: PFM mode (best efficiency)
    //   1: PWM mode (improved ripple) <-- select this for less Audio noise
    // For Waveshare RP2040-LCD-0.96: PS/SYNC control of TPS63000
    //   0: Power-save mode enable (better efficiency)
    //   1: Power-save mode disable    <-- select this
    gpio_init(PIN_DCDC_PSM_CTRL);
    gpio_set_dir(PIN_DCDC_PSM_CTRL, GPIO_OUT);
    gpio_put(PIN_DCDC_PSM_CTRL, 1);

    // BackLight
    ConfigMenu& cfg = ConfigMenu::instance();
    OLED_BLK_Set_PWM(cfg.get(ConfigMenuId::DISPLAY_BACKLIGHT_HIGH_LEVEL));

    // Battery Check Timer start
    _timer_init_battery_check();
}

bool pm_get_active_battery_check()
{
    return _use_active_batt_check;
}

void pm_set_audio_dac_enable(bool flag)
{
    gpio_put(PIN_AUDIO_DAC_ENABLE, flag);
}

void pm_monitor_battery_voltage()
{
    static int count = 0;
    if (_use_active_batt_check && (count % (TIMER_BATTERY_CHECK_HZ*BATT_CHECK_INTERVAL_SEC) == TIMER_BATTERY_CHECK_HZ*BATT_CHECK_INTERVAL_SEC-2)) {
        // Prepare to check battery voltage
        gpio_put(PIN_ACTIVE_BATT_CHECK, 1);
    } else if (count % (TIMER_BATTERY_CHECK_HZ*BATT_CHECK_INTERVAL_SEC) == TIMER_BATTERY_CHECK_HZ*BATT_CHECK_INTERVAL_SEC-1) {
        float voltage = _get_battery_voltage(_use_active_batt_check);
        if (_use_active_batt_check) {
            gpio_put(PIN_ACTIVE_BATT_CHECK, 0);
        }
        if (_board_type == WAVESHARE_RP2040_LCD_096) {
            voltage += 0.33;  // Forward voltage of D2 (MBR230LSFT1G)
        }
        //printf("Battery Voltage = %7.4f V\r\n", voltage);
        _battery_voltage = voltage;
    }
    count++;
}

bool pm_usb_power_detected()
{
    return gpio_get(PIN_USB_POWER_DETECT);
}

void pm_set_power_keep(bool flag)
{
    gpio_put(PIN_POWER_KEEP, flag);
}

void pm_enable_button_control(bool flag)
{
    gpio_put(PIN_COND_PU_BUTTONS, flag);
}

bool pm_get_low_battery()
{
#ifdef NO_BATTERY_VOLTAGE_CHECK
    return false;
#else
    return (_battery_voltage < LOW_BATT_LVL);
#endif
}

float pm_get_battery_voltage()
{
    return _battery_voltage;
}

// === 'recover_from_sleep' part (start) ===================================
// great reference from 'recover_from_sleep'
// https://github.com/ghubcoder/PicoSleepDemo | https://ghubcoder.github.io/posts/awaking-the-pico/
static void _preserve_clock_before_sleep()
{
    _scr = scb_hw->scr;
    _sleep_en0 = clocks_hw->sleep_en0;
    _sleep_en1 = clocks_hw->sleep_en1;
}

static void _recover_clock_after_sleep()
{
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS); //Re-enable ring Oscillator control
    scb_hw->scr = _scr;
    clocks_hw->sleep_en0 = _sleep_en0;
    clocks_hw->sleep_en1 = _sleep_en1;
    clocks_init(); // reset clocks
}
// === 'recover_from_sleep' part (end) ===================================

static void pm_enter_dormant_and_wake_core(uint32_t pin)
{
    // === [2] goto dormant then wake up ===
    uint32_t ints = save_and_disable_interrupts(); // (+stepB)
    _preserve_clock_before_sleep(); // (+stepC)
    //--
    sleep_run_from_xosc();
    sleep_goto_dormant_until_pin(pin, true, false); // dormant until fall edge detected
    //--
    _recover_clock_after_sleep(); // (-stepC)
    restore_interrupts(ints); // (-stepB)
}

void pm_enter_dormant_and_wake()
{
    // === [1] Preparation for dormant ===
    OLED_BLK_Set_PWM(0);

    gpio_put(PIN_DCDC_PSM_CTRL, 0); // PFM mode for better efficiency
    stdio_usb_deinit(); // terminate usb cdc

    {
        // set ADC pin to wake up gpio pin (+stepA)
        uint32_t pin = ui_set_center_switch_for_wakeup(true);

        // === [2] goto dormant then wake up ===
        pm_enter_dormant_and_wake_core(pin);
        // repeat dormant unless pin is being pushed for 500ms
        sleep_ms(500);
        while (gpio_get(pin) == true) {
            pm_enter_dormant_and_wake_core(pin);
            sleep_ms(500);
        }

        // wake up gpio pin recover to ADC pin function (-stepA)
        ui_set_center_switch_for_wakeup(false);
    }

    // === [3] treatments after wake up ===
    pw_set_pll_usb_96MHz();
    gpio_put(PIN_DCDC_PSM_CTRL, 1); // PWM mode for less Audio noise
    pm_backlight_update();

    // wake up alert
    sleep_ms(500);
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

void pm_reboot()
{
    watchdog_reboot(0, 0, PICO_STDIO_USB_RESET_RESET_TO_FLASH_DELAY_MS);
}

bool pm_is_caused_reboot()
{
    return watchdog_caused_reboot();
}