//
// Created by maodie on 2025/10/31.
//
#include "HCI.h"

#include "control.h"
#include "lcd.h"
#include "stm32h7xx_hal_gpio.h"
#include "usart.h"

//必要结构体定义
struct button_obj_t ecbt;

//必要变量定义
uint8_t pid_control_flag = 0;
uint8_t data = 0;

uint8_t read_button_pin(uint8_t button_id) //key底层
{
    uint8_t pin_state = 0;
    switch (button_id)
    {
    case BUTTON_1:
        pin_state = HAL_GPIO_ReadPin(KEY1_GPIO_PORT, KEY1_GPIO_PIN); // Replace with actual GPIO pin for BUTTON_1
        break;
    default:
        pin_state = 0; // Invalid button ID
        break;
    }
    return pin_state;
}

void Handle_btn_event(void) //event类型判断
{
    data = get_button_key_value(&ecbt);
    if (!pid_control_flag)
    {
        if (data == 0x07)
        {
            set_yuntai_flag(MOTOR_PARAM_FLAG);
        }
        else if (data == 0x0a) //进入pid模式
        {
            Laser_set_power(75);
            Laser_on_off(1);
            set_yuntai_flag(PID_CONTROL_FLAG);
            pid_control_flag = 1;
        }
        else if (data == 0x00)
        {
            set_yuntai_flag(STABLE_CONTROL_FLAG);
        }
    }
    else
    {
        if (data == 0x2a) //退出pid控制并清空输出
        {
            Laser_on_off(0);
            the_yun_tai.Pitch_pid->output = 0.0f; //所有输出全部置零
            the_yun_tai.Yaw_pid->output = 0.0f;
            the_yun_tai.Pitch_pid->iout = 0.0f;
            the_yun_tai.Yaw_pid->iout = 0.0f;
            the_yun_tai.Pitch_pid->pout = 0.0f;
            the_yun_tai.Yaw_pid->pout = 0.0f;
            set_yuntai_flag(STABLE_CONTROL_FLAG);
            pid_control_flag = 0;
        }
    }
}

void HCI_init(void)
{
    //按钮控制初始化
    button_init(&ecbt, read_button_pin, 1, BUTTON_1, NULL, 0);
    button_start(&ecbt);
}
