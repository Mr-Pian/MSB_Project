//
// Created by maodie on 2025/10/29.
//

#ifndef MSB_CONTROL_H
#define MSB_CONTROL_H

#include "pid.h"
#include "yuntai.h"

/************ 参数设计 ************/
#define MAX_YAW_OUT 10.0
#define MAX_PITCH_OUT 10.0
/************ 参数设计 ************/


typedef struct
{
    PID_Controller_TypeDef* Yaw_pid;
    PID_Controller_TypeDef* Pitch_pid;
} Yun_tai_typedef;

typedef struct
{
    int pixel_x;
    int pixel_y;
} Pixel_typedef;

typedef struct
{
    Pixel_typedef* real_pixel;
    Pixel_typedef* target_pixel;
} Pixel_target_typedef;

typedef struct
{
    Yun_tai_typedef* the_yun_tai;
    Pixel_target_typedef* the_pixel_target;
} MSB_typedef;

extern MSB_typedef the_msb;

#endif //MSB_CONTROL_H

