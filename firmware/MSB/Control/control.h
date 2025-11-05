//
// Created by maodie on 2025/10/29.
//

#ifndef MSB_CONTROL_H
#define MSB_CONTROL_H

#include "pid.h"
#include "yuntai.h"

/************ 参数设置 ************/
//最大转动角度限幅
#define MAX_YAW_OUT 60.0
#define MAX_PITCH_OUT 45.0
//初始pid参数
#define PITCH_KP 0.0028f
#define PITCH_KI 0.0037f
#define PITCH_KD 0.0f
#define YAW_KP 0.0028f
#define YAW_KI 0.0037f
#define YAW_KD 0.0f
//激光器最大输出占空比(5v)(0-99)
#define MAX_PWM_CP 70.0
/************ 参数设置 ************/

typedef union
{
    uint8_t raw_data[2];
    uint16_t data;
} uint16_convert_typedef;

typedef struct
{
    PID_Controller_TypeDef* Yaw_pid;
    PID_Controller_TypeDef* Pitch_pid;
} Yun_tai_typedef;

typedef struct
{
    uint16_t pixel_x;
    uint16_t pixel_y;
} Pixel_typedef;

typedef struct
{
    Pixel_typedef* real_pixel;
    Pixel_typedef* target_pixel;
} Pixel_target_typedef;

typedef struct
{
    Pixel_target_typedef* the_pixel_target;
} MSB_typedef;

void Control_Init(void);
void get_xy_from_raspi(uint8_t* data, int fifo_length);
void Laser_X_Control(void);
void Laser_Y_Control(void);
void Laser_set_power(uint8_t power);
void Laser_on_off(uint8_t state);

extern MSB_typedef the_msb;
extern Yun_tai_typedef the_yun_tai;

#endif //MSB_CONTROL_H

