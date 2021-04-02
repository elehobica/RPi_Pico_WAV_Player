/*-----------------------------------------------------------/
/ LcdElementBox: Lcd Element Box API
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include "LcdElementBox.h"

//=================================
// Implementation of ImageBox class
//=================================
ImageBox::ImageBox(int16_t pos_x, int16_t pos_y, uint16_t width, uint16_t height, uint16_t bgColor)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), width(width), height(height), bgColor(bgColor),
        image(NULL), img_w(width), img_h(height), align(center)
{
    image = (uint16_t *) calloc(2, width * height);
}

void ImageBox::setBgColor(uint16_t bgColor)
{
    if (this->bgColor == bgColor) { return; }
    this->bgColor = bgColor;
    update();
}

void ImageBox::update()
{
    isUpdated = true;
}

void ImageBox::draw()
{
    if (!isUpdated || image == NULL) { return; }
    isUpdated = false;
    int16_t ofs_x = 0;
    int16_t ofs_y = 0;

    if (align == center) {
        ofs_x = (width-img_w)/2;
        ofs_y = (height-img_h)/2;
    }
    LCD_ShowPicture(
        pos_x+ofs_x, pos_y+ofs_y,
        pos_x+ofs_x+img_w-1, pos_y+ofs_y+img_h-1,
        (u8 *) image
    );
}

void ImageBox::clear()
{
    LCD_Fill(pos_x, pos_y, pos_x+width-1, pos_y+height-1, bgColor);
}

void ImageBox::getImagePtr(uint16_t **img_ptr, uint16_t *width, uint16_t *height)
{
    *img_ptr = this->image;
    *width = this->width;
    *height = this->height;
}

void ImageBox::setImageSize(int16_t img_w, int16_t img_h)
{
    this->img_w = img_w;
    this->img_h = img_h;
    update();
}

void ImageBox::setPixel(int16_t x, int16_t y, uint16_t rgb565)
{
    if (image == NULL) { return; }
    if (!(x >= 0 && x < img_w && y >= 0 && y < img_h)) { return; }
    image[img_w*y+x] = rgb565;
}

void ImageBox::clearBuf()
{
    this->img_w = width;
    this->img_h = height;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            setPixel(x, y, bgColor);
        }
    }
}

//=================================
// Implementation of IconBox class
//=================================
IconBox::IconBox(int16_t pos_x, int16_t pos_y, uint16_t fgColor, uint16_t bgColor)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), fgColor(fgColor), bgColor(bgColor), icon(0) {}

IconBox::IconBox(int16_t pos_x, int16_t pos_y, uint8_t icon, uint16_t fgColor, uint16_t bgColor)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), fgColor(fgColor), bgColor(bgColor), icon(icon) {}

void IconBox::setFgColor(uint16_t fgColor)
{
    if (this->fgColor == fgColor) { return; }
    this->fgColor = fgColor;
    update();
}

void IconBox::setBgColor(uint16_t bgColor)
{
    if (this->bgColor == bgColor) { return; }
    this->bgColor = bgColor;
    update();
}

void IconBox::update()
{
    isUpdated = true;
}

void IconBox::draw()
{
    if (!isUpdated) { return; }
    isUpdated = false;
    clear();
    LCD_ShowIcon(pos_x, pos_y, icon, 1, fgColor);
}

void IconBox::clear()
{
    LCD_Fill(pos_x, pos_y, pos_x+iconWidth-1, pos_y+iconHeight-1, bgColor);
}

void IconBox::setIcon(uint8_t icon)
{
    if (this->icon == icon) { return; }
    this->icon = icon;
    update();
}

//=================================
// Implementation of TextBox class
//=================================
TextBox::TextBox(int16_t pos_x, int16_t pos_y, uint16_t fgColor, uint16_t bgColor)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), fgColor(fgColor), bgColor(bgColor),
      x0(pos_x), y0(pos_y), w0(0), h0(0), align(LcdElementBox::AlignLeft), str("") {}

TextBox::TextBox(int16_t pos_x, int16_t pos_y, align_enm align, uint16_t fgColor, uint16_t bgColor)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), fgColor(fgColor), bgColor(bgColor),
      x0(pos_x), y0(pos_y), w0(0), h0(0), align(align), str("") {}

TextBox::TextBox(int16_t pos_x, int16_t pos_y, const char *str, align_enm align, uint16_t fgColor, uint16_t bgColor)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), fgColor(fgColor), bgColor(bgColor),
      x0(pos_x), y0(pos_y), w0(0), h0(0), align(align)
{
    setText(str);
}

void TextBox::setFgColor(uint16_t fgColor)
{
    if (this->fgColor == fgColor) { return; }
    this->fgColor = fgColor;
    update();
}

void TextBox::setBgColor(uint16_t bgColor)
{
    if (this->bgColor == bgColor) { return; }
    this->bgColor = bgColor;
    update();
}

void TextBox::update()
{
    isUpdated = true;
}

void TextBox::draw()
{
    if (!isUpdated) { return; }
    int16_t x_ofs;
    isUpdated = false;
    TextBox::clear(); // call clear() of this class
    w0 = strlen(str)*8;
    h0 = 16;
    x_ofs = (align == LcdElementBox::AlignRight) ? -w0 : (align == LcdElementBox::AlignCenter) ? -w0/2 : 0; // at this point, w0 could be not correct
    x0 = pos_x+x_ofs;
    y0 = pos_y;
    LCD_ShowStringLn(x0, y0, x0, x0+w0-1, (u8 *) str, fgColor);
}

void TextBox::clear()
{
    LCD_Fill(x0, y0, x0+w0-1, y0+h0-1, bgColor); // clear previous rectangle
}

void TextBox::setText(const char *str)
{
    w0 = 0;
    h0 = 16;
    if (strncmp(this->str, str, charSize) == 0) { return; }
    w0 = strlen(str)*8;
    memcpy(this->str, str, charSize);
    update();
}

void TextBox::setFormatText(const char *fmt, ...)
{
    char str_temp[charSize];
    va_list va;
    va_start(va, fmt);
    vsprintf(str_temp, fmt, va);
    va_end(va);
    setText(str_temp);
}

void TextBox::setInt(int value)
{
    setFormatText("%d", value);
}

//=================================
// Implementation of IconTextBox class
//=================================
IconTextBox::IconTextBox(int16_t pos_x, int16_t pos_y, uint8_t icon, uint16_t fgColor, uint16_t bgColor)
    : TextBox(pos_x+16, pos_y, fgColor, bgColor), iconBox(pos_x, pos_y, icon, fgColor, bgColor) {}

void IconTextBox::setFgColor(uint16_t fgColor)
{
    iconBox.setFgColor(fgColor);
    TextBox::setFgColor(fgColor);
}

void IconTextBox::setBgColor(uint16_t bgColor)
{
    iconBox.setBgColor(bgColor);
    TextBox::setBgColor(bgColor);
}

void IconTextBox::update()
{
    iconBox.update();
    TextBox::update();
}

void IconTextBox::draw()
{
    // For IconBox: Don't display IconBox if str of TextBox is ""
    if (strlen(str) == 0) {
        if (isUpdated) { iconBox.clear(); }
    } else {
        iconBox.draw();
    }
    // For TextBox
    TextBox::draw();
}

void IconTextBox::clear()
{
    iconBox.clear();
    TextBox::clear();
}

void IconTextBox::setIcon(uint8_t icon)
{
    iconBox.setIcon(icon);
}

//=================================
// Implementation of ScrollTextBox class < LcdElementBox
//=================================
ScrollTextBox::ScrollTextBox(int16_t pos_x, int16_t pos_y, uint16_t width, uint16_t height, uint16_t fgColor, uint16_t bgColor)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), fgColor(fgColor), bgColor(bgColor),
      str(""), width(width), height(height), sft_val(0), count(0), scr_en(true)
{
}

ScrollTextBox::~ScrollTextBox()
{
}

void ScrollTextBox::setFgColor(uint16_t fgColor)
{
    if (this->fgColor == fgColor) { return; }
    this->fgColor = fgColor;
    update();
}

void ScrollTextBox::setBgColor(uint16_t bgColor)
{
    if (this->bgColor == bgColor) { return; }
    this->bgColor = bgColor;
    update();
}

void ScrollTextBox::update()
{
    isUpdated = true;
    sft_val = 0;
    count = 0;
}

void ScrollTextBox::draw()
{
    if (!isUpdated && !scr_en) { return; }
    if (isUpdated) { ScrollTextBox::clear(); }// call clear() of this class
    isUpdated = false;
    LCD_Scroll_ShowString(pos_x, pos_y, pos_x, pos_x+width-1, (u8 *) str, fgColor, &sft_val, count);
    count++;
}

void ScrollTextBox::clear()
{
    LCD_Fill(pos_x, pos_y, pos_x+width-1, pos_y+height-1, bgColor);
}

void ScrollTextBox::setScroll(bool scr_en)
{
    if (this->scr_en == scr_en) { return; }
    this->scr_en = scr_en;
    update();
}

void ScrollTextBox::setText(const char *str)
{
    if (strncmp(this->str, str, charSize) == 0) { return; }
    update();
    memcpy(this->str, str, charSize);
}

//=================================
// Implementation of IconScrollTextBox class < ScrollTextBox
//=================================
IconScrollTextBox::IconScrollTextBox(int16_t pos_x, int16_t pos_y, uint8_t icon, uint16_t width, uint16_t height, uint16_t fgColor, uint16_t bgColor)
    : ScrollTextBox(pos_x+16, pos_y, width-16, height, fgColor, bgColor), iconBox(pos_x, pos_y, icon, fgColor, bgColor) {}

void IconScrollTextBox::setFgColor(uint16_t fgColor)
{
    iconBox.setFgColor(fgColor);
    ScrollTextBox::setFgColor(fgColor);
}

void IconScrollTextBox::setBgColor(uint16_t bgColor)
{
    iconBox.setBgColor(bgColor);
    ScrollTextBox::setBgColor(bgColor);
}

void IconScrollTextBox::update()
{
    iconBox.update();
    ScrollTextBox::update();
}

void IconScrollTextBox::draw()
{
    // For IconBox: Don't display IconBox if str of ScrollTextBox is ""
    if (strlen(str) == 0) {
        if (isUpdated) { iconBox.clear(); }
    } else {
        iconBox.draw();
    }
    // For ScrollTextBox
    ScrollTextBox::draw();
}

void IconScrollTextBox::clear()
{
    iconBox.clear();
    ScrollTextBox::clear();
}

void IconScrollTextBox::setIcon(uint8_t icon)
{
    iconBox.setIcon(icon);
}