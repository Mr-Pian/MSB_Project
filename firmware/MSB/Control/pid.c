/**** written by abo --2025.7.23
* pid控制器
* 使用PID_TypeDef声明实例，在控制器初始化中调用pid_init初始化每个pid实例,同时调用pid_switch,使能pid
* pid计算建议在定时器中断中定期调用
* 支持位置式pid和增量式pid
*/
/* Includes ------------------------------------------------------------------*/
#include "pid.h"
#include "math.h"
/*参数初始化--------------------------------------------------------------*/
static void pid_param_init(
	volatile PID_Controller_TypeDef * pid, //pid结构体
	PID_KIND   kind,			//PID类型
	float maxout,				//PID输出限幅
	float intergral_limit,		//积分限幅
	float  target,				//PID目标
	float 	kp, 							
	float 	ki, 
	float 	kd)
{
	pid->kind = kind;		
	pid->IntegralLimit = intergral_limit;
	pid->MaxOutput = maxout;
	pid->target = target;
	pid->kp = kp;
	pid->ki = ki;
	pid->kd = kd;
	pid->output = 0;
}

/*PID开启:switch_value=1 or关闭:switch_value=0*/
static void pid_switch(volatile PID_Controller_TypeDef * pid,uint8_t switch_value)
{
	pid->enable =switch_value;
}


/*中途更改参数设定--------------------------------------------------------------*/
static void pid_param_change(volatile PID_Controller_TypeDef * pid, float kp, float ki, float kd)
{
	pid->kp = kp;
	pid->ki = ki;
	pid->kd = kd;
}

/*pid计算-----------------------------------------------------------------------*/	
static float pid_calculate(volatile PID_Controller_TypeDef* pid, float measure)
{
	if(pid->enable)//enable the PID Controller
	{
		//更新测量值
		pid->measure = measure;
		//更新历史偏差
		pid->far_err = pid->last_err;	
		pid->last_err  = pid->now_err;
		//计算当前偏差
		pid->now_err = pid->target - pid->measure;
		
		if(pid->kind==PID_Position) //位置式PID
		{ 
			pid->pout = pid->kp * pid->now_err;//比例
			pid->iout += (pid->ki * pid->now_err);//积分
			pid->dout =  pid->kd * (pid->now_err - pid->last_err);//微分 
			
			//积分是否超出限制
			if(pid->iout > pid->IntegralLimit)
				pid->iout = pid->IntegralLimit;
			if(pid->iout < - pid->IntegralLimit)
				pid->iout = - pid->IntegralLimit;

			//pid输出和
			pid->output = pid->pout + pid->iout + pid->dout;//绝对输出
	
			/***输出限幅***/
			if(pid->output>pid->MaxOutput)         
			{
				pid->output = pid->MaxOutput;
			}
			else if(pid->output < -(pid->MaxOutput))
			{
				pid->output = -(pid->MaxOutput);
			}
		}
		else if(pid->kind==PID_Speed)//增量式PID
		{ 
			pid->pout = pid->kp * (pid->now_err - pid->last_err);
			pid->iout = pid->ki * (pid->now_err);
			pid->dout = pid->kd * (pid->now_err - 2*pid->last_err + pid->far_err); 
			//pid输出和
			pid->output += pid->pout + pid->iout + pid->dout;//积分输出(外部执行器无需再进行增量积分)

			//输出限幅
			if(pid->output > pid->MaxOutput)         
			{
				pid->output = pid->MaxOutput;
			}
			else if(pid->output < -(pid->MaxOutput))
			{
				pid->output = -(pid->MaxOutput);
			}
		}	
	}
	else//unable the PID Controller
	{
		/*reset all the variable*/
		pid->output=0;
		pid->pout=0;
		pid->iout=0;
		pid->dout=0;
		pid->far_err = 0;
		pid->last_err = 0;
		pid->now_err = 0;

	}
	return pid->output;
}

/*pid结构体初始化，每一个pid参数需要调用一次-----------------------------------------------------*/
void pid_init(volatile PID_Controller_TypeDef* pid)
{
	pid->f_param_init = pid_param_init;
	pid->f_pid_param_change = pid_param_change;
	pid->f_cal_pid = pid_calculate;
	pid->f_pid_switch = pid_switch;
}



