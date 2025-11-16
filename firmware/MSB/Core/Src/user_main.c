//
// Created by pickaxehit on 2025/11/11.
//

#include "delay/delay.h"
#include "yuntai.h"
#include "control.h"
#include "HCI.h"
#include "TFT18_ST7735S.h"
#include "DMR4315.h"
#include "laser_control.h"
#include "tim.h"
#include "usart.h"

/************基本参数配置************/
#define TASK1_ERROR 20
/************基本参数配置************/

/************必要变量定义************/
uint8_t fifo_data[100] = {0};//树莓派串口接受缓冲区
/************必要变量定义************/

/************必要标志定义************/
uint8_t task2_laser_mode_set_flag = 0;//在基础第二问开始时设置一次UARTmode
uint8_t task1_laser_mode_set_flag = 0;//在基础第一问开始时设置一次UARTmode
/************必要标志定义************/

int user_main(void) {
    TFTSPI_Init();
    TFT_Printf(0, 0, COLOR_WHITE,COLOR_BLACK, fsize_8X16, "%5s", "FatFs Init:");
    TFT_Printf(0, 16, COLOR_WHITE,COLOR_BLACK, fsize_6X8, "%5s", "wait 1 sec");
    HCI_init();//人机交互初始化 (这里有读sd卡的1s延时)
    Control_Init();//控制初始化 (一定要放在HCI_init后面)
    HAL_UART_Receive_DMA(&huart7, fifo_data, 12);
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&huart2, buffer_from_screen, 100);
    HAL_TIM_Base_Start_IT(&htim14);//中低优先级定时器 5ms
    HAL_TIM_Base_Start_IT(&htim16);//高优先级定时器 1ms
    HAL_TIM_Base_Start_IT(&htim15);//中优先级定时器 2ms
    HAL_TIM_Base_Start_IT(&htim17);//低优先级定时器 100ms

    for (;;) {
        if (Fatfs_save_flag)//Fatfs保存
        {
            Fatfs_save_flag = 0;
            Write_SD_data(&MSB_Data);
        }
        if (pid_start_flag) {
            pid_start_flag = 0;
            pid_control_flag = 1;
            yuntai_set_zero_point();
            yuntai_set_flag(PID_CONTROL_FLAG);
        }
        switch (quest_num) {
            case -1: {
                // task1_laser_mode_set_flag = 0;//退出重置task1
                // task2_laser_mode_set_flag = 0;//退出重置task2
                break;
            }
            case 1: {
                //基础第一问
                if (!task1_laser_mode_set_flag) {
                    task1_laser_mode_set_flag = 1;
                    laser_mode_set(LASER_MODE_GPIO);
                }
                float err = the_yun_tai.Pitch_pid->now_err + the_yun_tai.Yaw_pid->now_err;
                if (err < TASK1_ERROR && err > -TASK1_ERROR) {
                    laser_set_level(1);
                } else {
                    laser_set_level(0);
                };
                break;
            }
            case 2: {
                //基础第二问
                if (!task2_laser_mode_set_flag) {
                    task2_laser_mode_set_flag = 1;
                    laser_mode_set(LASER_MODE_UART);
                }
                HAL_Delay(10);
                laser_transmit_data(laser_buffer, 6);//激光发送
                break;
            }
            default: {
                quest_num = 0;
                pid_stop_flag = 1;
                task1_laser_mode_set_flag = 0;//退出重置task1
                task2_laser_mode_set_flag = 0;//退出重置task2
                break;
            }
        }
        if (pid_stop_flag) {
            pid_stop_flag = 0;
            pid_control_flag = 0;

            the_yun_tai.Pitch_pid->output = 0.0f;
            the_yun_tai.Yaw_pid->output = 0.0f;
            yuntai_set_flag(STABLE_CONTROL_FLAG);
        }
    }
    return 0;
}