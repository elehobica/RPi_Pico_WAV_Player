/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "power_manage.h"
#include "ui_control.h"
#include "lcd.h"

const char *VersionStr = "0.9.3";

static inline uint32_t _millis(void)
{
	return to_ms_since_boot(get_absolute_time());
}

int main() {
    // Call stdio_usb_init() in pw_set_pll_usb_96MHz() for modified code rather than stdio_init_all()
    //stdio_init_all();

    // Set PLL_USB 96MHz and use it for PIO clock for I2S
    pw_set_pll_usb_96MHz();

    // Power Manage Init
    pm_init();

    // LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    // Wait before stable power-on for 750ms
    // to avoid unintended power-on when Headphone plug in
    for (int i = 0; i < 30; i++) {
        sleep_ms(25);
    }
    printf("Raspberry Pi Pico Player ver. %s\n", VersionStr);

    // ADC Initialize
    adc_init();

    // UI initialize
    ui_init();

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
