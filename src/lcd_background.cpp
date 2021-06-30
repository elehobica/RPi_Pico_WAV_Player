/*-----------------------------------------------------------/
/ lcd_background
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#include "LcdCanvas.h"
#include "lcd_background.h"

u16 LCD_GetBackground(u16 x, u16 y)
{
    uint16_t pix =  lcd.getTiledImage(x, y);
    pix = ((pix & 0xff00) >> 8) | ((pix & 0x00ff) << 8);
    uint16_t r = ((pix & 0xf800) >> 2) & 0xf800;
    uint16_t g = ((pix & 0x07e0) >> 2) & 0x07e0;
    uint16_t b = ((pix & 0x001f) >> 2) & 0x001f;
    /*
    uint16_t r = (((pix & 0xf800) >> 2) + ((pix & 0xf800) >> 4)) & 0xf800;
    uint16_t g = (((pix & 0x07e0) >> 2) + ((pix & 0x07e0) >> 4)) & 0x07e0;
    uint16_t b = (((pix & 0x001f) >> 2) + ((pix & 0x001f) >> 4)) & 0x001f;
    */
    pix = r | g | b;
    return (u16) pix;
}
