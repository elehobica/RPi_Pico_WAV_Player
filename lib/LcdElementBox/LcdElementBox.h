/*-----------------------------------------------------------/
/ LcdElementBox: Lcd Element Box API
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#ifndef __LCDELEMENTBOX_H_INCLUDED__
#define __LCDELEMENTBOX_H_INCLUDED__

#include "lcd_extra.h"

//#define DEBUG_LCD_ELEMENT_BOX

// Colors for LCD
#define LCD_WHITE         0xFFFF
#define LCD_BLACK         0x0000
#define LCD_RED           0xF800
#define LCD_GREEN         0x07E0
#define LCD_BLUE          0x001F
#define LCD_BRED          0xF81F
#define LCD_GRED          0xFFE0
#define LCD_GBLUE         0x07FF
#define LCD_BROWN         0xBC40
#define LCD_BRRED         0xFC07
#define LCD_GRAY          0x8430
#define LCD_LIGHTBLUE     0x7D7C
#define LCD_LIGHTGREEN    0x841F
#define LCD_GRAYBLUE      0x5458
#define LCD_DARKGRAY      0x4208

//=================================
// Definition of LcdElementBox interface
//=================================
class LcdElementBox {
public:
	typedef enum _align_enm {
		AlignLeft = 0,
		AlignRight,
		AlignCenter
	} align_enm;
	//virtual ~LcdElementBox() {}
	virtual void setBgColor(uint16_t bgColor) = 0;
	virtual void update() = 0;
	virtual void draw() = 0;
	virtual void clear() = 0;
};

//=================================
// Definition of ImageBox class < LcdElementBox
//=================================
class ImageBox : public LcdElementBox
{
public:
	typedef enum _align_t {
		origin = 0, // LeftTop
		center
	} align_t;
	ImageBox(int16_t pos_x, int16_t pos_y, uint16_t width, uint16_t height, uint16_t bgColor = LCD_BLACK);
	void setBgColor(uint16_t bgColor);
	void update();
	void draw();
	void clear();
	void getImagePtr(uint16_t **img_ptr, uint16_t *width, uint16_t *height);
	void setImageSize(int16_t img_w, int16_t img_h);
	void setPixel(int16_t x, int16_t y, uint16_t rgb565);
	void clearBuf();
	uint16_t getPixel(uint16_t x, uint16_t y, bool tiled = false);
protected:
	bool isUpdated;
	int16_t pos_x, pos_y;
	uint16_t width, height; // ImageBox dimension
	uint16_t bgColor;
	uint16_t *image;
	uint16_t img_w, img_h; // dimention of image stored
	align_t align;
};

//=================================
// Definition of IconBox class < LcdElementBox
//=================================
class IconBox : public LcdElementBox
{
public:
	IconBox(int16_t pos_x, int16_t pos_y, uint16_t fgColor = LCD_WHITE, uint16_t bgColor = LCD_BLACK, bool bgOpaque = false);
	IconBox(int16_t pos_x, int16_t pos_y, uint8_t icon, uint16_t fgColor = LCD_WHITE, uint16_t bgColor = LCD_BLACK, bool bgOpaque = false);
	void setFgColor(uint16_t fgColor);
	void setBgColor(uint16_t bgColor);
	void setBgOpaque(bool bgOpaque);
	void update();
	void draw();
	void clear();
	void setIcon(uint8_t icon);
	static const int iconWidth = 16;
	static const int iconHeight = 16;
protected:
	bool isUpdated;
	int16_t pos_x, pos_y;
	uint16_t fgColor;
	uint16_t bgColor;
	uint8_t icon;
	bool bgOpaque;
};

//=================================
// Definition of TextBox class < LcdElementBox
//=================================
class TextBox : public LcdElementBox
{
public:
	TextBox(int16_t pos_x, int16_t pos_y, uint16_t fgColor = LCD_WHITE, uint16_t bgColor = LCD_BLACK, bool bgOpaque = false);
	TextBox(int16_t pos_x, int16_t pos_y, align_enm align, uint16_t fgColor = LCD_WHITE, uint16_t bgColor = LCD_BLACK, bool bgOpaque = false);
	TextBox(int16_t pos_x, int16_t pos_y, const char *str, align_enm align, uint16_t fgColor = LCD_WHITE, uint16_t bgColor = LCD_BLACK, bool bgOpaque = false);
	void setFgColor(uint16_t fgColor);
	void setBgColor(uint16_t bgColor);
	void update();
	void draw();
	void clear();
	virtual void setText(const char *str);
	void setFormatText(const char *fmt, ...);
	void setInt(int value);
	void setBlink(bool blink);
	static const int charSize = 256;
protected:
	static const int BlinkInterval = 20;
	bool isUpdated;
	int16_t pos_x, pos_y;
	uint16_t fgColor;
	uint16_t bgColor;
	int16_t x0, y0; // previous rectangle origin
	uint16_t w0, h0; // previous rectangle size
	align_enm align;
	bool bgOpaque;
	uint32_t drawCount;
	bool blink;
	char str[charSize];
};

//=================================
// Definition of IconTextBox class < TextBox
//=================================
class IconTextBox : public TextBox
{
public:
	IconTextBox(int16_t pos_x, int16_t pos_y, uint8_t icon, uint16_t fgColor = LCD_WHITE, uint16_t bgColor = LCD_BLACK, bool bgOpaque = false);
	void setFgColor(uint16_t fgColor);
	void setBgColor(uint16_t bgColor);
	void update();
	void draw();
	void clear();
	void setIcon(uint8_t icon);
protected:
	IconBox iconBox;
};

//=================================
// Definition of ScrollTextBox class < LcdElementBox
//=================================
class ScrollTextBox : public LcdElementBox
{
public:
	ScrollTextBox(int16_t pos_x, int16_t pos_y, uint16_t width, uint16_t height, uint16_t fgColor = LCD_WHITE, uint16_t bgColor = LCD_BLACK, bool bgOpaque = false);
	virtual ~ScrollTextBox();
	void setFgColor(uint16_t fgColor);
	void setBgColor(uint16_t bgColor);
	void update();
	void draw();
	void clear();
	void setScroll(bool scr_en);
	virtual void setText(const char *str);
	static const int charSize = 256;
protected:
	bool isUpdated;
	int16_t pos_x, pos_y;
	uint16_t fgColor;
	uint16_t bgColor;
	char str[charSize];
	uint16_t width;
	uint16_t height;
	bool bgOpaque;
	uint16_t sft_val;
	uint32_t count;
	bool scr_en;
};

//=================================
// Definition of IconScrollTextBox class < ScrollTextBox
//=================================
class IconScrollTextBox : public ScrollTextBox
{
public:
	IconScrollTextBox(int16_t pos_x, int16_t pos_y, uint8_t icon, uint16_t width, uint16_t height, uint16_t fgColor = LCD_WHITE, uint16_t bgColor = LCD_BLACK, bool bgOpaque = false);
	void setFgColor(uint16_t fgColor);
	void setBgColor(uint16_t bgColor);
	void update();
	void draw();
	void clear();
	void setIcon(uint8_t icon);
protected:
	IconBox iconBox;
};

//=================================
// Definition of HorizontalBarBox class < LcdElementBox
//=================================
class HorizontalBarBox : public LcdElementBox
{
public:
	HorizontalBarBox(int16_t pos_x, int16_t pos_y, uint16_t width, uint16_t height, uint16_t fgColor = LCD_WHITE, uint16_t bgColor = LCD_BLACK, bool bgOpaque = false);
	void setfgColor(uint16_t fgColor);
	void setBgColor(uint16_t bgColor);
	void update();
	void draw();
	void clear();
	void setLevel(float value);
	float getLevel();
protected:
	bool isUpdated;
	int16_t pos_x, pos_y;
	uint16_t width, height;
	uint16_t fgColor;
	uint16_t bgColor;
	bool bgOpaque;
	float level;
};


#endif // __LCDELEMENTBOX_H_INCLUDED__
