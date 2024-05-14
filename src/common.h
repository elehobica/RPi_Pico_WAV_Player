/*------------------------------------------------------/
/ Copyright (c) 2024, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#pragma once

#include <cstdint>

typedef enum _board_type_t {
    RASPBERRY_PI_PICO = 0,
    WAVESHARE_RP2040_LCD_096
} board_type_t;

typedef struct {
    uint16_t head;
    uint16_t column;
} stack_data_t;

typedef enum {
    InitialMode = 0,
    ChargeMode,
    OpeningMode,
    FileViewMode,
    PlayMode,
    ConfigMode,
    PowerOffMode,
    NUM_UI_MODES
} ui_mode_enm_t;
