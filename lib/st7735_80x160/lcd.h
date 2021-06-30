#ifndef __LCD_H
#define __LCD_H		

#include "pico.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/clocks.h"

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;
typedef char  i8;
typedef short i16;
typedef long  i32;

// SPI Setting
#define SPI_INST        spi1
#define SPI_CLK_FREQ    (30 * MHZ)

// Pin Setting
#define PIN_LCD_CS      13
#define PIN_LCD_SCK     10
#define PIN_LCD_MOSI    11
#define PIN_LCD_DC      14
#define PIN_LCD_RST     15
#define PIN_LCD_BLK     9


#define RGB_ORDER 1 // 0: RGB, 1: BGR
#define HAS_BLK_CNTL    0

#define LCD_WIDTH 160
#define LCD_HEIGHT 80

//-----------------OLED端口定义---------------- 
#ifdef __cplusplus
extern "C" {
#endif

#define OLED_SCLK_Clr() 
#define OLED_SCLK_Set() 

#define OLED_SDIN_Clr()
#define OLED_SDIN_Set()

#define OLED_CS_Clr() gpio_put(PIN_LCD_CS, 0)
#define OLED_CS_Set() gpio_put(PIN_LCD_CS, 1)

#define OLED_RST_Clr() gpio_put(PIN_LCD_RST, 0)
#define OLED_RST_Set() gpio_put(PIN_LCD_RST, 1)

#define OLED_DC_Clr() gpio_put(PIN_LCD_DC, 0)
#define OLED_DC_Set() gpio_put(PIN_LCD_DC, 1)


#if     HAS_BLK_CNTL
#define OLED_BLK_Clr() gpio_put(PIN_LCD_BLK, 0)
#define OLED_BLK_Set() gpio_put(PIN_LCD_BLK, 1)
#else
#define OLED_BLK_Clr()
#define OLED_BLK_Set()
#endif

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

extern  u16 BACK_COLOR;   //背景色

void LCD_Writ_Bus(u8 dat);
void LCD_WR_DATA8(u8 dat);
void LCD_WR_DATA(u16 dat);
void LCD_WR_REG(u8 dat);
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);
void LCD_Init(void);
void LCD_Clear(u16 Color);
void LCD_SetRotation(u8 rot);
u16 LCD_W();
u16 LCD_H();
void LCD_ShowChinese(u16 x,u16 y,u8 index,u8 size,u16 color);
void LCD_DrawPoint(u16 x,u16 y,u16 color);
void LCD_DrawPoint_big(u16 x,u16 y,u16 color);
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color);
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color);
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color);
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 mode,u16 color);
void LCD_ShowString(u16 x,u16 y,const u8 *p,u16 color);
u32 mypow(u8 m,u8 n);
void LCD_ShowNum(u16 x,u16 y,u16 num,u8 len,u16 color);
void LCD_ShowNum1(u16 x,u16 y,float num,u8 len,u16 color);
void LCD_ShowPicture(u16 x1, u16 y1, u16 x2, u16 y2, u8* image);


//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

#define DARKGRAY         0x4208
#define DARKGREEN        0x03E0

#define DARKRED          0x7800

#ifdef __cplusplus
}
#endif

#endif
