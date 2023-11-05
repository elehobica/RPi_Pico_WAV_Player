#include "lcd.h"
#include "oledfont.h"

static u8 _rotation = 2;  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏
u16 BACK_COLOR;   //背景色

/******************************************************************************
      函数说明：LCD串行数据写入函数
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(u8 dat)
{
    OLED_CS_Clr();

    uint8_t *buff = (uint8_t *) &dat;
    spi_write_blocking(SPI_INST, buff, 1);

    OLED_CS_Set();
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(u8 dat)
{
    OLED_DC_Set();//写数据
    LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(u16 dat)
{
    OLED_DC_Set();//写数据
    LCD_Writ_Bus(dat>>8);
    LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(u8 dat)
{
    OLED_DC_Clr();//写命令
    LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2)
{
    if(_rotation==0)
    {
        LCD_WR_REG(0x2a);//列地址设置
        LCD_WR_DATA(x1+26);
        LCD_WR_DATA(x2+26);
        LCD_WR_REG(0x2b);//行地址设置
        LCD_WR_DATA(y1+1);
        LCD_WR_DATA(y2+1);
        LCD_WR_REG(0x2c);//储存器写
    }
    else if(_rotation==1)
    {
        LCD_WR_REG(0x2a);//列地址设置
        LCD_WR_DATA(x1+26);
        LCD_WR_DATA(x2+26);
        LCD_WR_REG(0x2b);//行地址设置
        LCD_WR_DATA(y1+1);
        LCD_WR_DATA(y2+1);
        LCD_WR_REG(0x2c);//储存器写
    }
    else if(_rotation==2)
    {
        LCD_WR_REG(0x2a);//列地址设置
        LCD_WR_DATA(x1+1);
        LCD_WR_DATA(x2+1);
        LCD_WR_REG(0x2b);//行地址设置
        LCD_WR_DATA(y1+26);
        LCD_WR_DATA(y2+26);
        LCD_WR_REG(0x2c);//储存器写
    }
    else
    {
        LCD_WR_REG(0x2a);//列地址设置
        LCD_WR_DATA(x1+1);
        LCD_WR_DATA(x2+1);
        LCD_WR_REG(0x2b);//行地址设置
        LCD_WR_DATA(y1+26);
        LCD_WR_DATA(y2+26);
        LCD_WR_REG(0x2c);//储存器写
    }
}

/*!
    \brief      configure the SPI peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void spi_config(void)
{
    gpio_init(PIN_LCD_SCK);
    gpio_set_function(PIN_LCD_SCK, GPIO_FUNC_SPI);

    gpio_init(PIN_LCD_MOSI);
    gpio_set_function(PIN_LCD_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_LCD_CS);
    gpio_set_dir(PIN_LCD_CS, GPIO_OUT);

    spi_init(SPI_INST, SPI_CLK_FREQ);

    /* SPI parameter config */
    spi_set_format(SPI_INST,
        8, /* data_bits */
        SPI_CPOL_0, /* cpol */
        SPI_CPHA_0, /* cpha */
        SPI_MSB_FIRST /* order */
    );

    OLED_CS_Set();
}

/******************************************************************************
      函数说明：LCD初始化函数
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_Init(void)
{
    gpio_init(PIN_LCD_DC);
    gpio_set_dir(PIN_LCD_DC, GPIO_OUT);

    gpio_init(PIN_LCD_RST);
    gpio_set_dir(PIN_LCD_RST, GPIO_OUT);

#if HAS_BLK_CNTL
    gpio_init(PIN_LCD_BLK);
    gpio_set_dir(PIN_LCD_BLK, GPIO_OUT);
#endif

    spi_config();

    OLED_RST_Clr();
    sleep_ms(200);
    OLED_RST_Set();
    sleep_ms(20);
    OLED_BLK_Set();

    LCD_WR_REG(0x11);   // turn off sleep mode
    sleep_ms(100);

    LCD_WR_REG(0x21);   // display inversion mode

    LCD_WR_REG(0xB1);   // Set the frame frequency of the full colors normal mode
                        // Frame rate=fosc/((RTNA x 2 + 40) x (LINE + FPA + BPA +2))
                        // fosc = 850kHz
    LCD_WR_DATA8(0x05); // RTNA
    LCD_WR_DATA8(0x3A); // FPA
    LCD_WR_DATA8(0x3A); // BPA

    LCD_WR_REG(0xB2);   // Set the frame frequency of the Idle mode
                        // Frame rate=fosc/((RTNB x 2 + 40) x (LINE + FPB + BPB +2))
                        // fosc = 850kHz
    LCD_WR_DATA8(0x05); // RTNB
    LCD_WR_DATA8(0x3A); // FPB
    LCD_WR_DATA8(0x3A); // BPB

    LCD_WR_REG(0xB3);   // Set the frame frequency of the Partial mode/ full colors
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3A);
    LCD_WR_DATA8(0x3A);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3A);
    LCD_WR_DATA8(0x3A);

    LCD_WR_REG(0xB4);
    LCD_WR_DATA8(0x03);

    LCD_WR_REG(0xC0);
    LCD_WR_DATA8(0x62);
    LCD_WR_DATA8(0x02);
    LCD_WR_DATA8(0x04);

    LCD_WR_REG(0xC1);
    LCD_WR_DATA8(0xC0);

    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x00);

    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x8D);
    LCD_WR_DATA8(0x6A);

    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x8D);
    LCD_WR_DATA8(0xEE);

    LCD_WR_REG(0xC5);  /*VCOM*/
    LCD_WR_DATA8(0x0E);

    LCD_WR_REG(0xE0);
    LCD_WR_DATA8(0x10);
    LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x02);
    LCD_WR_DATA8(0x03);
    LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x02);
    LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x0A);
    LCD_WR_DATA8(0x12);
    LCD_WR_DATA8(0x27);
    LCD_WR_DATA8(0x37);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x10);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA8(0x10);
    LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x03);
    LCD_WR_DATA8(0x03);
    LCD_WR_DATA8(0x0F);
    LCD_WR_DATA8(0x06);
    LCD_WR_DATA8(0x02);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x0A);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x26);
    LCD_WR_DATA8(0x36);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x10);

    LCD_WR_REG(0x3A);   // define the format of RGB picture data
    LCD_WR_DATA8(0x05); // 16-bit/pixel

    LCD_SetRotation(_rotation);
    /*
    // Memory data access control (MADCTL)
    LCD_WR_REG(0x36);
    if(_rotation==0)LCD_WR_DATA8(0x00 | (RGB_ORDER<<3));
    else if(_rotation==1)LCD_WR_DATA8(0xC0 | (RGB_ORDER<<3));
    else if(_rotation==2)LCD_WR_DATA8(0x70 | (RGB_ORDER<<3));
    else LCD_WR_DATA8(0xA0 | (RGB_ORDER<<3));
    */

    LCD_WR_REG(0x29);   // Display On
}

/******************************************************************************
      函数说明：LCD清屏函数
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_Clear(u16 Color)
{
    u16 i,j;
    LCD_Address_Set(0,0,LCD_W()-1,LCD_H()-1);
    for(i=0;i<LCD_W();i++)
    {
        for (j=0;j<LCD_H();j++)
        {
            LCD_WR_DATA(Color);
        }
    }
}

void LCD_SetRotation(u8 rot)
{
    _rotation = rot & 0x3;
    // Memory data access control (MADCTL)
    LCD_WR_REG(0x36);
    if(_rotation==0)LCD_WR_DATA8(0x00 | (RGB_ORDER<<3));
    else if(_rotation==1)LCD_WR_DATA8(0xC0 | (RGB_ORDER<<3));
    else if(_rotation==2)LCD_WR_DATA8(0x70 | (RGB_ORDER<<3));
    else LCD_WR_DATA8(0xA0 | (RGB_ORDER<<3));
}

u16 LCD_W()
{
    if (_rotation == 0 || _rotation == 1) {
        return LCD_HEIGHT;
    } else {
        return LCD_WIDTH;
    }
}

u16 LCD_H()
{
    if (_rotation == 0 || _rotation == 1) {
        return LCD_WIDTH;
    } else {
        return LCD_HEIGHT;
    }
}

#if 0
/******************************************************************************
      函数说明：LCD显示汉字
      入口数据：x,y   起始坐标
                index 汉字的序号
                size  字号
      返回值：  无
******************************************************************************/
void LCD_ShowChinese(u16 x,u16 y,u8 index,u8 size,u16 color)
{
    u8 i,j;
    u8 *temp,size1;
    if(size==16){temp=Hzk16;}//选择字号
    if(size==32){temp=Hzk32;}
  LCD_Address_Set(x,y,x+size-1,y+size-1); //设置一个汉字的区域
  size1=size*size/8;//一个汉字所占的字节
    temp+=index*size1;//写入的起始位置
    for(j=0;j<size1;j++)
    {
        for(i=0;i<8;i++)
        {
            if((*temp&(1<<i))!=0)//从数据的低位开始读
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
#endif


/******************************************************************************
      函数说明：LCD显示汉字
      入口数据：x,y   起始坐标
      返回值：  无
******************************************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
    LCD_Address_Set(x,y,x,y);//设置光标位置
    LCD_WR_DATA(color);
}


/******************************************************************************
      函数说明：LCD画一个大的点
      入口数据：x,y   起始坐标
      返回值：  无
******************************************************************************/
void LCD_DrawPoint_big(u16 x,u16 y,u16 color)
{
    LCD_Fill(x-1,y-1,x+1,y+1,color);
}


/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
      返回值：  无
******************************************************************************/
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{
    u16 i,j;
    LCD_Address_Set(xsta,ysta,xend,yend);      //设置光标位置
    for(i=ysta;i<=yend;i++)
    {
        for(j=xsta;j<=xend;j++)LCD_WR_DATA(color);//设置光标位置
    }
}


/******************************************************************************
      函数说明：画线
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
      返回值：  无
******************************************************************************/
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
    u16 t;
    int xerr=0,yerr=0,delta_x,delta_y,distance;
    int incx,incy,uRow,uCol;
    delta_x=x2-x1; //计算坐标增量
    delta_y=y2-y1;
    uRow=x1;//画线起点坐标
    uCol=y1;
    if(delta_x>0)incx=1; //设置单步方向
    else if (delta_x==0)incx=0;//垂直线
    else {incx=-1;delta_x=-delta_x;}
    if(delta_y>0)incy=1;
    else if (delta_y==0)incy=0;//水平线
    else {incy=-1;delta_y=-delta_y;}
    if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴
    else distance=delta_y;
    for(t=0;t<distance+1;t++)
    {
        LCD_DrawPoint(uRow,uCol,color);//画点
        xerr+=delta_x;
        yerr+=delta_y;
        if(xerr>distance)
        {
            xerr-=distance;
            uRow+=incx;
        }
        if(yerr>distance)
        {
            yerr-=distance;
            uCol+=incy;
        }
    }
}


/******************************************************************************
      函数说明：画矩形
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
      返回值：  无
******************************************************************************/
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
    LCD_DrawLine(x1,y1,x2,y1,color);
    LCD_DrawLine(x1,y1,x1,y2,color);
    LCD_DrawLine(x1,y2,x2,y2,color);
    LCD_DrawLine(x2,y1,x2,y2,color);
}


/******************************************************************************
      函数说明：画圆
      入口数据：x0,y0   圆心坐标
                r       半径
      返回值：  无
******************************************************************************/
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color)
{
    int a,b;
    // int di;
    a=0;b=r;
    while(a<=b)
    {
        LCD_DrawPoint(x0-b,y0-a,color);             //3
        LCD_DrawPoint(x0+b,y0-a,color);             //0
        LCD_DrawPoint(x0-a,y0+b,color);             //1
        LCD_DrawPoint(x0-a,y0-b,color);             //2
        LCD_DrawPoint(x0+b,y0+a,color);             //4
        LCD_DrawPoint(x0+a,y0-b,color);             //5
        LCD_DrawPoint(x0+a,y0+b,color);             //6
        LCD_DrawPoint(x0-b,y0+a,color);             //7
        a++;
        if((a*a+b*b)>(r*r))//判断要画的点是否过远
        {
            b--;
        }
    }
}


/******************************************************************************
      函数说明：显示字符
      入口数据：x,y    起点坐标
                num    要显示的字符
                mode   1叠加方式  0非叠加方式
      返回值：  无
******************************************************************************/
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 mode,u16 color)
{
    u8 temp;
    u8 pos,t;
      u16 x0=x;
    if(x>LCD_W()-8||y>LCD_H()-16)return;        //设置窗口
    num=num-' ';//得到偏移后的值
    LCD_Address_Set(x,y,x+8-1,y+16-1);      //设置光标位置
    if(!mode) //非叠加方式
    {
        for(pos=0;pos<16;pos++)
        {
            temp=asc2_1608[(u16)num*16+pos];         //调用1608字体
            for(t=0;t<8;t++)
            {
                if(temp&0x01)LCD_WR_DATA(color);
                else LCD_WR_DATA(BACK_COLOR);
                temp>>=1;
                x++;
            }
            x=x0;
            y++;
        }
    }else//叠加方式
    {
        for(pos=0;pos<16;pos++)
        {
            temp=asc2_1608[(u16)num*16+pos];         //调用1608字体
            for(t=0;t<8;t++)
            {
                if(temp&0x01)LCD_DrawPoint(x+t,y+pos,color);//画一个点
                temp>>=1;
            }
        }
    }
}


/******************************************************************************
      函数说明：显示字符串
      入口数据：x,y    起点坐标
                *p     字符串起始地址
      返回值：  无
******************************************************************************/
void LCD_ShowString(u16 x,u16 y,const u8 *p,u16 color)
{
    while(*p!='\0')
    {
        if(x>LCD_W()-8){x=0;y+=16;}
        if(y>LCD_H()-16){y=x=0;LCD_Clear(RED);}
        LCD_ShowChar(x,y,*p,0,color);
        x+=8;
        p++;
    }
}


/******************************************************************************
      函数说明：显示数字
      入口数据：m底数，n指数
      返回值：  无
******************************************************************************/
u32 mypow(u8 m,u8 n)
{
    u32 result=1;
    while(n--)result*=m;
    return result;
}


/******************************************************************************
      函数说明：显示数字
      入口数据：x,y    起点坐标
                num    要显示的数字
                len    要显示的数字个数
      返回值：  无
******************************************************************************/
void LCD_ShowNum(u16 x,u16 y,u16 num,u8 len,u16 color)
{
    u8 t,temp;
    u8 enshow=0;
    for(t=0;t<len;t++)
    {
        temp=(num/mypow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                LCD_ShowChar(x+8*t,y,' ',0,color);
                continue;
            }else enshow=1;

        }
        LCD_ShowChar(x+8*t,y,temp+48,0,color);
    }
}


/******************************************************************************
      函数说明：显示小数
      入口数据：x,y    起点坐标
                num    要显示的小数
                len    要显示的数字个数
      返回值：  无
******************************************************************************/
void LCD_ShowNum1(u16 x,u16 y,float num,u8 len,u16 color)
{
    u8 t,temp;
    // u8 enshow=0;
    u16 num1;
    num1=num*100;
    for(t=0;t<len;t++)
    {
        temp=(num1/mypow(10,len-t-1))%10;
        if(t==(len-2))
        {
            LCD_ShowChar(x+8*(len-2),y,'.',0,color);
            t++;
            len+=1;
        }
        LCD_ShowChar(x+8*t,y,temp+48,0,color);
    }
}


/******************************************************************************
      函数说明：显示40x40图片
      入口数据：x,y    起点坐标
      返回值：  无
******************************************************************************/
void LCD_ShowPicture(u16 x1, u16 y1, u16 x2, u16 y2, u8 *image)
{
    int i;
    int j;
    LCD_Address_Set(x1,y1,x2,y2);
    for(i=0;i<(x2-x1+1)*(y2-y1+1)*2;i++)
    {
        LCD_WR_DATA8(image[i]);
    }
}
