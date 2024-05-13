/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/
/* Except for 'recover_from_sleep' part, see comment for copyright */

#include "power_manage.h"

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

//#include <cstdio>

//#define USE_ACTIVE_BATTERY_CHECK // Additional circuit needed
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
#ifdef USE_ACTIVE_BATTERY_CHECK
// Battery Check Pin
static constexpr uint32_t PIN_BATT_CHECK = 8;
// Battery Voltage Pin (GPIO28: ADC2)
static constexpr uint32_t PIN_BATT_LVL = 28;
static constexpr uint32_t ADC_PIN_BATT_LVL = 2;
#else // USE_ACTIVE_BATTERY_CHECK
// Battery Voltage Pin (GPIO29: ADC3) (Raspberry Pi Pico built-in circuit)
static constexpr uint32_t PIN_BATT_LVL = 29;
static constexpr uint32_t ADC_PIN_BATT_LVL = 3;
#endif // USE_ACTIVE_BATTERY_CHECK

// ADC Timer & frequency
static repeating_timer_t timer;
const int TIMER_BATTERY_CHECK_HZ = 20;

// Battery monitor interval
const int BATT_CHECK_INTERVAL_SEC = 5;
static float _battery_voltage = 4.2;

// for preserving clock configuration
static uint32_t _scr;
static uint32_t _sleep_en0;
static uint32_t _sleep_en1;

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
    ConfigMenu& cfg = ConfigMenu::instance();
    OLED_BLK_Set_PWM(cfg.get(ConfigMenuId::DISPLAY_BACKLIGHT_HIGH_LEVEL));

    // Battery Check Timer start
    timer_init_battery_check();
}

void pm_set_audio_dac_enable(bool flag)
{
    gpio_put(PIN_AUDIO_DAC_ENABLE, flag);
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
        const float coef_a = 4.2;
        const float coef_b = -0.02;
#else // USE_ACTIVE_BATTERY_CHECK
        // ADC3 pin is connected to middle point of voltage divider 200Kohm + 100Kohm
        const float coef_a = 9.875;
        const float coef_b = -0.02;
#endif // USE_ACTIVE_BATTERY_CHECK
        adc_select_input(ADC_PIN_BATT_LVL);
        uint16_t result = adc_read();
#ifdef USE_ACTIVE_BATTERY_CHECK
        gpio_put(PIN_BATT_CHECK, 0);
#endif // USE_ACTIVE_BATTERY_CHECK
        float voltage = result * coef_a / 4095 + coef_b;
        if (_board_type == WAVESHARE_RP2040_LCD_096) {
            voltage += 0.33;  // Forward voltage of D2 (MBR230LSFT1G)
        }
        //printf("Battery Voltage = %7.4f (V)\n", voltage);
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
    return (_battery_voltage < 2.9);
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