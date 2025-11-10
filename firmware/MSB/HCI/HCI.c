//
// Created by maodie on 2025/10/31.
//
#include "HCI.h"
#include "control.h"
#include "lcd.h"
#include "stm32h7xx_hal_gpio.h"
#include "xprintf.h"
#include "usart.h"

//必要结构体定义
struct button_obj_t ecbt;
MSB_data_typedef MSB_Data; //存储数据结构体

//必要变量定义
uint8_t pid_control_flag = 0;
uint8_t data = 0;
uint8_t SD_Pop_flag = 0;

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
            set_yuntai_flag(PID_CONTROL_FLAG);
            pid_control_flag = 1;
        }
        else if (data == 0x2a)
        {
            SD_Pop_flag = 1;
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
            the_yun_tai.Pitch_pid->output = 0.0f;
            the_yun_tai.Yaw_pid->output = 0.0f;
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
    //从SD卡读之前保存的参数
    if (Read_SD_data(&MSB_Data))
    {
        Error_Handler();
    }
}

/*******************************SD 卡读取和保存部分 **************************************/
// 从SD卡里读数据，在文件起始处调用一次,把所有数据读入到栈里
/***********************FATfs所需变量（堆中定义）************************/
FATFS myFatFs; // FatFs 文件系统对象; 这个结构体占用598字节，有点大，需用static修饰(存放在全局数据区), 避免stack溢出
FIL myFile; // 文件对象; 这个结构体占用570字节，有点大，需用static修饰(存放在全局数据区), 避免stack溢出
FRESULT f_res; // 文件操作结果
uint8_t aReadData[1024] = {0}; // 读取缓冲区; 这个数组占用1024字节，需用static修饰(存放在全局数据区), 避免stack溢出
uint8_t aWriteBuf[] = "测试; This is FatFs Test ! \r\n"; // 要写入的数据
/***********************       定义结束       ************************/
uint8_t Read_SD_data(MSB_data_typedef* msb_data)
{
    /**********************首次启动挂载文件系统***************************/
    // 重要的延时：避免烧录期间的复位导致文件读写、格式化等错误
    delay_ms(1000); // 重要：稍作延时再开始读写测试; 避免有些仿真器烧录期间的多次复位，短暂运行了程序，导致下列读写数据不完整。
    f_res = f_mount(&myFatFs, "0:", 1); // 在SD卡上挂载文件系统; 参数：文件系统对象、驱动器路径、读写模式(0只读、1读写)
    if (f_res != FR_OK) // 挂载异常
    {
        return 1;
    }
    /**********************    挂载结束     ***************************/

    f_res = f_open(&myFile, FILENAME, FA_OPEN_EXISTING | FA_READ);
    // 打开文件; 参数：文件对象、路径和名称、操作模式; FA_OPEN_EXISTING：只打开已存在的文件; FA_READ: 以只读的方式打开文件
    if (f_res == FR_OK)
    {
        uint8_t recognizer = 0; //校验字
        if (f_gets((TCHAR*)aReadData, sizeof(aReadData), &myFile) == (TCHAR*)aReadData) //读取第一行数据（到\n）到缓冲区areaddata
        {
            float temp[2] = {0};
            int items_scanned = sscanf((char*)aReadData,
                                       "%e,%e,%e,%e,%e,%e,%e,%e,%e,%e,%d",
                                       temp,
                                       &temp[1],
                                       &msb_data->Max_x_output,
                                       &msb_data->Max_y_output,
                                       &msb_data->yaw_kp,
                                       &msb_data->yaw_ki,
                                       &msb_data->yaw_kd,
                                       &msb_data->pitch_kp,
                                       &msb_data->pitch_ki,
                                       &msb_data->pitch_kd,
                                       (int*)&recognizer);
            msb_data->pixel_x_target = (uint16_t)temp[0];
            msb_data->pixel_y_target = (uint16_t)temp[1];
            if (recognizer != 91 || items_scanned != 11)
            {
                f_close(&myFile);
                f_mount(NULL, "0:", 1);
                return 1; //未通过校验
            }
        }
        else
        {
            f_close(&myFile);
            f_mount(NULL, "0:", 1);
            return 1;
        }
    }
    else
    {
        f_close(&myFile);
        f_mount(NULL, "0:", 1);
        return 1;
    }

    f_close(&myFile); // 不再读写，关闭文件
    //f_mount(NULL, "0:", 1); // 不卸载文件系统
    return 0;
}

char data_buffer[256];

uint8_t Write_SD_data(MSB_data_typedef* msb_data)
{
    f_res = f_open(&myFile, FILENAME, FA_WRITE | FA_CREATE_ALWAYS);
    if (f_res != FR_OK)
    {
        return 1; // 打开文件失败
    }
    xsprintf(data_buffer,
             "%e,%e,%e,%e,%e,%e,%e,%e,%e,%e,91\n",
             (float)msb_data->pixel_x_target,
             (float)msb_data->pixel_y_target,
             msb_data->Max_x_output,
             msb_data->Max_y_output,
             msb_data->yaw_kp,
             msb_data->yaw_ki,
             msb_data->yaw_kd,
             msb_data->pitch_kp,
             msb_data->pitch_ki,
             msb_data->pitch_kd
    );

    f_res = f_puts(data_buffer, &myFile); //写入

    if (f_res != FR_OK)
    {
        f_close(&myFile);
        return 1; // 写入错误
    }

    f_res = f_close(&myFile);
    if (f_res != FR_OK)
    {
        return 1; // 关闭失败
    }

    // 5. 只有所有步骤都 OK，才算成功
    return 0;
}

//弹出SD卡
void Pop_sd(void)
{
    f_mount(NULL, "0:", 1);
}

/****************************SD 卡读取和保存部分 结束 ***********************************/
