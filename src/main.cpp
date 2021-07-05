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
#include "PlayAudio/PlayAudio.h"
#include "lcd_background.h"
#include "file_menu/file_menu_FatFs.h"
#include "UserFlash.h"
#include "ConfigParam.h"
#include "ConfigMenu.h"

//#define INITIALIZE_CONFIG_PARAM

const char *VersionStr = "0.8.2";

// PIN setting
static const uint32_t PIN_LED = 25;
static const uint32_t PIN_POWER_KEEP = 19;
static const uint32_t PIN_DCDC_PSM_CTRL = 23;
static const uint32_t PIN_AUDIO_DAC_ENABLE = 27;

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

static void loadFromFlash(stack_t *dir_stack, ui_mode_enm_t *init_dest_mode)
{
    // Load Configuration parameters from Flash
    userFlash.printInfo();
    #ifdef INITIALIZE_CONFIG_PARAM
    configParam.initialize(ConfigParam::FORCE_LOAD_DEFAULT);
    #else // INITIALIZE_CONFIG_PARAM
    configParam.initialize(ConfigParam::LOAD_DEFAULT_IF_FLASH_IS_BLANK);
    #endif // INITIALIZE_CONFIG_PARAM
    configMenu.scanHookFunc();
    configParam.printInfo();
    configParam.incBootCount();

    // Restore from configParam to user parameters
    srand(GET_CFG_SEED);
    PlayAudio::setVolume(GET_CFG_VOLUME);
    bool err_flg = false;
    for (int i = GET_CFG_STACK_COUNT - 1; i >= 0; i--) {
        stack_data_t item;
        item.head = GET_CFG_STACK_HEAD(i);
        item.column = GET_CFG_STACK_COLUMN(i);
        if (item.head+item.column >= file_menu_get_num()) { err_flg = true; break; } // idx overflow
        file_menu_sort_entry(item.head+item.column, item.head+item.column + 1);
        if (file_menu_is_dir(item.head+item.column) <= 0 || item.head+item.column == 0) { err_flg = true; break; } // Not Directory or Parent Directory
        stack_push(dir_stack, &item);
        file_menu_ch_dir(item.head+item.column);
    }

    *init_dest_mode = static_cast<ui_mode_enm_t>(GET_CFG_UIMODE);

    uint16_t idx_head = GET_CFG_IDX_HEAD;
    uint16_t idx_column = GET_CFG_IDX_COLUMN;
    if (idx_head+idx_column >= file_menu_get_num()) { err_flg = true; } // idx overflow
    if (err_flg) { // Load Error
        stack_delete(dir_stack);
        dir_stack = stack_init();
        file_menu_open_dir("/");
        idx_head = idx_column = 0;
        *init_dest_mode = FileViewMode;
    }
    uiv_set_file_idx(idx_head, idx_column);

    uint16_t idx_play = 0;
    size_t fpos = 0;
    uint32_t samples_played = 0;
    if (*init_dest_mode == PlayMode) {
        idx_play = GET_CFG_IDX_PLAY;
        uint64_t play_pos = GET_CFG_PLAY_POS;
        fpos = (size_t) play_pos;
        samples_played = GET_CFG_SAMPLES_PLAYED;
    }
    uiv_set_play_idx(idx_play);
    uiv_set_play_position(fpos, samples_played);
}

static void storeToFlash(stack_t *dir_stack)
{
    // Save user parameters to configParam
    configParam.setU32(ConfigParam::CFG_SEED, _millis());
    uint8_t volume = PlayAudio::getVolume();
    configParam.setU8(ConfigParam::CFG_VOLUME, volume);
    uint8_t stack_count = stack_get_count(dir_stack);
    configParam.setU8(ConfigParam::CFG_STACK_COUNT, stack_count);
    for (int i = 0; i < stack_count; i++) {
        stack_data_t item;
        stack_pop(dir_stack, &item);
        configParam.setU16(CFG_STACK_HEAD(i), item.head);
        configParam.setU16(CFG_STACK_COLUMN(i), item.column);
    }

    ui_mode_enm_t resume_ui_mode;
    uiv_get_resume_ui_mode(&resume_ui_mode);
    configParam.setU32(ConfigParam::CFG_UIMODE, resume_ui_mode);

    uint16_t idx_head, idx_column, idx_play;
    uiv_get_file_idx(&idx_head, &idx_column);
    uiv_get_play_idx(&idx_play);
    configParam.setU16(ConfigParam::CFG_IDX_HEAD, idx_head);
    configParam.setU16(ConfigParam::CFG_IDX_COLUMN, idx_column);
    configParam.setU16(ConfigParam::CFG_IDX_PLAY, idx_play);

    size_t fpos;
    uint32_t samples_played;
    uiv_get_play_position(&fpos, &samples_played);
    configParam.setU64(ConfigParam::CFG_PLAY_POS, (uint64_t) fpos);
    configParam.setU32(ConfigParam::CFG_SAMPLES_PLAYED, samples_played);


    // Store Configuration parameters to Flash
    configParam.finalize();
}

static void backlight_init(uint32_t bl_val)
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

static void backlight_update()
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

static void power_off(const char *msg, bool is_error = false)
{
    const int LoopCycleMs = UIMode::UpdateCycleMs; // loop cycle (50 ms)
    if (msg != NULL) { lcd.setMsg(msg, is_error); }
    uint32_t stay_time = (is_error) ? 4000 : 1000;
    uint32_t time = _millis();
    gpio_put(PIN_AUDIO_DAC_ENABLE, 0);
    audio_codec_deinit();
    while (_millis() - time < stay_time) {
        sleep_ms(LoopCycleMs);
        lcd.drawPowerOff();
    }
    gpio_put(PIN_POWER_KEEP, 0);
    while (true) {} // endless loop
}

int main() {
    int count = 0;
    FATFS fs;
    FRESULT fr;

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

    // Audio DAC Disable (Mute On)
    gpio_init(PIN_AUDIO_DAC_ENABLE);
    gpio_set_dir(PIN_AUDIO_DAC_ENABLE, GPIO_OUT);
    gpio_put(PIN_AUDIO_DAC_ENABLE, 0);

    audio_clock_96MHz();

    // Initialise UART 0
    uart_init(uart0, 115200);
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    // LED
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);

    // BackLight
    backlight_init(192);

    // Wait before stable power-on for 750ms
    // to avoid unintended power-on when Headphone plug in
    for (int i = 0; i < 30; i++) {
        sleep_ms(25);
    }

    printf("Raspberry Pi Pico Player ver. %s\n", VersionStr);

    // Power Keep Enable
    gpio_put(PIN_POWER_KEEP, 1);

    // Mount FAT
    while (true) {
        fr = f_mount(&fs, "", 1); // 0: mount successful ; 1: mount failed
        if (fr == FR_OK || count++ > 10) break;
        sleep_ms(10);
    }
    if (fr != FR_OK) { // Mount Fail
        power_off("No Card Found!", true);
    }

    // Open root directory
    stack_t *dir_stack = stack_init();
    file_menu_open_dir("/");
    if (file_menu_get_num() <= 1) { // Directory read Fail
        power_off("Card Read Error!", true);
    }

    const char *fs_type_str[5] = {"NOT_MOUNTED", "FAT12", "FAT16", "FAT32", "EXFAT"};
    printf("SD Card File System = %s\n", fs_type_str[fs.fs_type]);

    // Opening Logo
    lcd.setImageJpeg("logo.jpg");

    // Load from Flash
    ui_mode_enm_t init_dest_mode;
    loadFromFlash(dir_stack, &init_dest_mode);

    // Audio DAC Enable (Mute Off)
    gpio_put(PIN_AUDIO_DAC_ENABLE, 1);
    // Audio Codec Initialize
    audio_codec_init();

    // UI initialize
    ui_init(init_dest_mode, dir_stack, fs.fs_type);

    // UI Loop
    const int LoopCycleMs = UIMode::UpdateCycleMs; // loop cycle (50 ms)
    while (true) {
        uint32_t time = _millis();

        if (ui_update() == PowerOffMode) { break; }
        backlight_update();

        time = _millis() - time;
        if (time < LoopCycleMs) {
            sleep_ms(LoopCycleMs - time);
        } else {
            sleep_ms(1);
        }
    }

    storeToFlash(dir_stack);
    power_off(NULL, uiv_get_low_battery()); // never return

    return 0;
}
