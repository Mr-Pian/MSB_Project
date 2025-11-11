#ifndef __THT18_ST7735S_H__
#define __THT18_ST7735S_H__

#include "main.h"
#include "stdint.h"

/*******************接口定义******************************/
/***TFT SPI模块管脚定义***/
#define TFTSPI_CS     GPIOE,GPIO_PIN_11     // CS管脚 默认拉低，可以不用
#define TFTSPI_SCK    GPIOE,GPIO_PIN_12     // SPI SCK管脚
#define TFTSPI_SDI    GPIOE,GPIO_PIN_14     // SPI MOSI管脚
#define TFTSPI_DC     GPIOE,GPIO_PIN_13     // D/C管脚
#define TFTSPI_RST    GPIOA,GPIO_PIN_6     // RESET管脚

#define TFTSPI_CS_H     HAL_GPIO_WritePin(TFTSPI_CS,GPIO_PIN_SET);      /* CS管脚 */
#define TFTSPI_SCK_H    HAL_GPIO_WritePin(TFTSPI_SCK,GPIO_PIN_SET);     /* SCL管脚 */
#define TFTSPI_SDI_H    HAL_GPIO_WritePin(TFTSPI_SDI,GPIO_PIN_SET);     /* SDI管脚 */
#define TFTSPI_DC_H     HAL_GPIO_WritePin(TFTSPI_DC,GPIO_PIN_SET);      /* DC管脚 */
#define TFTSPI_RST_H    HAL_GPIO_WritePin(TFTSPI_RST,GPIO_PIN_SET);     /* RST管脚 */

#define TFTSPI_CS_L     HAL_GPIO_WritePin(TFTSPI_CS,GPIO_PIN_RESET);     /* CS管脚(默认拉低，不做控制) */
#define TFTSPI_SCK_L    HAL_GPIO_WritePin(TFTSPI_SCK,GPIO_PIN_RESET);    /* SCL管脚 */
#define TFTSPI_SDI_L    HAL_GPIO_WritePin(TFTSPI_SDI,GPIO_PIN_RESET);    /* SDI管脚 */
#define TFTSPI_DC_L     HAL_GPIO_WritePin(TFTSPI_DC,GPIO_PIN_RESET);     /* DC管脚 */
#define TFTSPI_RST_L    HAL_GPIO_WritePin(TFTSPI_RST,GPIO_PIN_RESET);    /* RST管脚 */

#define TFT18W        160 //TFT屏幕宽像素个数
#define TFT18H        128//TFT屏幕高像素个数

#define USE_SPI_DMA   0 //使能DMA传输
#define USE_EX_GRAM   0  //使能外部显存
#define USE_HORIZONTAL 2//设置横屏或者竖屏显示
//0:竖屏（正向）
//1:竖屏（倒转）
//2:横屏（正向）
//3:横屏（倒转）
/***字体大小选择***/
enum Font_size
{
	fsize_6X8 = 0,
	fsize_8X16,
	fsize_12X24,
	fsize_16X32
};

/***图片颜色类型选择***/
enum Image_color
{
	img_gray = 0,
	img_rgb565
};

/****************RGB565颜色定义*****************/
/* 16-bit RGB565颜色宏定义（格式：R:5位 G:6位 B:5位） */
#define COLOR_BLACK     0x0000   // 黑色
#define COLOR_WHITE     0xFFFF   // 白色
#define COLOR_RED       0xF800   // 红色
#define COLOR_LIME      0x07E0   // 绿（类似HTML lime色）
#define COLOR_BLUE      0x001F   // 蓝色
#define COLOR_YELLOW    0xFFE0   // 黄色
#define COLOR_CYAN      0x07FF   // 青色
#define COLOR_MAGENTA   0xF81F   // 品红
#define COLOR_SILVER    0xB596   // 银灰色
#define COLOR_GRAY      0x7BEF   // 标准灰色
#define COLOR_MAROON    0x8000   // 栗色（暗红）
#define COLOR_OLIVE     0x8400   // 橄榄绿（暗黄）
#define COLOR_GREEN     0x03E0   // 纯绿色（HTML标准）
#define COLOR_PURPLE    0x8010   // 紫色（红蓝混合）
#define COLOR_TEAL      0x0438   // 水鸭色（暗青）
#define COLOR_NAVY      0x0010   // 藏青色（暗蓝）
#define COLOR_ORANGE    0xFB80   // 橙色
#define COLOR_PINK      0xF8B2   // 粉红色
#define COLOR_BROWN     0x8A22   // 棕色
#define COLOR_DARKGRAY  0x4208   // 深灰色
#define COLOR_LIGHTGRAY 0xC618   // 浅灰色
#define ST7735_COLOR565(r, g, b) (((r & 0x00F8) << 8) | ((-g & 0x00FC) << 3) | ((b & 0x00F8) >> 3))

/*********TFT屏幕SPI基本驱动**********/
#if USE_EX_GRAM
extern uint16_t ex_gram[TFT18W * TFT18H];
#endif
void TFTSPI_Init(void);

void TFTSPI_Write_Cmd(uint8_t cmd);

void TFTSPI_Write_Byte(uint8_t dat);

void TFTSPI_Write_Word(unsigned short dat);
void TFT_DMA_sendGRAM(uint16_t* gram);
void TFT_sendGRAM(uint16_t* gram);
/*********TFT屏幕SPI基本驱动*********/

/*********TFT屏幕图形基本驱动********/
void TFTSPI_Set_Pos(unsigned short xs, unsigned short ys, unsigned short xe, unsigned short ye);

void TFTSPI_Fill_Area(unsigned short xs, unsigned short ys, unsigned short xe, unsigned short ye, unsigned short color);

void TFTSPI_CLS(unsigned short color);

void TFTSPI_Draw_Part(unsigned short xs, unsigned short ys, unsigned short xe, unsigned short ye,
                      unsigned short color_dat);

void TFTSPI_Draw_Rectangle(unsigned short xs, unsigned short ys, unsigned short xe, unsigned short ye,
                           unsigned short color_dat);

void TFTSPI_Draw_Circle(unsigned short x, unsigned short y, unsigned short r, unsigned short color_dat);

void TFTSPI_Draw_Line(unsigned short xs, unsigned short ys, unsigned short xe, unsigned short ye,
                      unsigned short color_dat);

void TFTSPI_Draw_Dot(unsigned short x, unsigned short y, unsigned short color_dat);
/*********TFT屏幕图形基本驱动********/

/*********TFT屏幕字符打印************/
void TFT_ShowChar(uint8_t X, uint8_t Y, char Char, unsigned short word_color, unsigned short back_color,
                  enum Font_size Size);
void TFT_ShowString(uint8_t X, uint8_t Y, char* String, unsigned short word_color, unsigned short back_color,
                    enum Font_size Size);
void TFT_Printf(uint8_t X, uint8_t Y, unsigned short word_color, unsigned short back_color, enum Font_size Size,
                char* format, ...);
/*********TFT屏幕图片显示************/
void TFT_ShowPicture(uint8_t x, uint8_t y, uint8_t length, uint8_t width, const uint8_t pic[]);
/*********TFT屏幕图片显示************/
#endif
