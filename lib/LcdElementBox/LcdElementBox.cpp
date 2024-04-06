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
    : isUpdated(true), _hasImage(false), pos_x(pos_x), pos_y(pos_y), width(width), height(height), bgColor(bgColor),
        image(NULL), img_w(width), img_h(height), align(center)
{
    image = (uint16_t*) calloc(width * height, sizeof(uint16_t));
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
        (u8*) image
    );
}

void ImageBox::clear()
{
    LCD_Fill(pos_x, pos_y, pos_x+width-1, pos_y+height-1, bgColor);
}

void ImageBox::getImagePtr(uint16_t** img_ptr, uint16_t* width, uint16_t* height)
{
    *img_ptr = this->image;
    *width = this->width;
    *height = this->height;
}

void ImageBox::setImageSize(int16_t img_w, int16_t img_h)
{
    this->img_w = img_w;
    this->img_h = img_h;
    _hasImage = true;
    update();
}

void ImageBox::setPixel(int16_t x, int16_t y, uint16_t rgb565)
{
    if (image == NULL) { return; }
    if (!(x >= 0 && x < img_w && y >= 0 && y < img_h)) { return; }
    image[img_w*y+x] = rgb565;
}

void ImageBox::resetImage()
{
    this->img_w = width;
    this->img_h = height;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            setPixel(x, y, bgColor);
        }
    }
    _hasImage = false;
}

uint16_t ImageBox::getPixel(uint16_t x, uint16_t y, bool tiled)
{
    uint16_t xx = (tiled) ? x % img_w : x;
    uint16_t yy = (tiled) ? y % img_h : y;
    if (xx < img_w && yy < img_h) {
        return image[img_w*yy+xx];
    } else {
        return (uint16_t) 0x0000;
    }
}

bool ImageBox::hasImage()
{
    return _hasImage;
}

//=================================
// Implementation of IconBox class
//=================================
IconBox::IconBox(int16_t pos_x, int16_t pos_y, uint8_t* icon, uint16_t fgColor, uint16_t bgColor, bool bgOpaque)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), fgColor(fgColor), bgColor(bgColor), icon(icon), bgOpaque(bgOpaque) {}

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

void IconBox::setBgOpaque(bool bgOpaque)
{
    if (this->bgOpaque == bgOpaque) { return; }
    this->bgOpaque = bgOpaque;
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
    LCD_ShowIcon(pos_x, pos_y, icon, !bgOpaque, fgColor);
}

void IconBox::clear()
{
    LCD_FillBackground(pos_x, pos_y, pos_x+iconWidth-1, pos_y+iconHeight-1, !bgOpaque, bgColor);
}

void IconBox::setIcon(uint8_t* icon)
{
    if (this->icon == icon) { return; }
    this->icon = icon;
    update();
}

//=================================
// Implementation of TextBox class
//=================================
TextBox::TextBox(int16_t pos_x, int16_t pos_y, uint16_t fgColor, uint16_t bgColor, bool bgOpaque)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), fgColor(fgColor), bgColor(bgColor),
      x0(pos_x), y0(pos_y), w0(0), h0(0), align(LcdElementBox::AlignLeft), bgOpaque(bgOpaque), drawCount(0), blink(false), str("") {}

TextBox::TextBox(int16_t pos_x, int16_t pos_y, align_enm align, uint16_t fgColor, uint16_t bgColor, bool bgOpaque)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), fgColor(fgColor), bgColor(bgColor),
      x0(pos_x), y0(pos_y), w0(0), h0(0), align(align), bgOpaque(bgOpaque), drawCount(0), blink(false), str("") {}

TextBox::TextBox(int16_t pos_x, int16_t pos_y, const char* str, align_enm align, uint16_t fgColor, uint16_t bgColor, bool bgOpaque)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), fgColor(fgColor), bgColor(bgColor),
      x0(pos_x), y0(pos_y), w0(0), h0(0), align(align), bgOpaque(bgOpaque), drawCount(0), blink(false)
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
    if (!isUpdated && !(blink && drawCount % (BlinkInterval/2) == 0)) { drawCount++; return; }
    int16_t x_ofs;
    isUpdated = false;
    //TextBox::clear(); // call clear() of this class
    if (strlen(str) == 0) { return; } // not to calculate x0, y0, w0, h0 because illegal values cause clear() mulfunction
    uint16_t w1, h1;
    int16_t x1, y1;
    w1 = strlen(str)*8;
    h1 = 16;
    x_ofs = (align == LcdElementBox::AlignRight) ? -w1 : (align == LcdElementBox::AlignCenter) ? -w1/2 : 0;
    x1 = pos_x+x_ofs;
    y1 = pos_y;
    // clear left & right wing
    if (x0 < x1) { // left wing
        LCD_FillBackground(x0, y1, x1-1, y1+h1-1, !bgOpaque, bgColor);
    }
    if (x0+w0 > x1+w1) { // right wing
        LCD_FillBackground(x1+w1, y1, x0+w0-1, y1+h1-1, !bgOpaque, bgColor);
    }
    w0 = w1;
    h0 = h1;
    x0 = x1;
    y0 = y1;
    if (blink && (drawCount % BlinkInterval >= (BlinkInterval/2))) { // Blink (Disappear) case
        LCD_FillBackground(x0, y0, x0+w0-1, y0+h0-1, !bgOpaque, bgColor);
    } else {
        LCD_ShowStringLn(x0, y0, x0, x0+w0-1, (u8*) str, !bgOpaque, fgColor);
    }
    drawCount++;
}

void TextBox::clear()
{
    LCD_FillBackground(x0, y0, x0+w0-1, y0+h0-1, !bgOpaque, bgColor); // clear previous rectangle
}

void TextBox::setText(const char* str)
{
    if (strncmp(this->str, str, charSize) == 0) { return; }
    memcpy(this->str, str, charSize);
    update();
}

void TextBox::setFormatText(const char* fmt, ...)
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

void TextBox::setBlink(bool blink)
{
    if (this->blink == blink) { return; }
    this->blink = blink;
    update();
}

//=================================
// Implementation of IconTextBox class
//=================================
IconTextBox::IconTextBox(int16_t pos_x, int16_t pos_y, uint8_t* icon, uint16_t fgColor, uint16_t bgColor, bool bgOpaque)
    : TextBox(pos_x+16, pos_y, fgColor, bgColor, bgOpaque), iconBox(pos_x, pos_y, icon, fgColor, bgColor, bgOpaque) {}

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

void IconTextBox::setIcon(uint8_t* icon)
{
    iconBox.setIcon(icon);
}

//=================================
// Implementation of ScrollTextBox class < LcdElementBox
//=================================
ScrollTextBox::ScrollTextBox(int16_t pos_x, int16_t pos_y, uint16_t width, uint16_t height, uint16_t fgColor, uint16_t bgColor, bool bgOpaque)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), fgColor(fgColor), bgColor(bgColor),
      str(""), width(width), height(height), bgOpaque(bgOpaque), sft_val(0), count(0), scr_en(true)
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
    LCD_Scroll_ShowString(pos_x, pos_y, pos_x, pos_x+width-1, (u8*) str, !bgOpaque, fgColor, (u16*) &sft_val, count);
    count++;
}

void ScrollTextBox::clear()
{
    LCD_FillBackground(pos_x, pos_y, pos_x+width-1, pos_y+height-1, !bgOpaque, bgColor);
}

void ScrollTextBox::setScroll(bool scr_en)
{
    if (this->scr_en == scr_en) { return; }
    this->scr_en = scr_en;
    update();
}

void ScrollTextBox::setText(const char* str)
{
    if (strncmp(this->str, str, charSize) == 0) { return; }
    update();
    memcpy(this->str, str, charSize);
}

//=================================
// Implementation of IconScrollTextBox class < ScrollTextBox
//=================================
IconScrollTextBox::IconScrollTextBox(int16_t pos_x, int16_t pos_y, uint8_t* icon, uint16_t width, uint16_t height, uint16_t fgColor, uint16_t bgColor, bool bgOpaque)
    : ScrollTextBox(pos_x+16, pos_y, width-16, height, fgColor, bgColor, bgOpaque), iconBox(pos_x, pos_y, icon, fgColor, bgColor, bgOpaque) {}

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

void IconScrollTextBox::setIcon(uint8_t* icon)
{
    iconBox.setIcon(icon);
}

//=================================
// Implementation of HorizontalBarBox class
//=================================
HorizontalBarBox::HorizontalBarBox(int16_t pos_x, int16_t pos_y, uint16_t width, uint16_t height, uint16_t fgColor, uint16_t bgColor, bool bgOpaque)
    : isUpdated(true), pos_x(pos_x), pos_y(pos_y), width(width), height(height), fgColor(fgColor), bgColor(bgColor), bgOpaque(bgOpaque), level(0)
{
}

void HorizontalBarBox::setfgColor(uint16_t fgColor)
{
    if (this->fgColor == fgColor) { return; }
    this->fgColor = fgColor;
    update();
}

void HorizontalBarBox::setBgColor(uint16_t bgColor)
{
    if (this->bgColor == bgColor) { return; }
    this->bgColor = bgColor;
    update();
}

void HorizontalBarBox::update()
{
    isUpdated = true;
}

void HorizontalBarBox::draw()
{
    if (!isUpdated) { return; }
    isUpdated = false;
    uint16_t w0 = (uint16_t) ((float) width * level);
    uint16_t h0 = height;
    if (w0 > 0) {
        LCD_Fill(pos_x, pos_y, pos_x+w0-1, pos_y+h0-1, fgColor);
    }
    if (w0 < width) {
        LCD_FillBackground(pos_x+w0, pos_y, pos_x+width-1, pos_y+h0-1, !bgOpaque, bgColor);
    }
}

void HorizontalBarBox::clear()
{
    LCD_FillBackground(pos_x, pos_y, pos_x+width-1, pos_y+height-1, !bgOpaque, bgColor);
}

void HorizontalBarBox::setLevel(float value) // 0.0 ~ 1.0
{
    if (this->level == value) { return; }
    if (value < 0.0) { value = 0.0; }
    if (value > 1.0) { value = 1.0; }
    this->level = value;
    update();
}

float HorizontalBarBox::getLevel()
{
    return level;
}