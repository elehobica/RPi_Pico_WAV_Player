#include "my_lcd.h"
#include "iconfont.h"

extern const u8 asc2_1608[1520];
extern unsigned char *image; // 80*80*2

// Show 16x16 ICON
// mode: 0: non-overlay, 1: overlay
void LCD_ShowIcon(u16 x,u16 y,u8 index,u8 mode,u16 color)
{
    u8 pos,t;
	u8 *temp,size1;
	u8 size = 16;
	temp=Icon16;
	LCD_Address_Set(x,y,x+size-1,y+size-1); //设置一个汉字的区域
	size1=size*size/8;//一个汉字所占的字节
	temp+=index*size1;//写入的起始位置
	if(!mode)
	{
		for(pos=0;pos<size1;pos++)
		{
			for(t=0;t<8;t++)
			{
				if((*temp&(1<<t))!=0)//从数据的低位开始读
				{
					LCD_WR_DATA(color);//点亮
				}
				else
				{
					LCD_WR_DATA(BACK_COLOR);//不点亮
				}
			}
			temp++;
		}
	}
	else
	{
		for(pos=0;pos<size1;pos++)
		{
			for(t=0;t<8;t++)
			{
				if((*temp&(1<<t))!=0)//从数据的低位开始读
				{
					LCD_DrawPoint(x+(pos%2)*8+t,y+pos/2,color);
				}
			}
			temp++;
		}
	}
}

// Show Partial Charactor within rectanble (x_min, y_min)-(x_max, y_max) from the point (x, y)
// num: char code
// mode: 0: non-overlay, 1: overlay
// color: Color
void LCD_ShowPartialChar(int16_t x,int16_t y,u16 x_min,u16 x_max,u16 y_min,u16 y_max,u8 num,u8 mode,u16 color)
{
    u8 temp;
    u8 pos,t;
	int16_t x0=x;
	if(x<-8+1||y<-16+1)return;
    if(x>LCD_W-1||y>LCD_H-1)return;
	num=num-' ';
	x_min = ((int32_t) x >= (int32_t) x_min) ? (u16) x : x_min;
	y_min = ((int32_t) y >= (int32_t) y_min) ? (u16) y : y_min;
	x_max = ((int32_t) x+8-1 <= (int32_t) x_max) ? (u16) (x+8-1) : x_max;
	y_max = ((int32_t) y+16-1 <= (int32_t) y_max) ? (u16) (y+16-1) : y_max;
	LCD_Address_Set(x_min,y_min,x_max,y_max);
	if(!mode)
	{
		for(pos=0;pos<16;pos++)
		{
			temp=asc2_1608[(u16)num*16+pos];
			for(t=0;t<8;t++)
		    {
				if ((int32_t) x >= (int32_t) x_min && (int32_t) x <= (int32_t) x_max &&
					(int32_t) y >= (int32_t) y_min && (int32_t) y <= (int32_t) y_max) {
					if(temp&0x01)LCD_WR_DATA(color);
					else LCD_WR_DATA(BACK_COLOR);
				}
				temp>>=1;
				x++;
		    }
			x=x0;
			y++;
		}
	}else
	{
		for(pos=0;pos<16;pos++)
		{
		    temp=asc2_1608[(u16)num*16+pos];
			for(t=0;t<8;t++)
		    {
				if ((int32_t) x >= (int32_t) x_min && (int32_t) x <= (int32_t) x_max &&
					(int32_t) y >= (int32_t) y_min && (int32_t) y <= (int32_t) y_max) {
			        if(temp&0x01)LCD_DrawPoint(x+t,y+pos,color);
				}
		        temp>>=1;
		    }
		}
	}
}

// Show String within one line from x_min to x_max
// return: 0: column not overflow, column overflowed
u16 LCD_ShowStringLn(int16_t x,int16_t y,u16 x_min,u16 x_max,const u8 *p,u16 color)
{
	u16 res = 0;
    while(*p!='\0')
    {
        LCD_ShowPartialChar(x,y,x_min,x_max,0,LCD_H-1,*p,0,color);
        if((int32_t) x > (int32_t) x_max-7){res=1;break;}
        x+=8;
        p++;
    }
	return res;
}

// Show String within one line from x_min to x_max with Overlay
// return: 0: column not overflow, column overflew
u16 LCD_ShowStringLnOL(int16_t x,int16_t y,u16 x_min,u16 x_max,const u8 *p,u16 color)
{
	u16 res = 0;
    while(*p!='\0')
    {
        LCD_ShowPartialChar(x,y,x_min,x_max,0,LCD_H-1,*p,1,color);
		if((int32_t) x >(int32_t) x_max-7){res=1;break;}
        x+=8;
        p++;
    }
	return res;
}

// Show String within one line from x_min to x_max with Auto-Scroll
// When string is within one line, write in overlay (Non-scroll)
// Otherwise, write in non-overlay (Scroll)
void LCD_Scroll_ShowString(u16 x, u16 y, u16 x_min, u16 x_max, u8 *p, u16 color, uint16_t *sft_val, uint32_t tick)
{
    if (*sft_val == 0) { // Head display
        if (LCD_ShowStringLnOL(x,  y, x_min, x_max, (u8 *) p, color) && (tick % 16 == 0)) {
            LCD_ShowStringLn(x,  y, x_min, x_max, (u8 *) p, color);
            (*sft_val)++;
        }
    } else {
        if (((x + *sft_val)%8) != 0) {
            // delete head & tail  gabage
            LCD_ShowString(x, y, (u8 *) " ", color);
            LCD_ShowString(LCD_W-8, y, (u8 *) " ", color);
        }
		if (LCD_ShowStringLn((int16_t) x - (*sft_val)%8, y, x_min, x_max, (u8 *) &p[(*sft_val)/8], color)) {
            (*sft_val)++;
        } else if (tick % 16 == 7) { // Tail display returns back to Head display
            LCD_ShowStringLn(x,  y, x_min, x_max, (u8 *) p, color);
            *sft_val = 0;
        }
    }
}

// dim: 0(dark) ~ 255(original)
void LCD_ShowDimPicture(u16 x1, u16 y1, u16 x2, u16 y2, u8 dim)
{
	LCD_ShowDimPictureOfs(x1, y1, x2, y2, dim, 0, 0);
#if 0
	int i;
	int j;
	LCD_Address_Set(x1,y1,x2,y2);
	u16 val;
	u16 r, g, b;
	for (i=0; i < (x2-x1+1)*(y2-y1+1)*2; i+=2) {
		for (j=0; j < 8; j++) {
			if (i >= 80*j*10*2 && i < 80*(j+1)*10*2) {
				val = ((u16) image[j][i-80*j*2] << 8) | ((u16) image[j][i+1-80*j*2]);
				break;
			}
		}
		r = (u16) ((((u32) val & 0xf800) * dim / 255) & 0xf800);
		g = (u16) ((((u32) val & 0x07e0) * dim / 255) & 0x07e0);
		b = (u16) ((((u32) val & 0x001f) * dim / 255) & 0x001f);
		val = r | g | b;
		/*
		r = ((val >> 11) & 0x1f) * dim / 255;
		g = ((val >> 5) & 0x3f)  * dim / 255;
		b = (val & 0x1f) * dim / 255;
		val = ((r&0x1f)<<11) | ((g&0x3f)<<5) | (b&0x1f);
		*/
		LCD_WR_DATA8((val >> 8)&0xff);
		LCD_WR_DATA8(val & 0xff);
	}
#endif
}


// dim: 0(dark) ~ 255(original)
void LCD_ShowDimPictureOfs(u16 x1, u16 y1, u16 x2, u16 y2, u8 dim, u16 ofs_x, u16 ofs_y)
{
	int i;
	int j;
	int x, y;
	LCD_Address_Set(x1,y1,x2,y2);
	u16 val;
	u16 r, g, b;
	for (y = y1; y <= y2; y++) {
		i = ((ofs_y + y - y1)*80+ofs_x)*2;
		//j = i/(80*10*2);
		for (x = x1; x <= x2; x++) {
			//val = ((u16) image[j][i-80*j*10*2] << 8) | ((u16) image[j][i-80*j*10*2+1]);
			val = ((u16) image[i] << 8) | ((u16) image[i+1]);
			r = (u16) ((((u32) val & 0xf800) * dim / 255) & 0xf800);
			g = (u16) ((((u32) val & 0x07e0) * dim / 255) & 0x07e0);
			b = (u16) ((((u32) val & 0x001f) * dim / 255) & 0x001f);
			val = r | g | b;
			/*
			r = ((val >> 11) & 0x1f) * dim / 255;
			g = ((val >> 5) & 0x3f)  * dim / 255;
			b = (val & 0x1f) * dim / 255;
			val = ((r&0x1f)<<11) | ((g&0x3f)<<5) | (b&0x1f);
			*/
			LCD_WR_DATA8((val >> 8)&0xff);
			LCD_WR_DATA8(val & 0xff);
			i += 2;
		}
	}
}



