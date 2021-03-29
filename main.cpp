/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "stack.h"
#include "ui_control.h"
#include "LcdCanvas.h"
#include "fatfs/ff.h"
#include "ImageFitter.h"

const int Version = 0*100*100 + 0*100 + 1;
unsigned char image[160*80*2];
stack_t *dir_stack;
const int LoopCycleMs = UIMode::UpdateCycleMs; // loop cycle (50 ms)

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

int main() {
    int count = 0;
    FATFS fs;
    FIL fil;
    FRESULT fr;     /* FatFs return code */
    UINT br;
    UINT bw;

    stdio_init_all();

    // Initialise UART 0
    uart_init(uart0, 115200);
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    // LED
    const uint32_t LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // BackLight PWM (125MHz / 65536 / 4 = 476.84 Hz)
    gpio_set_function(PIN_LCD_BLK, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.f);
    pwm_init(slice_num, &config, true);
    int bl_val = 196;
    // Square bl_val to make brightness appear more linear
    pwm_set_gpio_level(PIN_LCD_BLK, bl_val * bl_val);

    // init LCD
    LcdCanvas lcd = LcdCanvas();

    // Progress Bar display before stable power-on for 1 sec
    // to avoid unintended power-on when Headphone plug in
    for (int i = 0; i < 40; i++) {
        LCD_Fill(i*LCD_W/40, LCD_H-8, (i+1)*LCD_W/40-1, LCD_H-1, GRAY);
        sleep_ms(25);
    }

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

    // Display Logo
    LCD_Clear(WHITE);
    imgFit.config((uint16_t *) image, 160, 80);
    imgFit.loadJpegFile("logo.jpg");
    uint16_t w, h;
    imgFit.getSizeAfterFit(&w, &h);
    LCD_ShowPicture(0, 0, w-1, h-1);
    sleep_ms(1000);
    LCD_Clear(BLACK);

    // UI initialize
    ui_init(FileViewMode, dir_stack, &lcd, fs.fs_type);

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