/*-----------------------------------------------------------/
/ LcdCanvas
/------------------------------------------------------------/
/ Copyright (c) 2020-2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#pragma once

#include "LcdElementBox.h"
#include "LcdCanvasIconDef.h"
#include "ui_control.h"

// Select LCD device
#define USE_ST7735S_160x80

#if defined(USE_ST7735S_160x80)
#include "lcd_extra.h" // Hardware-specific library for ST7735
#endif

#define FONT_HEIGHT     16

uint8_t* ICON2PTR(IconIndex_t index);

//=================================
// Definition of BatteryIconBox class < IconBox
//=================================
class BatteryIconBox : public IconBox
{
public:
    BatteryIconBox(int16_t pos_x, int16_t pos_y, uint16_t fgColor = LCD_WHITE, uint16_t bgColor = LCD_BLACK);
    void draw();
    void setLevel(const uint8_t& value);
    void setCharging();
protected:
    bool isCharging;
    uint8_t level;
};

//=================================
// Definition of LcdCanvas Class
//=================================
class LcdCanvas
{
public:
    static void configureLcd(board_type_t board_type, uint cfg_id);
    static LcdCanvas& instance();  // Singleton, however dynamic generation
    void clear(bool bgOpaque = false);
    void setRotation(uint8_t rot);
    void setImageJpeg(const char* filename, const uint64_t pos = 0, const size_t size = 0);
    void resetImage();
    void setMsg(const char* str, bool blink = false);
    void setListItem(int column, const char* str, const IconIndex_t index = IconIndex_t::UNDEF, bool isFocused = false);
    void setVolume(uint8_t value);
    void setAudioLevel(float levelL, float levelR);
    void setBitRes(uint16_t value);
    void setSampleFreq(uint32_t sampFreq);
    void setPlayTime(uint32_t posionSec, uint32_t lengthSec, bool blink = false);
    void setTrack(const char* str);
    void setTitle(const char* str);
    void setAlbum(const char* str);
    void setArtist(const char* str);
    //void setYear(const char* str);
    void setBatteryVoltage(const float& voltage);
    void switchToOpening();
    void switchToListView();
    void switchToPlay();
    void switchToPowerOff();
    void drawOpening();
    void drawListView();
    void drawPlay();
    void drawPowerOff();
    uint16_t getTiledImage(uint16_t x, uint16_t y);

protected:
    int play_count;
    const int play_cycle = 400;
    const int play_change = 350;
    uint8_t bitSampIcon[32] = {};
#if defined(USE_ST7735S_160x80)
    IconScrollTextBox listItem[5] = {
        IconScrollTextBox(16*0, 16*0, nullptr, LCD_W(), FONT_HEIGHT, LCD_GRAY, LCD_BLACK, true),
        IconScrollTextBox(16*0, 16*1, nullptr, LCD_W(), FONT_HEIGHT, LCD_GRAY, LCD_BLACK, true),
        IconScrollTextBox(16*0, 16*2, nullptr, LCD_W(), FONT_HEIGHT, LCD_GRAY, LCD_BLACK, true),
        IconScrollTextBox(16*0, 16*3, nullptr, LCD_W(), FONT_HEIGHT, LCD_GRAY, LCD_BLACK, true),
        IconScrollTextBox(16*0, 16*4, nullptr, LCD_W()-16*1, FONT_HEIGHT, LCD_GRAY, LCD_BLACK, true) // Battery Icon at Right-Bottom
    };
    ImageBox image = ImageBox(0, 0, LCD_W(), LCD_H());
    BatteryIconBox battery = BatteryIconBox(LCD_W()-16*1, LCD_H()-16*1, LCD_GRAY);
    IconTextBox volume = IconTextBox(LCD_W()-16*4, LCD_H()-16*1, ICON2PTR(IconIndex_t::VOLUME), LCD_GRAY);
    TextBox playTime = TextBox(LCD_W()-16*4-8, LCD_H()-16*1, LcdElementBox::AlignRight, LCD_GRAY, LCD_BLACK);
    IconScrollTextBox title = IconScrollTextBox(16*0, 16*0, ICON2PTR(IconIndex_t::TITLE), LCD_W(), FONT_HEIGHT, LCD_LIGHTBLUE, LCD_BLACK);
    IconScrollTextBox artist = IconScrollTextBox(16*0, 16*1, ICON2PTR(IconIndex_t::ARTIST), LCD_W(), FONT_HEIGHT, LCD_LIGHTGREEN, LCD_BLACK);
    IconScrollTextBox album = IconScrollTextBox(16*0, 16*2, ICON2PTR(IconIndex_t::ALBUM), LCD_W(), FONT_HEIGHT, LCD_GRAYBLUE, LCD_BLACK);
    HorizontalBarBox levelMeterL = HorizontalBarBox(16*0, 16*3, LCD_W()-16, 4, LCD_DARKGRAY);
    HorizontalBarBox levelMeterR = HorizontalBarBox(16*0, 16*3+8, LCD_W()-16, 4, LCD_DARKGRAY);
    IconBox bitSamp = IconBox(LCD_W()-16, 16*3, ICON2PTR(IconIndex_t::UNDEF), LCD_GRAY);
    HorizontalBarBox timeProgress = HorizontalBarBox(16*0, 16*4-1, LCD_W(), 1, LCD_BLUE, LCD_DARKGRAY, true);
    TextBox track = TextBox(16*0, LCD_H()-16*1, LcdElementBox::AlignLeft, LCD_GRAY);
    TextBox msg = TextBox(LCD_W()/2, LCD_H()/2-FONT_HEIGHT/2, LcdElementBox::AlignCenter, LCD_WHITE, LCD_BLACK, true);
    LcdElementBox* groupOpening[2] = {&image, &msg};
    LcdElementBox* groupListView[6] = {
        &listItem[0], &listItem[1], &listItem[2], &listItem[3], &listItem[4], &battery
    };
    LcdElementBox* groupPlay[1] = {&battery}; // Common for Play mode 0 and 1
    LcdElementBox* groupPlay0[10] = {&title, &artist, &album, &levelMeterL, &levelMeterR, &bitSamp, &timeProgress, &track, &playTime, &volume}; // Play mode 0 only
    LcdElementBox* groupPlay1[2] = {&image, &msg}; // Play mode 1 only
    LcdElementBox* groupPowerOff[1] = {&msg};

    // Singleton
    LcdCanvas() = default;
    virtual ~LcdCanvas() = default;
    LcdCanvas(const LcdCanvas&) = delete;
    LcdCanvas& operator=(const LcdCanvas&) = delete;
#endif // USE_ST7735S_160x80
};

