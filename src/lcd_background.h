/*-----------------------------------------------------------/
/ lcd_background
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#ifndef __LCD_BACKGROUND_H_INCLUDED__
#define __LCD_BACKGROUND_H_INCLUDED__

#include <stdlib.h>
#include "st7735_80x160/lcd.h"

#ifdef __cplusplus
extern "C" {
#endif

u16 LCD_GetBackground(u16 x, u16 y);

#ifdef __cplusplus
}
#endif

#endif // __LCD_BACKGROUND_H_INCLUDED__
