/**modify--by JLB****/

#include "TFT18_ST7735S.h"
#include "delay/delay.h"
#include "Font.h"
#include "stm32h7xx_hal_spi.h"
#include "spi.h"
#include "stdlib.h"
#include "stdio.h"
#include <stdarg.h>
#include <string.h>

#if USE_EX_GRAM  //使用外部显存

uint16_t ex_gram[TFT18W * TFT18H] = {0};

#else

#endif

#define USE_QSPI 1  //使用硬件SPI

void TFTSPI_Inversion(uint8_t enable)
{
	if (enable)
	{
		TFTSPI_Write_Cmd(0x21); //启动显示反转
	}
	else
	{
		TFTSPI_Write_Cmd(0x20); //关闭显示反转
	}
}

/*!
 * @brief    TFT18初始化
 *
 * @param    type ： 0:横屏  1：竖屏
 *
 * @return   无
 *
 * @note     如果修改管脚 需要修改初始化的管脚
 *
 * @see      TFTSPI_Init(1);
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Init(void)
{
	//-----端口初始化----//
	delay_ms(120);
	// TFTSPI_RST_L
	//
	// delay_ms(20);
	// TFTSPI_RST_H
	TFTSPI_Write_Cmd(0x01); //软件复位
	delay_ms(120);

	TFTSPI_Write_Cmd(0x11); //关闭睡眠
	delay_ms(10);
	TFTSPI_Write_Cmd(0x3a); //设置像素格式
	TFTSPI_Write_Byte(0x05); //每次传送16位数据(VIPF3-0=0101)，每个像素16位(IFPF2-0=101)
	// TFTSPI_Write_Cmd(0x26); //设置伽马曲线
	// TFTSPI_Write_Byte(0x04); //伽马曲线3
	// TFTSPI_Write_Cmd(0xf2);                   //Driver Output Control(1)
	// TFTSPI_Write_Byte(0x01);
	TFTSPI_Write_Cmd(0xe0); //设置伽马（+极性）校正特性
	TFTSPI_Write_Byte(0x02);
	TFTSPI_Write_Byte(0x1c);
	TFTSPI_Write_Byte(0x07);
	TFTSPI_Write_Byte(0x12);
	TFTSPI_Write_Byte(0x37);
	TFTSPI_Write_Byte(0x32);
	TFTSPI_Write_Byte(0x29);
	TFTSPI_Write_Byte(0x2d);
	TFTSPI_Write_Byte(0x29);
	TFTSPI_Write_Byte(0x25);
	TFTSPI_Write_Byte(0x2b);
	TFTSPI_Write_Byte(0x39);
	TFTSPI_Write_Byte(0x00);
	TFTSPI_Write_Byte(0x01);
	TFTSPI_Write_Byte(0x03);
	TFTSPI_Write_Byte(0x10);
	TFTSPI_Write_Cmd(0xe1); //设置伽马（-极性）校正特性
	TFTSPI_Write_Byte(0x03);
	TFTSPI_Write_Byte(0x1d);
	TFTSPI_Write_Byte(0x07);
	TFTSPI_Write_Byte(0x06);
	TFTSPI_Write_Byte(0x2e);
	TFTSPI_Write_Byte(0x2c);
	TFTSPI_Write_Byte(0x29);
	TFTSPI_Write_Byte(0x2d);
	TFTSPI_Write_Byte(0x2e);
	TFTSPI_Write_Byte(0x2e);
	TFTSPI_Write_Byte(0x37);
	TFTSPI_Write_Byte(0x3f);
	TFTSPI_Write_Byte(0x00);
	TFTSPI_Write_Byte(0x00);
	TFTSPI_Write_Byte(0x02);
	TFTSPI_Write_Byte(0x10);
	TFTSPI_Write_Cmd(0xb1); //设置屏幕刷新频率
	TFTSPI_Write_Byte(0x01);
	TFTSPI_Write_Byte(0x01);
	TFTSPI_Write_Byte(0x01);
	TFTSPI_Write_Cmd(0xb4); //显示反转控制
	TFTSPI_Write_Byte(0x07); //NLA=1,NLB=1,NLC=1
	TFTSPI_Write_Cmd(0xc0); //功率控制1
	TFTSPI_Write_Byte(0x0a);
	TFTSPI_Write_Byte(0x02);
	TFTSPI_Write_Cmd(0xc1); //功率控制2
	TFTSPI_Write_Byte(0x02);
	TFTSPI_Write_Cmd(0xc5); //VCOM控制1
	TFTSPI_Write_Byte(0x4f);
	TFTSPI_Write_Byte(0x5a);
	TFTSPI_Write_Cmd(0xc7); //VCOM偏移控制
	TFTSPI_Write_Byte(0x40);
	TFTSPI_Write_Cmd(0x2a); //设置MCU可操作的LCD内部RAM横坐标起始、结束参数
	TFTSPI_Write_Byte(0x00); //横坐标起始地址0x0000
	TFTSPI_Write_Byte(0x00);
	TFTSPI_Write_Byte(0x00); //横坐标结束地址0x007f(127)
	TFTSPI_Write_Byte(0x7f);
	TFTSPI_Write_Cmd(0x2b); //设置MCU可操作的LCD内部RAM纵坐标起始结束参数
	TFTSPI_Write_Byte(0x00); //纵坐标起始地址0x0000
	TFTSPI_Write_Byte(0x00);
	TFTSPI_Write_Byte(0x00); //纵坐标结束地址0x009f(159)
	TFTSPI_Write_Byte(0x9f);
	TFTSPI_Write_Cmd(0x36); //设置显示方向及颜色格式（RGB/BRG）

	switch (USE_HORIZONTAL)
	{
	case 0: TFTSPI_Write_Byte(0xC0);
		break;
	case 1: TFTSPI_Write_Byte(0x00);
		break;
	case 2: TFTSPI_Write_Byte(0x78);
		break;
	case 3: TFTSPI_Write_Byte(0xA0);
		break;
	}
	TFTSPI_Inversion(1);
	TFTSPI_Write_Cmd(0x29); //开启屏幕显示
	TFTSPI_Write_Cmd(0x2c); //设置为LCD接收数据/命令模式

	TFTSPI_CLS(COLOR_BLACK); //初始为全黑
}

/*!
 * @brief    简单延时函数
 *
 * @param    Del ：延时时间
 *
 * @return   无
 *
 * @note     内部调用
 *
 * @see      tft18delay_1us(1);
 *
 * @date     2019/6/13 星期四
 */
void tft18delay_1us(unsigned int Del) //
{
	delay_us(Del);
}

#if USE_QSPI
/*!
 * @brief    写命令
 *
 * @param    cmd ：命令
 *
 * @return   无
 *
 * @note     内部调用
 *
 * @see      TFTSPI_Write_Cmd(0xb7); //LCD Driveing control
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Write_Cmd(uint8_t cmd)
{
	TFTSPI_DC_L; //DC=0发送命令
#if USE_SPI_DMA
	HAL_SPI_Transmit_DMA(&hspi4, &cmd, 1);
#else
	HAL_SPI_Transmit(&hspi4, &cmd, 1, 0xffff); //发送数据
#endif
}

/*!
 * @brief    写字节
 *
 * @param    dat ：数据
 *
 * @return   无
 *
 * @note     内部调用
 *
 * @see      TFTSPI_Write_Byte(0x00);
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Write_Byte(uint8_t dat)
{
	TFTSPI_DC_H; //DC=1发送数据
#if USE_SPI_DMA
	HAL_SPI_Transmit_DMA(&hspi4, &dat, 1);
#else
	HAL_SPI_Transmit(&hspi4, &dat, 1, 0xffff); //发送数据
#endif
}

/*!
 * @brief    写半字
 *
 * @param    dat ：数据
 *
 * @return   无
 *
 * @note     无
 *
 * @see      TFTSPI_Write_Word(0xFFFF);
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Write_Word(unsigned short dat)
{
	uint8_t Data[2] = {0, 0};
	TFTSPI_DC_H; //DC=1发送数据
	Data[0] = dat >> 8;
	Data[1] = (uint8_t)dat;
#if USE_SPI_DMA
	HAL_SPI_Transmit_DMA(&hspi4, Data, 2);
#else
	HAL_SPI_Transmit(&hspi4, Data, 2, 0xffff); //发送数据
#endif
}

#else

/*!
 * @brief    写命令
 *
 * @param    cmd ：命令
 *
 * @return   无
 *
 * @note     内部调用
 *
 * @see      TFTSPI_Write_Cmd(0xb7); //LCD Driveing control
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Write_Cmd(uint8_t cmd)
{
	unsigned short i;
	TFTSPI_DC_L; //DC=0发送数据

	for (i = 0; i < 8; i++)
	{
		TFTSPI_SCK_L;
		tft18delay_1us(1); // SCK=0  ILI9163_SCK=0;
		if (cmd & 0x80)
		{
			TFTSPI_SDI_H;
		} // SDI=1
		else
		{
			TFTSPI_SDI_L;
		} // SDI=0
		TFTSPI_SCK_H;
		tft18delay_1us(1); // SCK=1  ILI9163_SCK = 1;
		cmd = (cmd << 1);
	}
}

/*!
 * @brief    写字节
 *
 * @param    dat ：数据
 *
 * @return   无
 *
 * @note     内部调用
 *
 * @see      TFTSPI_Write_Byte(0x00);    //CRL=0
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Write_Byte(uint8_t dat)
{
	unsigned short i;
	TFTSPI_DC_H; //DC=1发送数据
	for (i = 0; i < 8; i++)
	{
		TFTSPI_SCK_L;
		tft18delay_1us(1); // SCK=0  ILI9163_SCK=0;
		if (dat & 0x80)
		{
			TFTSPI_SDI_H;
		} // SDI=1
		else
		{
			TFTSPI_SDI_L;
		} // SDI=0
		TFTSPI_SCK_H;
		tft18delay_1us(1); // SCK=1 ILI9163_SCK = 1;
		dat = (dat << 1);
	}
}

/*!
 * @brief    写半字
 *
 * @param    dat ：数据
 *
 * @return   无
 *
 * @note     无
 *
 * @see      TFTSPI_Write_Word(0xFFFF);
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Write_Word(unsigned short dat)
{
	unsigned short i;

	TFTSPI_DC_H; //DC=1发送数据

	for (i = 0; i < 16; i++)
	{
		TFTSPI_SCK_L;
		tft18delay_1us(1); // SCK=0  ILI9163_SCK=0;
		if (dat & 0x8000)
		{
			TFTSPI_SDI_H;
		} // SDI=1
		else
		{
			TFTSPI_SDI_L;
		} // SDI=0
		TFTSPI_SCK_H;
		tft18delay_1us(1); // SCK=1  ILI9163_SCK=1;
		dat <<= 1;
	}
}

#endif

/*!
 * @brief    重新定位输入信息位置
 *
 * @param    xs ：起始x
 * @param    ys ：起始y
 * @param    xe ：结束x
 * @param    ys ：结束y
 *
 * @return   无
 *
 * @note     内部调用
 *
 * @see      TFTSPI_Set_Pos(10, 20, 30, 40);
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Set_Pos(unsigned short xs, unsigned short ys, unsigned short xe,
                    unsigned short ye)
{
	//针对0.9寸屏的偏移
	xs += 1;
	ys += 26;
	xe += 1;
	ye += 26;
	TFTSPI_Write_Cmd(0x2A); //Colulm addRSTs set
	TFTSPI_Write_Byte(0x00); //行起始坐标高8位，始终为零
	TFTSPI_Write_Byte(xs); //行起始坐标低8位
	TFTSPI_Write_Byte(0x00); //行终止坐标高8位，始终为零
	TFTSPI_Write_Byte(xe); //行终止坐标低8位
	TFTSPI_Write_Cmd(0x2B); //Colulm addRSTs set
	TFTSPI_Write_Byte(0x00); //列起始坐标高8位，始终为零
	TFTSPI_Write_Byte(ys); //列起始坐标低8位
	TFTSPI_Write_Byte(0x00); //列终止坐标高8位，始终为零
	TFTSPI_Write_Byte(ye); //列终止坐标低8位
	TFTSPI_Write_Cmd(0x2C); //GRAM接收MCU数据或命令
}


/*!
 * @brief    重置地址
 *
 * @param    无
 *
 * @return   无
 *
 * @note     内部调用
 *
 * @see      TFTSPI_Addr_Rst();
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Addr_Rst(void)
{
	TFTSPI_Write_Cmd(0x2a); //配置MCU可操作的LCD内部RAM横坐标起始、结束参数
	TFTSPI_Write_Byte(0x00); //横坐标起始地址0x0000
	TFTSPI_Write_Byte(0x00);
	TFTSPI_Write_Byte(0x00); //横坐标结束地址0x007f(127)
	TFTSPI_Write_Byte(0xa8); //7f
	TFTSPI_Write_Cmd(0x2b); //配置MCU可操作的LCD内部RAM纵坐标起始结束参数
	TFTSPI_Write_Byte(0x00); //纵坐标起始地址0x0000
	TFTSPI_Write_Byte(0x00);
	TFTSPI_Write_Byte(0x00); //纵坐标结束地址0x009f(159)
	TFTSPI_Write_Byte(0xb3); //9f
	TFTSPI_Write_Cmd(0x2C); //GRAM接收MCU数据或命令
}

void TFT_sendGRAM_DMA(uint16_t* gram)
{
	TFTSPI_Addr_Rst();
	TFTSPI_DC_H; //DC=1发送数据
	HAL_SPI_Transmit_DMA(&hspi4, (uint8_t*)gram,TFT18H * TFT18W * 2);
}

void TFT_sendGRAM(uint16_t* gram)
{
	TFTSPI_Addr_Rst();
	TFTSPI_DC_H; //DC=1发送数据
	HAL_SPI_Transmit(&hspi4, (uint8_t*)gram,TFT18H * TFT18W * 2, 0xffff);
}

/*!
 * @brief    填充指定区域
 *
 * @param    xs ：起始x
 * @param    ys ：起始y
 * @param    xe ：结束x
 * @param    ys ：结束y
 * @param    color ：填充的颜色
 *
 * @return   无
 *
 * @note     起始、终止横坐标(0-127)，纵坐标(0-159),显示颜色uint16
 *
 * @see      TFTSPI_Fill_Area(10, 20, 30, 40, u16YELLOW);
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Fill_Area(unsigned short xs, unsigned short ys, unsigned short xe,
                      unsigned short ye, unsigned short color)
{
	unsigned int i, j;
#if USE_EX_GRAM
	for (i = ys; i < ye; i++)
	{
		for (j = xs; j < xe; j++)
		{
			ex_gram[i * TFT18W + j] = color;
		}
	}
#else
	TFTSPI_Set_Pos(xs, ys, xe, ye);
	for (i = 0; i < (xe - xs + 1); i++)
	{
		for (j = 0; j < (ye - ys); j++)
		{
			TFTSPI_Write_Word(color);
		}
	}
#endif
}

/*!
 * @brief    全屏显示单色画面
 *
 * @param    color ：填充的颜色
 *
 * @return   无
 *
 * @note     起始、终止横坐标(0-127)，纵坐标(0-159),显示颜色uint16
 *
 * @see      TFTSPI_CLS(u16YELLOW);
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_CLS(unsigned short color)
{
	unsigned int i, j;
#if USE_EX_GRAM
	for (i = 0; i < TFT18H; i++)
	{
		for (j = 0; j < TFT18W; j++)
		{
			ex_gram[i * TFT18W + j] = color;
		}
	}
#else
	TFTSPI_Addr_Rst();
	uint8_t Data[2] = {0, 0};
	TFTSPI_DC_H; //DC=1发送数据
	Data[0] = color >> 8;
	Data[1] = (uint8_t)color;
	for (i = 0; i < TFT18W * TFT18H; i++)
		HAL_SPI_Transmit(&hspi4, Data, 2, 0xffff); //发送数据

	//    for (i = 0; i < TFT18W; i++)      //160
	//            {
	//        for (j = 0; j < TFT18H; j++) {
	//            TFTSPI_Write_Word(color);
	//        }
	//    }
#endif
}

/*!
 * @brief    填充矩形区域
 *
 * @param    xs ：起始x
 * @param    ys ：起始y
 * @param    xe ：结束x
 * @param    ys ：结束y
 * @param    color_dat ：填充的颜色
 *
 * @return   无
 *
 * @note     起始、终止横坐标(0-127)，纵坐标(0-159),显示颜色uint16
 *
 * @see      TFTSPI_Draw_Part(10, 20, 30, 40, u16YELLOW);
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Draw_Part(unsigned short xs, unsigned short ys, unsigned short xe,
                      unsigned short ye, unsigned short color_dat)
{
	unsigned short i, j;
#if USE_EX_GRAM
	for (j = ys; j < (ye + 1); j++)
	{
		for (i = xs; i < (xe + 1); i++)
		{
			ex_gram[j * TFT18W + i] = color_dat;
		}
	}
#else
	TFTSPI_Set_Pos(xs, ys, xe, ye);
	uint8_t Data[2] = {0, 0};
	TFTSPI_DC_H; //DC=1发送数据
	Data[0] = color_dat >> 8;
	Data[1] = (uint8_t)color_dat;
	for (i = 0; i < (ye - ys + 1) * (xe - xs + 1); i++)
		HAL_SPI_Transmit(&hspi4, Data, 2, 0xffff); //发送数据
	//    for (j = 0; j < (ye - ys + 1); j++) {
	//        for (i = 0; i < (xe - xs + 1); i++) {
	//            TFTSPI_Write_Word(color_dat);
	//        }
	//    }
#endif
}

/*!
 * @brief    画矩形边框
 *
 * @param    xs ：起始x
 * @param    ys ：起始y
 * @param    xe ：结束x
 * @param    ys ：结束y
 * @param    color_dat ：颜色
 *
 * @return   无
 *
 * @note     起始、终止横坐标(0-127)，纵坐标(0-159),显示颜色uint16
 *
 * @see      TFTSPI_Draw_Rectangle(10, 20, 30, 40, u16YELLOW);
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Draw_Rectangle(unsigned short xs, unsigned short ys,
                           unsigned short xe, unsigned short ye, unsigned short color_dat)
{
	TFTSPI_Draw_Line(xs, ys, xs, ye, color_dat); //画矩形左边
	TFTSPI_Draw_Line(xe, ys, xe, ye, color_dat); //画矩形右边
	TFTSPI_Draw_Line(xs, ys, xe, ys, color_dat); //画矩形上边
	TFTSPI_Draw_Line(xs, ye, xe, ye, color_dat); //画矩形下边
}

/*!
 * @brief    画圆
 *
 * @param    x ：圆心x   (0-127)
 * @param    y ：圆心y   (0-159)
 * @param    r ：半径    (0-128)
 * @param    color_dat ：颜色
 *
 * @return   无
 *
 * @note     圆心坐标不要超出屏幕范围
 *
 * @see      TFTSPI_Draw_Circle(50, 50, 30, u16YELLOW);
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Draw_Circle(unsigned short x, unsigned short y, unsigned short r,
                        unsigned short color_dat)
{
	unsigned short dx, dy = r;

	if ((x >= r) && ((TFT18W - x) >= r) && (y >= r) && ((TFT18H - y) >= r)) //确定所画圆在屏幕范围内，没有超出最外边，(暂不支持与屏幕边相交)
	{
		for (dx = 0; dx <= r; dx++)
		{
			while ((r * r + 1 - dx * dx) < (dy * dy)) dy--;
			TFTSPI_Draw_Dot(x + dx, y - dy, color_dat);
			TFTSPI_Draw_Dot(x - dx, y - dy, color_dat);
			TFTSPI_Draw_Dot(x - dx, y + dy, color_dat);
			TFTSPI_Draw_Dot(x + dx, y + dy, color_dat);

			TFTSPI_Draw_Dot(x + dy, y - dx, color_dat);
			TFTSPI_Draw_Dot(x - dy, y - dx, color_dat);
			TFTSPI_Draw_Dot(x - dy, y + dx, color_dat);
			TFTSPI_Draw_Dot(x + dy, y + dx, color_dat);
		}
	}
}

/*!
 * @brief    画线
 *
 * @param    xs ：起始x
 * @param    ys ：起始y
 * @param    xe ：结束x
 * @param    ys ：结束y
 * @param    color_dat ：颜色
 *
 * @return   无
 *
 * @note     起始、终止横坐标(0-127)，纵坐标(0-159),显示颜色uint16
 *
 * @see      TFTSPI_Draw_Line(10, 20, 30, 40, u16YELLOW);
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Draw_Line(unsigned short xs, unsigned short ys, unsigned short xe,
                      unsigned short ye, unsigned short color_dat)
{
	uint16_t i;
	int16_t x, y, dx, dy, d, incrE, incrNE, temp;
	int16_t x0 = xs, y0 = ys, x1 = xe, y1 = ye;
	uint8_t yflag = 0, xyflag = 0;
#if USE_EX_GRAM
	if (xs == xe) //如果是画垂直线则只需对竖直坐标计数
	{
		for (i = ys; i < (ye + 1); i++)
		{
			ex_gram[i * TFT18W + xs] = color_dat;
		}
	}
	else if (ys == ye) //如果是水平线则只需要对水平坐标计数
	{
		for (i = xs; i < (xe + 1); i++)
		{
			ex_gram[ys * TFT18W + i] = color_dat;
		}
	}
	else //如果是斜线，则重新计算，使用画点函数画出直线
	{
		/*使用Bresenham算法画直线，可以避免耗时的浮点运算，效率更高*/
		/*参考文档：https://www.cs.montana.edu/courses/spring2009/425/dslectures/Bresenham.pdf*/
		/*参考教程：https://www.bilibili.com/video/BV1364y1d7Lo*/

		if (x0 > x1) //0号点X坐标大于1号点X坐标
		{
			/*交换两点坐标*/
			/*交换后不影响画线，但是画线方向由第一、二、三、四象限变为第一、四象限*/
			temp = x0;
			x0 = x1;
			x1 = temp;
			temp = y0;
			y0 = y1;
			y1 = temp;
		}

		if (y0 > y1) //0号点Y坐标大于1号点Y坐标
		{
			/*将Y坐标取负*/
			/*取负后影响画线，但是画线方向由第一、四象限变为第一象限*/
			y0 = -y0;
			y1 = -y1;

			/*置标志位yflag，记住当前变换，在后续实际画线时，再将坐标换回来*/
			yflag = 1;
		}

		if (y1 - y0 > x1 - x0) //画线斜率大于1
		{
			/*将X坐标与Y坐标互换*/
			/*互换后影响画线，但是画线方向由第一象限0~90度范围变为第一象限0~45度范围*/
			temp = x0;
			x0 = y0;
			y0 = temp;
			temp = x1;
			x1 = y1;
			y1 = temp;

			/*置标志位xyflag，记住当前变换，在后续实际画线时，再将坐标换回来*/
			xyflag = 1;
		}

		/*以下为Bresenham算法画直线*/
		/*算法要求，画线方向必须为第一象限0~45度范围*/
		dx = x1 - x0;
		dy = y1 - y0;
		incrE = 2 * dy;
		incrNE = 2 * (dy - dx);
		d = 2 * dy - dx;
		x = x0;
		y = y0;

		/*画起始点，同时判断标志位，将坐标换回来*/
		if (yflag && xyflag) { TFTSPI_Draw_Dot(y, -x, color_dat); }
		else if (yflag) { TFTSPI_Draw_Dot(x, -y, color_dat); }
		else if (xyflag) { TFTSPI_Draw_Dot(y, x, color_dat); }
		else { TFTSPI_Draw_Dot(x, y, color_dat); }

		while (x < x1) //遍历X轴的每个点
		{
			x++;
			if (d < 0) //下一个点在当前点东方
			{
				d += incrE;
			}
			else //下一个点在当前点东北方
			{
				y++;
				d += incrNE;
			}

			/*画每一个点，同时判断标志位，将坐标换回来*/
			if (yflag && xyflag) { TFTSPI_Draw_Dot(y, -x, color_dat); }
			else if (yflag) { TFTSPI_Draw_Dot(x, -y, color_dat); }
			else if (xyflag) { TFTSPI_Draw_Dot(y, x, color_dat); }
			else { TFTSPI_Draw_Dot(x, y, color_dat); }
		}
	}
#else
	if (xs == xe) //如果是画垂直线则只需对竖直坐标计数
	{
		TFTSPI_Set_Pos(xs, ys, xe, ye);
		for (i = 0; i < (ye - ys + 1); i++)
		{
			TFTSPI_Write_Word(color_dat);
		}
	}
	else if (ys == ye) //如果是水平线则只需要对水平坐标计数
	{
		TFTSPI_Set_Pos(xs, ys, xe, ye);
		for (i = 0; i < (xe - xs + 1); i++)
		{
			TFTSPI_Write_Word(color_dat);
		}
	}
	else //如果是斜线，则重新计算，使用画点函数画出直线
	{
		/*使用Bresenham算法画直线，可以避免耗时的浮点运算，效率更高*/
		/*参考文档：https://www.cs.montana.edu/courses/spring2009/425/dslectures/Bresenham.pdf*/
		/*参考教程：https://www.bilibili.com/video/BV1364y1d7Lo*/

		if (x0 > x1) //0号点X坐标大于1号点X坐标
		{
			/*交换两点坐标*/
			/*交换后不影响画线，但是画线方向由第一、二、三、四象限变为第一、四象限*/
			temp = x0;
			x0 = x1;
			x1 = temp;
			temp = y0;
			y0 = y1;
			y1 = temp;
		}

		if (y0 > y1) //0号点Y坐标大于1号点Y坐标
		{
			/*将Y坐标取负*/
			/*取负后影响画线，但是画线方向由第一、四象限变为第一象限*/
			y0 = -y0;
			y1 = -y1;

			/*置标志位yflag，记住当前变换，在后续实际画线时，再将坐标换回来*/
			yflag = 1;
		}

		if (y1 - y0 > x1 - x0) //画线斜率大于1
		{
			/*将X坐标与Y坐标互换*/
			/*互换后影响画线，但是画线方向由第一象限0~90度范围变为第一象限0~45度范围*/
			temp = x0;
			x0 = y0;
			y0 = temp;
			temp = x1;
			x1 = y1;
			y1 = temp;

			/*置标志位xyflag，记住当前变换，在后续实际画线时，再将坐标换回来*/
			xyflag = 1;
		}

		/*以下为Bresenham算法画直线*/
		/*算法要求，画线方向必须为第一象限0~45度范围*/
		dx = x1 - x0;
		dy = y1 - y0;
		incrE = 2 * dy;
		incrNE = 2 * (dy - dx);
		d = 2 * dy - dx;
		x = x0;
		y = y0;

		/*画起始点，同时判断标志位，将坐标换回来*/
		if (yflag && xyflag) { TFTSPI_Draw_Dot(y, -x, color_dat); }
		else if (yflag) { TFTSPI_Draw_Dot(x, -y, color_dat); }
		else if (xyflag) { TFTSPI_Draw_Dot(y, x, color_dat); }
		else { TFTSPI_Draw_Dot(x, y, color_dat); }

		while (x < x1) //遍历X轴的每个点
		{
			x++;
			if (d < 0) //下一个点在当前点东方
			{
				d += incrE;
			}
			else //下一个点在当前点东北方
			{
				y++;
				d += incrNE;
			}

			/*画每一个点，同时判断标志位，将坐标换回来*/
			if (yflag && xyflag) { TFTSPI_Draw_Dot(y, -x, color_dat); }
			else if (yflag) { TFTSPI_Draw_Dot(x, -y, color_dat); }
			else if (xyflag) { TFTSPI_Draw_Dot(y, x, color_dat); }
			else { TFTSPI_Draw_Dot(x, y, color_dat); }
		}
	}
#endif
}

/*!
 * @brief    画点
 *
 * @param    x ：x
 * @param    y ：y
 * @param    color_dat ：颜色
 *
 * @return   无
 *
 * @note     起始、终止横坐标(0-127)，纵坐标(0-159),显示颜色uint16
 *
 * @see      TFTSPI_Draw_Dot(10, 20, u16YELLOW);
 *
 * @date     2019/6/13 星期四
 */
void TFTSPI_Draw_Dot(unsigned short x, unsigned short y,
                     unsigned short color_dat)
{
#if USE_EX_GRAM
	ex_gram[y * TFT18W + x] = color_dat;
#else
	TFTSPI_Set_Pos(x, y, x, y);
	TFTSPI_Write_Word(color_dat);
#endif
}

void TFT_ShowChar(uint8_t X, uint8_t Y, char Char, unsigned short word_color, unsigned short back_color,
                  enum Font_size Size)
{
	unsigned short i, j;
#if USE_EX_GRAM
	switch (Size)
	{
	case fsize_6X8:
		if (X < 0 || X > ((TFT18W - 1) - 6) || Y < 0 || Y > ((TFT18H - 1) - 8))
		{
			return;
		}
		for (j = 0; j < 8; j++)
		{
			for (i = 0; i < 6; i++)
			{
				if ((Font_code8[Char - 32][i]) & (0x01 << j))
					TFTSPI_Draw_Dot(X + i, Y + j, word_color); //画点
				else
					TFTSPI_Draw_Dot(X + i, Y + j, back_color); //画点
			}
		}
		break;

	case fsize_8X16:
		if (X < 0 || X > ((TFT18W - 1) - 8) || Y < 0 || Y > ((TFT18H - 1) - 16))
		{
			return;
		}
		for (j = 0; j < 16; j++)
		{
			for (i = 0; i < 8; i++)
			{
				if ((Font_code16[Char - 32][j]) & (0x01 << i))
					TFTSPI_Draw_Dot(X + i, Y + j, word_color); //画点
				else
					TFTSPI_Draw_Dot(X + i, Y + j, back_color); //画点
			}
		}
		break;
	case fsize_12X24:
		if (X < 0 || X > ((TFT18W - 1) - 12) || Y < 0 || Y > ((TFT18H - 1) - 24))
		{
			return;
		}
		for (j = 0; j < 48; j++)
		{
			for (i = 0; i < 8; i++)
			{
				if ((Font_code16[Char - 32][j]) & (0x0001 << i))
					TFTSPI_Draw_Dot(X + i, Y + j, word_color); //画点
				else
					TFTSPI_Draw_Dot(X + i, Y + j, back_color); //画点
			}
		}
		break;
	case fsize_16X32:
		if (X < 0 || X > ((TFT18W - 1) - 8) || Y < 0 || Y > ((TFT18H - 1) - 16))
		{
			return;
		}
		for (j = 0; j < 64; j++)
		{
			for (i = 0; i < 8; i++)
			{
				if ((Font_code16[Char - 32][j]) & (0x0001 << i))
					TFTSPI_Draw_Dot(X + i, Y + j, word_color); //画点
				else
					TFTSPI_Draw_Dot(X + i, Y + j, back_color); //画点
			}
		}
		break;
	}
#else
	switch (Size)
	{
	case fsize_6X8:
		TFTSPI_Set_Pos(X, Y, X + 5, (Y + 1) * 8 - 1); //定位字符显示区域
		for (j = 0; j < 8; j++)
		{
			for (i = 0; i < 6; i++)
			{
				if ((Font_code8X6[Char - 32][i]) & (0x01 << j))
					TFTSPI_Write_Word(word_color);
				else
					TFTSPI_Write_Word(back_color);
			}
		}
		break;


	case fsize_8X16:
		TFTSPI_Set_Pos(X, Y, X + 7, (Y + 1) * 16 - 1); //定位字符显示区域
		for (j = 0; j < 16; j++)
		{
			for (i = 0; i < 8; i++)
			{
				if ((Font_code8X16[Char - 32][j]) & (0x01 << i))
					TFTSPI_Write_Word(word_color);
				else
					TFTSPI_Write_Word(back_color);
			}
		}
		break;
	case fsize_12X24:
		TFTSPI_Set_Pos(X, Y, X + 11, (Y + 1) * 24 - 1); //定位字符显示区域
		for (j = 0; j < 48; j++)
		{
			for (i = 0; i < 8; i++)
			{
				if (j % 2 == 1 && i >= 4)
				{
					//处理16bit字宽多出的4bit
					break;
				}

				if ((Font_code12X24[Char - 32][j]) & (0x0001 << i))
					TFTSPI_Write_Word(word_color);
				else
					TFTSPI_Write_Word(back_color);
			}
		}
		break;
	case fsize_16X32:
		TFTSPI_Set_Pos(X, Y, X + 15, (Y + 1) * 32 - 1); //定位字符显示区域
		for (j = 0; j < 64; j++)
		{
			for (i = 0; i < 8; i++)
			{
				if ((Font_code16X32[Char - 32][j]) & (0x0001 << i))
					TFTSPI_Write_Word(word_color);
				else
					TFTSPI_Write_Word(back_color);
			}
		}
		break;

	default: //其余情况默认最小字体
		TFTSPI_Set_Pos(X, Y, X + 5, (Y + 1) * 8 - 1); //定位字符显示区域
		for (j = 0; j < 8; j++)
		{
			for (i = 0; i < 6; i++)
			{
				if ((Font_code8X6[Char - 32][i]) & (0x01 << j))
					TFTSPI_Write_Word(word_color);
				else
					TFTSPI_Write_Word(back_color);
			}
		}
		break;
	}

#endif
}

void TFT_ShowString(uint8_t X, uint8_t Y, char* String, unsigned short word_color, unsigned short back_color,
                    enum Font_size Size)
{
	switch (Size)
	{
	case fsize_6X8:
		while (*String)
		{
			TFT_ShowChar(X, Y, *String++, word_color, back_color, fsize_6X8);
			X += 6;
		}
		break;

	case fsize_8X16:
		while (*String)
		{
			TFT_ShowChar(X, Y, *String++, word_color, back_color, fsize_8X16);
			X += 8;
		}
		break;

	case fsize_12X24:
		while (*String)
		{
			TFT_ShowChar(X, Y, *String++, word_color, back_color, fsize_12X24);
			X += 12;
		}
		break;
	case fsize_16X32:
		while (*String)
		{
			TFT_ShowChar(X, Y, *String++, word_color, back_color, fsize_16X32);
			X += 16;
		}
		break;
	}
}

void TFT_Printf(uint8_t X, uint8_t Y, unsigned short word_color, unsigned short back_color, enum Font_size Size,
                char* format, ...)
{
	char String[30]; //定义字符数组
	va_list arg; //定义可变参数列表数据类型的变量arg
	va_start(arg, format); //从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg); //使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg); //结束变量arg
	TFT_ShowString(X, Y, String, word_color, back_color, Size);
}

/******************************************************************************
      函数说明：显示图片
      入口数据：x,y起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组
      返回值：  无
******************************************************************************/
void TFT_ShowPicture(uint8_t x, uint8_t y, uint8_t length, uint8_t width, const uint8_t pic[])
{
	TFTSPI_Set_Pos(x, y, x + length - 1, y + width - 1); //定位图片显示区域
	TFTSPI_DC_H; //DC=1发送数据

	HAL_SPI_Transmit(&hspi4, pic, 2 * length * width, 0xffff); //发送数据
}


