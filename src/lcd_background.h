/*-----------------------------------------------------------/
/ lcd_background
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#pragma once

#include "lcd_extra.h"

// extern "C" is needed to overwrite (weak) LCD_GetBackground() in lcd_extra.c (pico_st7735_80x160 library)

#ifdef __cplusplus
extern "C" {
#endif

u16 LCD_GetBackground(u16 x, u16 y);

#ifdef __cplusplus
}
#endif
