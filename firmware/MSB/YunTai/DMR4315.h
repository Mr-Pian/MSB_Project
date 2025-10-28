/*************
///海泰机电 DMR4315&DMR3515 云台电机 串口RS485驱动
///



/// 2023_7 V1.0
/// 2024_7 V2.0
/// Composed by
/// UESTC-School Of Automation Engineering-Technology Association-Turtle
**************/
#ifndef __DMR_4315_H
#define __DMR_4315_H
#include "stdint.h"
#include "string.h"

#define DMR_UART_HANDLER &huart8

typedef struct{
     uint16_t val1;  //单圈绝对值角度 串口原始数据
     int32_t val2;  //多圈绝对值角度 串口原始数据
     int16_t val3;  //转速rpm 串口原始数据
	float angle_s; //单圈绝对值角度
	float angle;   //多圈绝对值角度
	float rpm;     //转速rpm
     /*云台电机内部闭环控制器参数*/
     float pkp;   //位置环Kp
     float skp;   //速度环Kp
     float ski;   //速度环Ki
     int16_t tgt_spd;  //闭环目标速度  0.1rpm

}	MOTOR_STATE;

   
     
extern MOTOR_STATE yuntai_motor[3];
     
     
void motor_set_speed(uint8_t id,int speed);
void motor_set_position(uint8_t id,uint32_t pos);
void motor_set_related_position(uint8_t id,int16_t pos);
void motor_turn_off(uint8_t id);
void motor_set2zero(uint8_t id);
void motor_read_state(uint8_t id);
void yuntai_motor_rec(uint8_t cnt,uint8_t* _uart_rx_buff);
void motor_set_target_spd(uint8_t id,int16_t spd);
void motor_set_zero_point(uint8_t id);
void motor_set_param(uint8_t id,float pkp,float skp,float ski,float tgt_spd);
void motor_read_param(uint8_t id);
void motor_set_position_angle(uint8_t id,float angle);

#endif

