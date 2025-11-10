//
// Created by maodie on 2025/10/31.
//

#ifndef MSB_HCI_H
#define MSB_HCI_H

#include "bits_button.h"
#include "main.h"
#include "delay/delay.h"
#include "fatfs.h"

#define KEY1_GPIO_PORT GPIOC
#define KEY1_GPIO_PIN GPIO_PIN_13

// Add your macro definitions and declarations here
#define  BUTTON_1 0x01

//Fatfs 文件名
#define FILENAME "0:MSB_parameters.txt"

typedef struct
{
    // 预瞄点坐标
    uint16_t pixel_x_target;
    uint16_t pixel_y_target;
    // 云台转动角度限幅
    float Max_x_output;
    float Max_y_output;
    // x 方向云台转动pid
    float yaw_kp;
    float yaw_ki;
    float yaw_kd;
    // y 方向云台转动pid
    float pitch_kp;
    float pitch_ki;
    float pitch_kd;
} MSB_data_typedef; //从SD卡中读取的数据（同时也在这个结构题里做修改）

uint8_t read_button_pin(uint8_t button_id);
void HCI_init(void);
void Handle_btn_event(void);
uint8_t Read_SD_data(MSB_data_typedef* msb_data);
uint8_t Write_SD_data(MSB_data_typedef* msb_data);
void Pop_sd(void);

extern struct button_obj_t ecbt;
extern MSB_data_typedef MSB_Data; //存储数据结构体
extern uint8_t SD_Pop_flag;
#endif //MSB_HCI_H
