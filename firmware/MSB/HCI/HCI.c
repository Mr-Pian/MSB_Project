//
// Created by maodie on 2025/10/31.
//
#include "HCI.h"
#include "control.h"
#include "stm32h7xx_hal_gpio.h"
#include "xprintf.h"
#include "usart.h"
#include "TFT18_ST7735S.h"
#include "stdio.h"
#include "DMR4315.h"

//必要结构体定义
struct button_obj_t ecbt;
MSB_data_typedef MSB_Data;//存储数据结构体

//必要变量定义
uint8_t pid_start_flag = 0;
uint8_t pid_control_flag = 0;
uint8_t data = 0;
uint8_t SD_Pop_flag = 0;
uint8_t yuntai_flags_control_enable = 1;//flag使能控制 默认使能
uint8_t Fatfs_save_flag = 0;//fatfs 前后台保存flag
uint8_t pid_stop_flag = 0;
int8_t quest_num = 0;
uint8_t laser_buffer[20] = "helloworld\n";

//必要缓冲区定义
uint8_t buffer_from_screen[100] = {0};//串口屏接收缓冲区
/*******************************   Button 控制部分  ************************************/
uint8_t read_button_pin(uint8_t button_id)//key底层
{
    uint8_t pin_state = 0;
    switch (button_id) {
        case BUTTON_1:
            pin_state = HAL_GPIO_ReadPin(KEY1_GPIO_PORT, KEY1_GPIO_PIN);// Replace with actual GPIO pin for BUTTON_1
            break;
        default:
            pin_state = 0;// Invalid button ID
            break;
    }
    return pin_state;
}

void Handle_btn_event(void)//event类型判断
{
    data = get_button_key_value(&ecbt);
    if (!pid_control_flag) {
        if (data == 0x07) {
            quest_num = -1;
            yuntai_set_flag(MOTOR_PARAM_FLAG);
        } else if (data == 0x0a)//进入pid模式
        {
            // yuntai_set_flag(PID_CONTROL_FLAG);
            // pid_control_flag = 1;
            quest_num = -1;
            pid_start_flag = 1;
        } else if (data == 0x2a) {
            SD_Pop_flag = 1;
        } else if (data == 0x00) {
            quest_num = -1;
            yuntai_set_flag(STABLE_CONTROL_FLAG);
        }
    } else {
        if (data == 0x2a)//退出pid控制并清空输出
        {
            quest_num = 0;
            the_yun_tai.Pitch_pid->output = 0.0f;
            the_yun_tai.Yaw_pid->output = 0.0f;
            yuntai_set_flag(STABLE_CONTROL_FLAG);
            pid_control_flag = 0;
        }
    }
}

/*******************************   Button 控制结束  ************************************/
/*******************************   串口屏 状态机开始  ************************************/
void UART_Instru(uint8_t *uart_buffer, int buffer_length) {
    for (int i = 0; i < buffer_length; i++) {
        if (uart_buffer[i] == 0x91 && uart_buffer[i + 6] == 0x0A)//找到帧
        {
            switch (uart_buffer[i + 1]) {
                case 0x67: //上
                    /******* 调试界面 ********/
                    yuntai_flags_control_enable = 0;//暂时失能flag控制
                    motor_set_related_position(MOTOR_PITCH, -(int16_t) (10 * uart_buffer[i + 3]));
                    break;
                case 0x68: //左
                    yuntai_flags_control_enable = 0;//暂时失能flag控制
                    motor_set_related_position(MOTOR_YAW, -(int16_t) (10 * uart_buffer[i + 3]));
                    break;
                case 0x69: //右
                    yuntai_flags_control_enable = 0;//暂时失能flag控制
                    motor_set_related_position(MOTOR_YAW, (int16_t) (10 * uart_buffer[i + 3]));
                    break;
                case 0x70: //下
                    yuntai_flags_control_enable = 0;//暂时失能flag控制
                    motor_set_related_position(MOTOR_PITCH, (int16_t) (10 * uart_buffer[i + 3]));
                    break;
                case 0xCC: //确定
                    if (the_msb.the_pixel_target->real_pixel->pixel_x && the_msb.the_pixel_target->real_pixel->pixel_y)
                    //非0
                    {
                        MSB_Data.pixel_x_target = the_msb.the_pixel_target->real_pixel->pixel_x;
                        MSB_Data.pixel_y_target = the_msb.the_pixel_target->real_pixel->pixel_y;
                        the_msb.the_pixel_target->target_pixel->pixel_y = MSB_Data.pixel_y_target;
                        the_msb.the_pixel_target->target_pixel->pixel_x = MSB_Data.pixel_x_target;
                        Fatfs_save_flag = 1;//异步写入
                    }
                    break;
                case 0xaf: //退出调试界面标志
                    yuntai_flags_control_enable = 1;//重新使能flag控制
                    break;
                /******* 调试界面 ********/

                case 0xbb:
                    switch (uart_buffer[i + 2]) {
                        case 0x00: {
                            pid_stop_flag = 1;
                            quest_num = 0;
                            break;
                        }
                        case 0x01: {
                            quest_num = 1;
                            pid_start_flag = 1;
                            break;
                        }
                        case 0x02: {
                            quest_num = 2;
                            pid_start_flag = 1;
                            // memcpy(laser_buffer, uart_buffer + 3, 3);
                            break;

                        }
                        case 0x03: {

                            quest_num = 0;
                            pid_stop_flag = 1;
                            break;
                        }

                        default:
                            break;
                    }
                    break;
                default: //不应该发生
                    break;
            }
        }
    }
}

/*******************************   串口屏 状态机结束  ************************************/
/******************************* LCD 显示绘制部分 **************************************/
//Motor 状态绘制 0代表Lock 1代表pid 2代表free 3代表调参
void draw_motor(uint8_t state, uint8_t last_state) {
    if (state == 0) {
        if (last_state != state) {
            TFTSPI_Fill_Area(0, 60, 160, 80,COLOR_GREEN);
            TFT_Printf(0, 62, COLOR_WHITE,COLOR_GREEN, fsize_8X16, "%5s", "Motor:     Lock");
        }
    } else if (state == 1) {
        if (last_state != state) {
            TFTSPI_Fill_Area(0, 60, 160, 80,COLOR_YELLOW);
            TFT_Printf(0, 62, COLOR_BLACK,COLOR_YELLOW, fsize_8X16, "%5s", "Motor:      PID");
        }
    } else if (state == 2) {
        if (last_state != state) {
            TFTSPI_Fill_Area(0, 60, 160, 80,COLOR_RED);
            TFT_Printf(0, 62, COLOR_WHITE,COLOR_RED, fsize_8X16, "%5s", "Motor:     Free");
        }
    } else if (state == 3) {
        if (last_state != state) {
            TFTSPI_Fill_Area(0, 60, 160, 80,COLOR_BLUE);
            TFT_Printf(0, 62, COLOR_WHITE,COLOR_BLUE, fsize_8X16, "%5s", "Motor:    Debug");
        }
    }
}

//Info 绘制
void draw_info(uint8_t info) {
    if (info == 0)//清屏
    {
        TFTSPI_Fill_Area(0, 0, 142, 59,COLOR_BLACK);//清屏
        TFT_Printf(0, 0, COLOR_WHITE,COLOR_BLACK, fsize_8X16, "%s", "Sys info:");
    } else if (info == 1)//显示target坐标和树莓派回传坐标
    {
        TFT_Printf(0, 18, COLOR_LIME,COLOR_BLACK, fsize_6X8, "tgt: %d,%d",
                   MSB_Data.pixel_x_target, MSB_Data.pixel_y_target);
        TFT_Printf(80, 18,COLOR_RED,COLOR_BLACK, fsize_6X8, "real: %d,%d",
                   the_msb.the_pixel_target->real_pixel->pixel_x,
                   the_msb.the_pixel_target->real_pixel->pixel_y);
    } else if (info == 2)//显示PID输出
    {
        ;
    }
}

//Fatfs 状态绘制 1代表挂载 0代表卸载
//保留区域（143,0）（160,20）
void draw_sd(uint8_t sd_state) {
    if (sd_state == 1) {
        TFT_Printf(143, 0, COLOR_GREEN,COLOR_BLACK, fsize_8X16, "%s", "SD");
    } else if (sd_state == 0) {
        TFT_Printf(143, 0, COLOR_GRAY,COLOR_BLACK, fsize_8X16, "%s", "SD");
    }
}

/******************************* LCD 显示绘制结束 **************************************/

void HCI_init(void) {
    //按钮控制初始化
    button_init(&ecbt, read_button_pin, 1, BUTTON_1, NULL, 0);
    button_start(&ecbt);
    //从SD卡读之前保存的参数
    if (Read_SD_data(&MSB_Data)) {
        Error_Handler();
    }
    draw_sd(1);//文件系统挂载成功
    draw_info(0);//初始界面显示
}

/*******************************SD 卡读取和保存部分 **************************************/
// 从SD卡里读数据，在文件起始处调用一次,把所有数据读入到栈里
/***********************FATfs所需变量（堆中定义）************************/
FATFS myFatFs;// FatFs 文件系统对象; 这个结构体占用598字节，有点大，需用static修饰(存放在全局数据区), 避免stack溢出
FIL myFile;// 文件对象; 这个结构体占用570字节，有点大，需用static修饰(存放在全局数据区), 避免stack溢出
FRESULT f_res;// 文件操作结果
uint8_t aReadData[1024] = {0};// 读取缓冲区; 这个数组占用1024字节，需用static修饰(存放在全局数据区), 避免stack溢出
uint8_t aWriteBuf[] = "测试; This is FatFs Test ! \r\n";// 要写入的数据
/***********************       定义结束       ************************/
uint8_t Read_SD_data(MSB_data_typedef *msb_data) {
    /**********************首次启动挂载文件系统***************************/
    // 重要的延时：避免烧录期间的复位导致文件读写、格式化等错误
    delay_ms(1000);// 重要：稍作延时再开始读写测试; 避免有些仿真器烧录期间的多次复位，短暂运行了程序，导致下列读写数据不完整。
    f_res = f_mount(&myFatFs, "0:", 1);// 在SD卡上挂载文件系统; 参数：文件系统对象、驱动器路径、读写模式(0只读、1读写)
    if (f_res != FR_OK)// 挂载异常
    {
        return 1;
    }
    /**********************    挂载结束     ***************************/

    f_res = f_open(&myFile, FILENAME, FA_OPEN_EXISTING | FA_READ);
    // 打开文件; 参数：文件对象、路径和名称、操作模式; FA_OPEN_EXISTING：只打开已存在的文件; FA_READ: 以只读的方式打开文件
    if (f_res == FR_OK) {
        uint8_t recognizer = 0;//校验字
        if (f_gets((TCHAR *) aReadData, sizeof(aReadData), &myFile) == (TCHAR *) aReadData)//读取第一行数据（到\n）到缓冲区areaddata
        {
            float float_temp[2] = {0.0f};
            int items_scanned = sscanf((char *) aReadData,
                                       "%e,%e,%e,%e,%e,%e,%e,%e,%e,%e,%d",
                                       float_temp,
                                       &float_temp[1],
                                       &msb_data->Max_x_output,
                                       &msb_data->Max_y_output,
                                       &msb_data->yaw_kp,
                                       &msb_data->yaw_ki,
                                       &msb_data->yaw_kd,
                                       &msb_data->pitch_kp,
                                       &msb_data->pitch_ki,
                                       &msb_data->pitch_kd,
                                       (int *) &recognizer);

            msb_data->pixel_x_target = (uint16_t) float_temp[0];
            msb_data->pixel_y_target = (uint16_t) float_temp[1];

            if (recognizer != 91 || items_scanned != 11) {
                f_close(&myFile);
                f_mount(NULL, "0:", 1);
                return 1;//未通过校验
            }
        } else {
            f_close(&myFile);
            f_mount(NULL, "0:", 1);
            return 1;
        }
    } else {
        f_close(&myFile);
        f_mount(NULL, "0:", 1);
        return 1;
    }

    f_close(&myFile);// 不再读写，关闭文件
    //f_mount(NULL, "0:", 1); // 不卸载文件系统
    return 0;
}

char data_buffer[256];

uint8_t Write_SD_data(MSB_data_typedef *msb_data) {
    f_res = f_open(&myFile, FILENAME, FA_WRITE | FA_CREATE_ALWAYS);
    if (f_res != FR_OK) {
        return 1;// 打开文件失败
    }
    xsprintf(data_buffer,
             "%e,%e,%e,%e,%e,%e,%e,%e,%e,%e,91\n",
             (float) msb_data->pixel_x_target,
             (float) msb_data->pixel_y_target,
             msb_data->Max_x_output,
             msb_data->Max_y_output,
             msb_data->yaw_kp,
             msb_data->yaw_ki,
             msb_data->yaw_kd,
             msb_data->pitch_kp,
             msb_data->pitch_ki,
             msb_data->pitch_kd
        );

    f_res = f_puts(data_buffer, &myFile);//写入

    if (f_res != FR_OK) {
        f_close(&myFile);
        return 1;// 写入错误
    }

    f_res = f_close(&myFile);
    if (f_res != FR_OK) {
        return 1;// 关闭失败
    }

    // 5. 只有所有步骤都 OK，才算成功
    return 0;
}

//弹出SD卡
void Pop_sd(void) {
    f_mount(NULL, "0:", 1);
}

/****************************SD 卡读取和保存部分 结束 ***********************************/
