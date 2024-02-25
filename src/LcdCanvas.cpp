/*-----------------------------------------------------------/
/ LcdCanvas
/------------------------------------------------------------/
/ Copyright (c) 2020-2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#include <cstdio>
#include <cstring>
#include "LcdCanvas.h"
#include "ImageFitter.h"
#include "my_lcd.h"
//#include "fonts/iconfont.h"

//=================================
// Implementation of BatteryIconBox class
//=================================
BatteryIconBox::BatteryIconBox(int16_t pos_x, int16_t pos_y, uint16_t fgColor, uint16_t bgColor)
    : IconBox(pos_x, pos_y, ICON16x16_BATTERY, fgColor, bgColor), level(0) {}

void BatteryIconBox::draw()
{
    if (!isUpdated) { return; }
    isUpdated = false;
    clear();
    LCD_ShowIcon(pos_x, pos_y, icon, !bgOpaque, fgColor);
    uint16_t color = (level >= 50) ? 0x0600 : (level >= 20) ? 0xc600 : 0xc000;
    if (level/10 < 9) {
        LCD_Fill(pos_x+4, pos_y+4, pos_x+4+8-1, pos_y+4+10-level/10-1-1, bgColor);
    }
    LCD_Fill(pos_x+4, pos_y+13-level/10, pos_x+4+8-1, pos_y+13-level/10+level/10+1-1, color);
}

void BatteryIconBox::setLevel(uint8_t value)
{
    value = (value <= 100) ? value : 100;
    if (this->level == value) { return; }
    this->level = value;
    update();
}

//=================================
// Implementation of LcdCanvas class
//=================================
LcdCanvas LcdCanvas::_instance; // Singleton

LcdCanvas& LcdCanvas::instance()
{
    return _instance;
}

LcdCanvas::LcdCanvas()
{
#if defined(USE_ST7735S_160x80)
    LCD_Init();
    LCD_Clear(BLACK);
    BACK_COLOR=BLACK;
#endif

    // ListView parts (nothing to set here)

    // Play parts (nothing to set here)
}

LcdCanvas::~LcdCanvas()
{
}

void LcdCanvas::switchToOpening()
{
    clear(true);
    msg.setText("");
    for (int i = 0; i < (int) (sizeof(groupOpening)/sizeof(*groupOpening)); i++) {
        groupOpening[i]->update();
    }
}

void LcdCanvas::switchToListView()
{
    clear(true);
    msg.setText("");
    battery.setBgOpaque(true);
    for (int i = 0; i < (int) (sizeof(groupListView)/sizeof(*groupListView)); i++) {
        groupListView[i]->update();
    }
}

void LcdCanvas::switchToPlay()
{
    clear(false);
    msg.setText("");
    battery.setBgOpaque(false);
    for (int i = 0; i < (int) (sizeof(groupPlay)/sizeof(*groupPlay)); i++) {
        groupPlay[i]->update();
    }
    for (int i = 0; i < (int) (sizeof(groupPlay0)/sizeof(*groupPlay0)); i++) {
        groupPlay0[i]->update();
    }
    for (int i = 0; i < (int) (sizeof(groupPlay1)/sizeof(*groupPlay1)); i++) {
        groupPlay1[i]->update();
    }
    play_count = 0;
}

void LcdCanvas::switchToPowerOff()
{
    clear(true);
    for (int i = 0; i < (int) (sizeof(groupPowerOff)/sizeof(*groupPowerOff)); i++) {
        groupPowerOff[i]->update();
    }
}

void LcdCanvas::clear(bool bgOpaque)
{
    LCD_FillBackground(0, 0, LCD_W()-1, LCD_H()-1, !bgOpaque, LCD_BLACK);
}

void LcdCanvas::setRotation(uint8_t rot)
{
    LCD_SetRotation(rot);
}

void LcdCanvas::drawOpening()
{
    for (int i = 0; i < (int) (sizeof(groupOpening)/sizeof(*groupOpening)); i++) {
        groupOpening[i]->draw();
    }
}

void LcdCanvas::drawListView()
{
    for (int i = 0; i < (int) (sizeof(groupListView)/sizeof(*groupListView)); i++) {
        groupListView[i]->draw();
    }
}

void LcdCanvas::drawPlay()
{
    for (int i = 0; i < (int) (sizeof(groupPlay)/sizeof(*groupPlay)); i++) {
        groupPlay[i]->draw();
    }
    if (play_count % play_cycle < play_change/* || albumArt.getCount() == 0*/) { // Play mode 0 display
        for (int i = 0; i < (int) (sizeof(groupPlay0)/sizeof(*groupPlay0)); i++) {
            groupPlay0[i]->draw();
        }
        /*
        if (albumArt.getCount() == 0) {
            msg.setText("No Image");
        } else if (play_count == 0 && albumArt.getCount() > 0) {
            if (albumArt.loadNext()) {
                msg.setText("");
            } else {
                msg.setText("Not Supported Image");
            }
            #ifdef USE_ALBUM_ART_SMALL
            if (albumArtSmall.loadNext()) {
                msg.setText("");
            } else {
                msg.setText("Not Supported Image");
            }
            #endif // #ifdef USE_ALBUM_ART_SMALL
        } else *///if (play_count % play_cycle == play_change-1 && albumArt.getCount() > 0) { // Play mode 0 -> 1
        if (play_count % play_cycle == play_change-1) {
            clear(false);
            for (int i = 0; i < (int) (sizeof(groupPlay)/sizeof(*groupPlay)); i++) {
                groupPlay[i]->update();
            }
            /*
            for (int i = 0; i < (int) (sizeof(groupPlay0)/sizeof(*groupPlay0)); i++) {
                groupPlay0[i]->clear();
            }
            */
            for (int i = 0; i < (int) (sizeof(groupPlay1)/sizeof(*groupPlay1)); i++) {
                groupPlay1[i]->update();
            }
            /*
            if (albumArt.getCount() > 1) {
                albumArt.clear();
                if (albumArt.loadNext()) {
                    msg.setText("");
                } else {
                    msg.setText("Not Supported Image");
                }
            }
            */
        }
    } else { // Play mode 1 display
        for (int i = 0; i < (int) (sizeof(groupPlay1)/sizeof(*groupPlay1)); i++) {
            groupPlay1[i]->draw();
        }
        if (play_count % play_cycle == play_cycle-1) { // Play mode 1 -> 0
            clear(false);
            for (int i = 0; i < (int) (sizeof(groupPlay)/sizeof(*groupPlay)); i++) {
                groupPlay[i]->update();
            }
            /*
            for (int i = 0; i < (int) (sizeof(groupPlay1)/sizeof(*groupPlay1)); i++) {
                groupPlay1[i]->clear();
            }
            */
            for (int i = 0; i < (int) (sizeof(groupPlay0)/sizeof(*groupPlay0)); i++) {
                groupPlay0[i]->update();
            }
        }
    }
    play_count++;
}

void LcdCanvas::drawPowerOff()
{
    for (int i = 0; i < (int) (sizeof(groupPowerOff)/sizeof(*groupPowerOff)); i++) {
        groupPowerOff[i]->draw();
    }
}

void LcdCanvas::setImageJpeg(const char *filename)
{
    uint16_t *img_ptr;
    uint16_t w, h;
    image.getImagePtr(&img_ptr, &w, &h);
    imgFit.config(img_ptr, w, h);
    imgFit.loadJpegFile(filename);
    imgFit.getSizeAfterFit(&w, &h);
    image.setImageSize(w, h);
    image.update();
}

void LcdCanvas::setMsg(const char *str, bool blink)
{
    msg.setText(str);
    msg.setBlink(blink);
}

void LcdCanvas::setListItem(int column, const char *str, uint8_t icon, bool isFocused)
{
    uint16_t color[2] = {LCD_GRAY, LCD_GBLUE};
    listItem[column].setIcon(icon);
    listItem[column].setFgColor(color[isFocused]);
    listItem[column].setText(str);
    listItem[column].setScroll(isFocused); // Scroll for focused item only
}

void LcdCanvas::setVolume(uint8_t value)
{
    volume.setFormatText("%3d", (int) value);
}

void LcdCanvas::setAudioLevel(float levelL, float levelR)
{
    levelMeterL.setLevel(levelL);
    levelMeterR.setLevel(levelR);
}

void LcdCanvas::setPlayTime(uint32_t positionSec, uint32_t lengthSec, bool blink)
{
    //playTime.setFormatText("%lu:%02lu / %lu:%02lu", positionSec/60, positionSec%60, lengthSec/60, lengthSec%60);
    playTime.setFormatText("%lu:%02lu", positionSec/60, positionSec%60);
    playTime.setBlink(blink);
    timeProgress.setLevel((float) positionSec/lengthSec);
}

void LcdCanvas::setTrack(const char *str)
{
    track.setText(str);
}

void LcdCanvas::setTitle(const char *str)
{
    title.setText(str);
}

void LcdCanvas::setAlbum(const char *str)
{
    album.setText(str);
}

void LcdCanvas::setArtist(const char *str)
{
    artist.setText(str);
}

uint16_t LcdCanvas::getTiledImage(uint16_t x, uint16_t y)
{
    return image.getPixel(x, y, true);
}

/*
void LcdCanvas::setYear(const char *str)
{
    year.setText(str);
}
*/

void LcdCanvas::setBatteryVoltage(uint16_t voltage_x1000)
{
    const uint16_t lvl100 = 4100;
    const uint16_t lvl0 = 2900;
    battery.setLevel(((voltage_x1000 - lvl0) * 100) / (lvl100 - lvl0));
}
