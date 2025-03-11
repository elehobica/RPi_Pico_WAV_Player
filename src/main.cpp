/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <cstdio>
#include <cstring>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

#include "common.h"
#include "lcd.h"
#include "power_manage.h"
#include "UIMode.h"
#include "ui_control.h"

static inline uint32_t _millis(void)
{
    return to_ms_since_boot(get_absolute_time());
}

int main() {
    // Call stdio_usb_init() in pw_set_pll_usb_96MHz() for modified code rather than stdio_init_all()
    //stdio_init_all();

    // Judge whether board is Pico or Waveshare RP2040-LCD-0.96
    // PICO_DEFAULT_LED_PIN: GPIO25
    //    Raspberry Pi Pico: board LED (connected to LCD only)
    //    Waveshare RP2040-LCD-0.96: LCD BLK (pulled-up to 3.3K and connected to driver of backlight)
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_IN);
#if defined(RASPBERRYPI_PICO2)
    board_type_t board_type = gpio_get(PICO_DEFAULT_LED_PIN) ? WAVESHARE_RP2350_LCD_096 : RASPBERRY_PI_PICO_2;
#else
    board_type_t board_type = gpio_get(PICO_DEFAULT_LED_PIN) ? WAVESHARE_RP2040_LCD_096 : RASPBERRY_PI_PICO;
#endif
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    // Set PLL_USB 96MHz and use it for PIO clock for I2S
    pw_set_pll_usb_96MHz();

    // ADC Initialize
    adc_init();

    // Power Manage Init
    pm_init(board_type);

    // Wait before stable power-on for 750ms
    // to avoid unintended power-on when Headphone plug in
    for (int i = 0; i < 30; i++) {
        sleep_ms(25);
    }
    printf("\r\n");
    switch (board_type) {
        case RASPBERRY_PI_PICO:
            printf("Board: Raspberry Pi Pico\r\n");
            break;
        case WAVESHARE_RP2040_LCD_096:
            printf("Board: Waveshare RP2040-LCD-0.96\r\n");
            break;
        case RASPBERRY_PI_PICO_2:
            printf("Board: Raspberry Pi Pico 2\r\n");
            break;
        case WAVESHARE_RP2350_LCD_096:
            printf("Board: Waveshare RP2350-LCD-0.96\r\n");
            break;
        default:
            printf("Board: unknown\r\n");
            break;
    }
    if (pm_get_active_battery_check()) {
        printf("Active Battery Check\r\n");
    } else {
        printf("Static Battery Check\r\n");
    }

    // UI initialize
    ui_init(board_type);

    // UI Loop (infinite)
    const int LoopCycleMs = UIMode::UpdateCycleMs; // loop cycle (50 ms)
    while (true) {
        uint32_t time = _millis();
        ui_update();
        time = _millis() - time;
        if (time < LoopCycleMs) {
            sleep_ms(LoopCycleMs - time);
        } else {
            sleep_ms(1);
        }
    }

    return 0;
}
