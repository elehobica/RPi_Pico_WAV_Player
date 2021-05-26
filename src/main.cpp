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
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "stack.h"
#include "ui_control.h"
#include "LcdCanvas.h"
#include "PlayAudio/audio_codec.h"
#include "lcd_background.h"

const int Version = 0*100*100 + 0*100 + 1;
unsigned char image[160*80*2];
stack_t *dir_stack;
const int LoopCycleMs = UIMode::UpdateCycleMs; // loop cycle (50 ms)

static const uint32_t PIN_LED = 25;
static const uint32_t PIN_POWER_KEEP = 19;
static const uint32_t PIN_DCDC_PSM_CTRL = 23;
static const uint32_t PIN_AUDIO_MUTE_CTRL = 27;

static inline uint32_t _millis(void)
{
	return to_ms_since_boot(get_absolute_time());
}

void power_off(const char *msg, int is_error)
{
    LCD_Clear(BLACK);
    if (strlen(msg) > 0) {
        LCD_ShowString(24,  0, (u8 *) msg, BLACK);
        LCD_ShowString(24, 16, (u8 *) msg, BLUE);
        LCD_ShowString(24, 32, (u8 *) msg, BRED);
        LCD_ShowString(24, 48, (u8 *) msg, GBLUE);
        LCD_ShowString(24, 64, (u8 *) msg, RED);
        sleep_ms(1000);
    }
    while (1) {
        sleep_ms(10);
    }
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
    int count = 0;
    FATFS fs;
    FIL fil;
    FRESULT fr;     /* FatFs return code */
    UINT br;
    UINT bw;

    stdio_init_all();

    // Power Keep Pin (Disable at initial)
    gpio_init(PIN_POWER_KEEP);
    gpio_set_dir(PIN_POWER_KEEP, GPIO_OUT);
    gpio_put(PIN_POWER_KEEP, 0);

    // DCDC PSM control
    // 0: PFM mode (best efficiency)
    // 1: PWM mode (improved ripple)
    gpio_init(PIN_DCDC_PSM_CTRL);
    gpio_set_dir(PIN_DCDC_PSM_CTRL, GPIO_OUT);
    gpio_put(PIN_DCDC_PSM_CTRL, 1); // PWM mode for less Audio noise

    // Audio Mute ON
    gpio_init(PIN_AUDIO_MUTE_CTRL);
    gpio_set_dir(PIN_AUDIO_MUTE_CTRL, GPIO_OUT);
    gpio_put(PIN_AUDIO_MUTE_CTRL, 1);

    audio_clock_96MHz();

    // Initialise UART 0
    uart_init(uart0, 115200);
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    // LED
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);

    // BackLight PWM (125MHz / 65536 / 4 = 476.84 Hz)
    gpio_set_function(PIN_LCD_BLK, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.f);
    pwm_init(slice_num, &config, true);
    int bl_val = 196;
    // Square bl_val to make brightness appear more linear
    pwm_set_gpio_level(PIN_LCD_BLK, bl_val * bl_val);

    // Progress Bar display before stable power-on for 1 sec
    // to avoid unintended power-on when Headphone plug in
    for (int i = 0; i < 40; i++) {
        //LCD_Fill(i*LCD_W/40, LCD_H-8, (i+1)*LCD_W/40-1, LCD_H-1, GRAY);
        sleep_ms(25);
    }

    // Power Keep Enable
    gpio_put(PIN_POWER_KEEP, 1);

    dir_stack = stack_init();
    // Mount FAT
    while (true) {
        fr = f_mount(&fs, "", 1); // 0: mount successful ; 1: mount failed
        if (fr == FR_OK || count++ > 10) break;
        sleep_ms(10);
    }

    if (fr != FR_OK) { // Mount Fail (Loop)
        power_off("No Card Found!", 1);
    }

    printf("Raspberry Pi Pico Player ver %d.%d.%d\n\r", (Version/10000)%100, (Version/100)%100, (Version/1)%100);
    printf("SD Card File System = %d\n\r", fs.fs_type); // FS_EXFAT = 4

    // Opening Logo
    lcd.setImageJpeg("logo.jpg");

    // Audio Mute OFF
    gpio_put(PIN_AUDIO_MUTE_CTRL, 0);
    // Audio Codec Initialize
    audio_codec_init();

    // UI initialize
    ui_init(FileViewMode, dir_stack, fs.fs_type);

    // UI Loop
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
