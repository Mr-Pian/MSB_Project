//
// Created by maodie on 2025/10/29.
//
#include "control.h"

// 初始化结构体
Yun_tai_typedef the_yun_tai;
PID_Controller_TypeDef yaw_pid_controller;
PID_Controller_TypeDef pitch_pid_controller;
Pixel_typedef real_pixel;
Pixel_typedef target_pixel;
Pixel_target_typedef the_pixel_target;
MSB_typedef the_msb;
uint16_convert_typedef convert_data;

void Control_Init(void)
{
    // // 复位云台
    // yuntai_reset();
    // 云台设置零点
    yuntai_set_zero_point();
    // Pid 初始化
    the_yun_tai.Yaw_pid = &yaw_pid_controller;
    the_yun_tai.Pitch_pid = &pitch_pid_controller;
    pid_init(the_yun_tai.Pitch_pid);
    the_yun_tai.Pitch_pid->f_param_init(the_yun_tai.Pitch_pid, PID_Speed, MAX_PITCH_OUT,
                                        MAX_PITCH_OUT, 0.0f, 0.0f, -0.0005f, 0.0f);
    the_yun_tai.Pitch_pid->f_pid_switch(the_yun_tai.Pitch_pid, 1); //使能pid

    pid_init(the_yun_tai.Yaw_pid);
    the_yun_tai.Yaw_pid->f_param_init(the_yun_tai.Yaw_pid, PID_Speed, MAX_YAW_OUT,
                                      MAX_YAW_OUT, 0.0f, 0.0f, -0.0005f, 0.0f);
    the_yun_tai.Yaw_pid->f_pid_switch(the_yun_tai.Yaw_pid, 1); //使能pid
    // 赋值
    the_pixel_target.real_pixel = &real_pixel;
    the_pixel_target.target_pixel = &target_pixel;
    the_msb.the_pixel_target = &the_pixel_target;
}

void get_xy_from_raspi(uint8_t* data, int fifo_length)
{
    for (int i = 0; i < fifo_length; i++)
    {
        if (data[i] == 0x0a && data[i + 5] == 0x55) //找到帧
        {
            convert_data.raw_data[0] = data[i + 2];
            convert_data.raw_data[1] = data[i + 1];
            the_msb.the_pixel_target->real_pixel->pixel_x = convert_data.data;
            convert_data.raw_data[0] = data[i + 4];
            convert_data.raw_data[1] = data[i + 3];
            the_msb.the_pixel_target->real_pixel->pixel_y = convert_data.data;
        }
    }
}

void Laser_X_Control(void)
{
    the_yun_tai.Yaw_pid->target = (float)the_msb.the_pixel_target->target_pixel->pixel_x;
    the_yun_tai.Yaw_pid->f_cal_pid(the_yun_tai.Yaw_pid,
                                   (float)the_msb.the_pixel_target->real_pixel->pixel_x);
    yuntai_set_pos_angle(MOTOR_YAW, the_yun_tai.Yaw_pid->output);
}

void Laser_Y_Control(void)
{
    the_yun_tai.Pitch_pid->target = (float)the_msb.the_pixel_target->target_pixel->pixel_y;
    the_yun_tai.Pitch_pid->f_cal_pid(the_yun_tai.Pitch_pid,
                                     (float)the_msb.the_pixel_target->real_pixel->pixel_y);
    yuntai_set_pos_angle(MOTOR_PITCH, the_yun_tai.Pitch_pid->output);
}
