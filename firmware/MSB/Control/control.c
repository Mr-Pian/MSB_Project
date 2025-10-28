//
// Created by maodie on 2025/10/29.
//
#include "control.h"

// 初始化结构体
Yun_tai_typedef the_yun_tai;
MSB_typedef the_msb;

void Control_Init(void)
{
    // 复位云台
    yuntai_reset();
    // Pid 初始化
    pid_init(the_yun_tai.Pitch_pid);
    the_yun_tai.Pitch_pid->f_param_init(the_yun_tai.Pitch_pid, PID_Speed, MAX_PITCH_OUT,
                                        MAX_PITCH_OUT, 0.0f, 0.0f, 0.0f, 0.0f);
    pid_init(the_yun_tai.Yaw_pid);
    the_yun_tai.Pitch_pid->f_param_init(the_yun_tai.Yaw_pid, PID_Speed, MAX_YAW_OUT,
                                        MAX_YAW_OUT, 0.0f, 0.0f, 0.0f, 0.0f);


    // 赋值
    the_msb.the_yun_tai = &the_yun_tai;
}
