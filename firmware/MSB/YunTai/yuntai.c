/*******
written by abo
2025.7.19-uestc-chengdu
*/
#include "yuntai.h"
//#include "Control.h"
#include "DMR4315.h"
#include "delay/delay.h"
YUNTAI_TIM_Control_Flag_Typedef the_yuntai;

/*两轴云台控制
在5ms定时中断中调用
*/
void yuntai_control_driver(void)
{
	static uint8_t motor_send_delay_flag = 0; //两个云台电机总线发送延迟状态机
	if (the_yuntai.motor_param_flag == 1)
	{
		//电机调参
	}
	else if (the_yuntai.stable_control_flag == 1) //云台原地自稳控制
	{
		if (motor_send_delay_flag == 2)
		{
			motor_set_related_position(MOTOR_YAW, 0);
			motor_send_delay_flag = 1;
		}
		else if (motor_send_delay_flag == 1)
		{
			motor_set_related_position(MOTOR_PITCH, 0);
			motor_send_delay_flag = 0;
		}
		else
		{
			motor_set_related_position(MOTOR_CAMERA, 0);
			motor_send_delay_flag = 2;
		}
	}
	else if (the_yuntai.motor_reset_flag == 1) //云台发生错误，复位控制
	{
		if (motor_send_delay_flag == 2)
		{
			motor_set2zero(MOTOR_YAW);
			motor_send_delay_flag = 1;
		}
		else if (motor_send_delay_flag == 1)
		{
			motor_set2zero(MOTOR_PITCH);
			motor_send_delay_flag = 0;
		}
		else
		{
			motor_set2zero(MOTOR_CAMERA);
			motor_send_delay_flag = 2;
		}
	}
	else if (the_yuntai.pid_control_flag == 1) //云台PID控制
	{
		// if (motor_send_delay_flag == 2)
		// {
		// 	//云台yaw PID驱动
		// 	Laser_yaw_set();
		// 	motor_send_delay_flag = 1;
		// }
		// else if (motor_send_delay_flag == 1)
		// {
		// 	//云台pitch PID驱动
		// 	Laser_pitch_set();
		// 	motor_send_delay_flag = 0;
		// }
		// else
		// {
		// 	//云台camera PID驱动
		// 	motor_send_delay_flag = 2;
		// }
	}
	else //云台回初始零点控制
	{
		if (motor_send_delay_flag == 2)
		{
			motor_set2zero(MOTOR_YAW);
			motor_send_delay_flag = 1;
		}
		else if (motor_send_delay_flag == 1)
		{
			motor_set2zero(MOTOR_PITCH);
			motor_send_delay_flag = 0;
		}
		else
		{
			motor_set2zero(MOTOR_CAMERA);
			motor_send_delay_flag = 2;
		}
	}
}

/*设定云台YAW、PITCH、CAMERA自由度角度位置*/
void yuntai_set_pos_angle(uint8_t motor_id, float pos_angle_degree)
{
	if (motor_id == MOTOR_YAW)
	{
		if (pos_angle_degree > (MOTOR_YAW_RIGHT_MAX / 360.0f * 16384))
			pos_angle_degree = (MOTOR_YAW_RIGHT_MAX / 360.0f * 16384);
		else if (pos_angle_degree < (MOTOR_YAW_LEFT_MAX / 360.0f * 16384))
			pos_angle_degree = (MOTOR_YAW_LEFT_MAX / 360.0f * 16384);
	}
	else if (motor_id == MOTOR_PITCH)
	{
		if (pos_angle_degree > (MOTOR_PITCH_DOWN_MAX / 360.0f * 16384))
			pos_angle_degree = (MOTOR_PITCH_DOWN_MAX / 360.0f * 16384);
		else if (pos_angle_degree < (MOTOR_PITCH_UP_MAX / 360.0f * 16384))
			pos_angle_degree = (MOTOR_PITCH_UP_MAX / 360.0f * 16384);
	}
	else if (motor_id == MOTOR_CAMERA)
	{
		if (pos_angle_degree > (MOTOR_CAMERA_RIGHT_MAX / 360.0f * 16384))
			pos_angle_degree = (MOTOR_CAMERA_RIGHT_MAX / 360.0f * 16384);
		else if (pos_angle_degree < (MOTOR_CAMERA_LEFT_MAX / 360.0f * 16384))
			pos_angle_degree = (MOTOR_CAMERA_LEFT_MAX / 360.0f * 16384);
	}
	else
	{
		return;
	}
	motor_set_position_angle(motor_id, pos_angle_degree);
}

/*云台关闭*/
void yuntai_turn_off(void)
{
	delay_ms(100);
	motor_turn_off(MOTOR_YAW);
	delay_ms(5);
	motor_turn_off(MOTOR_PITCH);
	delay_ms(5);
	motor_turn_off(MOTOR_CAMERA);
	delay_ms(5);
}

/*云台设置初始零点*/
void yuntai_set_zero_point(void)
{
	delay_ms(100);
	motor_set_zero_point(MOTOR_YAW);
	delay_ms(5);
	motor_set_zero_point(MOTOR_PITCH);
	delay_ms(5);
	motor_set_zero_point(MOTOR_CAMERA);
	delay_ms(5);
}

/*云台回到初始零点*/
void yuntai_reset(void)
{
	motor_set2zero(MOTOR_YAW);
	delay_ms(5);
	motor_set2zero(MOTOR_PITCH);
	delay_ms(5);
	motor_set2zero(MOTOR_CAMERA);
	delay_ms(5);
}

