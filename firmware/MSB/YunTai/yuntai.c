/*******
written by abo
2025.7.19-uestc-chengdu
*/
#include "yuntai.h"
//#include "Control.h"
#include "DMR4315.h"
#include "delay/delay.h"
#include "control.h"
#include "HCI.h"
YUNTAI_TIM_Control_Flag_Typedef yuntai_flags;
uint8_t motor_send_delay_flag = 0;//两个云台电机总线发送延迟状态机
/*两轴云台控制
在10ms定时中断中调用
*/
//实际云台控制频率100hz
void yuntai_control_driver(void) {
    if (yuntai_flags_control_enable_debug_mode && yuntai_flags_control_enable_none_debug_mode) {
        if (yuntai_flags.motor_param_flag == 1) {
            //电机调参
            if (motor_send_delay_flag == 0) {
                motor_turn_off(MOTOR_YAW);
                motor_send_delay_flag = 1;
            } else if (motor_send_delay_flag == 1) {
                motor_turn_off(MOTOR_PITCH);
                motor_send_delay_flag = 0;
            }
        } else if (yuntai_flags.stable_control_flag == 1)//云台原地自稳控制
        {
            if (motor_send_delay_flag == 0) {
                motor_set_related_position(MOTOR_YAW, 0);
                motor_send_delay_flag = 1;
            } else if (motor_send_delay_flag == 1) {
                motor_set_related_position(MOTOR_PITCH, 0);
                motor_send_delay_flag = 0;
            }
        } else if (yuntai_flags.motor_reset_flag == 1)//云台发生错误，复位控制
        {
            if (motor_send_delay_flag == 0) {
                motor_set2zero(MOTOR_YAW);
                motor_send_delay_flag = 1;
            } else if (motor_send_delay_flag == 1) {
                motor_set2zero(MOTOR_PITCH);
                motor_send_delay_flag = 0;
            }
        } else if (yuntai_flags.pid_control_flag == 1)//云台PID控制
        {
            if (motor_send_delay_flag == 0) {
                //云台yaw PID驱动
                Laser_X_Control();
                motor_send_delay_flag = 1;
            } else if (motor_send_delay_flag == 1) {
                //云台pitch PID驱动
                Laser_Y_Control();
                motor_send_delay_flag = 0;
            }
        } else//云台回初始零点控制
        {
            if (motor_send_delay_flag == 0) {
                motor_set2zero(MOTOR_YAW);
                motor_send_delay_flag = 1;
            } else if (motor_send_delay_flag == 1) {
                motor_set2zero(MOTOR_PITCH);
                motor_send_delay_flag = 0;
            }
        }
    }
}

/*云台关闭*/
void yuntai_turn_off(void) {
    delay_ms(100);
    motor_turn_off(MOTOR_YAW);
    delay_ms(5);
    motor_turn_off(MOTOR_PITCH);
    delay_ms(5);
    motor_turn_off(MOTOR_CAMERA);
    delay_ms(5);
}

/*云台设置初始零点*/
void yuntai_set_zero_point(void) {
    delay_ms(100);
    motor_set_zero_point(MOTOR_YAW);
    delay_ms(5);
    motor_set_zero_point(MOTOR_PITCH);
    delay_ms(5);
}

/*云台回到初始零点*/
void yuntai_reset(void) {
    motor_set2zero(MOTOR_YAW);
    delay_ms(5);
    motor_set2zero(MOTOR_PITCH);
    delay_ms(5);
    motor_set2zero(MOTOR_CAMERA);
    delay_ms(5);
}

//自动设置操作，调用该函数设置以防止出现两个1, 不要定时重复调用
void yuntai_set_flag(uint8_t flag) {
    yuntai_flags.motor_param_flag = 0;
    yuntai_flags.motor_reset_flag = 0;
    yuntai_flags.pid_control_flag = 0;
    yuntai_flags.stable_control_flag = 0;
    if (flag == MOTOR_PARAM_FLAG) {
        yuntai_flags.motor_param_flag = 1;
    } else if (flag == MOTOR_RESET_FLAG) {
        yuntai_flags.motor_reset_flag = 1;
    } else if (flag == PID_CONTROL_FLAG) {
        yuntai_flags.pid_control_flag = 1;
    } else if (flag == STABLE_CONTROL_FLAG) {
        yuntai_flags.stable_control_flag = 1;
    }
}
