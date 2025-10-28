/**** written by abo --2025.7.23
* pid控制器
* 使用PID_TypeDef声明实例，在控制器初始化中调用pid_init初始化每个pid实例,同时调用pid_switch,使能pid
* pid计算建议在定时器中断中定期调用
* 支持位置式pid和增量式pid
*/
#ifndef _PID_H
#define _PID_H
/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

typedef enum
{
	PID_Position,
	PID_Speed,
}PID_KIND;

typedef struct PID_Cotroller
{
	PID_KIND kind;
	uint8_t enable;			//使能开关
	float target;			//控制目标
	float measure;			//测量反馈
	float now_err;			//当前偏差
	float last_err;     	//上一次偏差
  	float far_err;      	//上上次偏差	
	float kp;				//比例
	float ki;				//积分
	float kd;				//微分
	float pout;	
	float iout;
	float dout;
	
	float output;			//控制器当前输出

	float MaxOutput;		//控制器输出限幅
	float IntegralLimit;	//控制器积分限幅
	
	void (*f_param_init)(volatile struct PID_Cotroller *pid,  //PID params initiate
				PID_KIND kind,
				float maxOutput,
				float integralLimit,
				float  target,
				float kp,
				float ki,
				float kd);
				   
	void (*f_pid_param_change)(volatile struct PID_Cotroller *pid, float kp,float ki, float kd);		//pid change params 
	float (*f_cal_pid)(volatile struct PID_Cotroller *pid, float measure);   //pid compute
	void (*f_pid_switch)(volatile struct PID_Cotroller *pid,uint8_t switch_value); 	//pid switch
}PID_Controller_TypeDef;


void pid_init(volatile PID_Controller_TypeDef* pid);
#endif
