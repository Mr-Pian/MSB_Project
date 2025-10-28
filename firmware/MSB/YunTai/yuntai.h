/*******
written by abo
2025.7.19-uestc-chengdu
*/
#ifndef YUNTAI_H
#define YUNTAI_H
#include "stdint.h"
#include "string.h"
/*云台电机组成*/
#define MOTOR_YAW    	1
#define MOTOR_PITCH   2
#define MOTOR_CAMERA 	4

/*电机旋转角度限幅*/
#define MOTOR_YAW_RIGHT_MAX   8200
#define MOTOR_YAW_LEFT_MAX    -8200
#define MOTOR_PITCH_UP_MAX    -4000
#define MOTOR_PITCH_DOWN_MAX  4000
#define MOTOR_CAMERA_RIGHT_MAX	1000
#define MOTOR_CAMERA_LEFT_MAX		-1000

typedef struct
{
    uint8_t motor_param_flag; //云台调参
    uint8_t stable_control_flag; //云台自稳
    uint8_t motor_reset_flag; //云台复位
    uint8_t pid_control_flag; //云台PID控制
} YUNTAI_TIM_Control_Flag_Typedef;

void yuntai_control_driver(void);
void yuntai_turn_off(void);
void yuntai_set_zero_point(void);
void yuntai_set_pos_angle(uint8_t motor_id, float pos_angle_degree);
void yuntai_reset(void);

extern YUNTAI_TIM_Control_Flag_Typedef the_yuntai;
#endif

