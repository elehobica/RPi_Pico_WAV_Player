/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "power_manage.h"
#include "ui_control.h"

const char *VersionStr = "0.8.2";

// PIN setting
static const uint32_t PIN_LED = 25;

static inline uint32_t _millis(void)
{
	return to_ms_since_boot(get_absolute_time());
}

static void audio_clock_96MHz()
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
    // Reinit uart now that clk_peri has changed
    stdio_init_all();
}

int main() {
    stdio_init_all();

    // Set PLL_USB 96MHz and use it for PIO clock for I2S
    audio_clock_96MHz();

    // Power Manage Init
    pm_init();

    // Initialise UART 0
    uart_init(uart0, 115200);
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    // LED
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);

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
