#include "main.h"// 假设包含HAL库和外设定义
#include "ssd1306.h"
#include "tim.h"
#include "usart.h"// 假设定义了huart2

#include <string.h>

uint8_t rx_data;
uint8_t rx_buffer[128];
uint8_t rx_idx = 0;
typedef enum {
    RX_IDLE = 0,
    RX_DATA = 1,
    RX_MUSIC = 2,
} rx_status_t;
uint8_t refresh_flag = 0;
rx_status_t rx_status = RX_IDLE;

uint32_t baud_rates[9] = {9600, 7200, 4800, 3600, 2400, 1800, 1200, 600, 300};
uint8_t baud_rate_idx = 0;

uint32_t idle_counter = 0;

const uint32_t note_arr_table[] = {
    15290,// [0] -> 对应 rx_data = 0x01 (C5: 523.25 Hz)
    13621,// [1] -> 对应 rx_data = 0x02 (D5: 587.33 Hz)
    12135,// [2] -> 对应 rx_data = 0x03 (E5: 659.26 Hz)
    11454,// [3] -> 对应 rx_data = 0x04 (F5: 698.46 Hz)
    10205,// [4] -> 对应 rx_data = 0x05 (G5: 783.99 Hz)
    9091, // [5] -> 对应 rx_data = 0x06 (A5: 880.00 Hz)
    8100  // [6] -> 对应 rx_data = 0x07 (B5: 987.77 Hz)
};
const char *note_str_table[] = {
    "Do\0\0\0", "Re\0\0\0", "Mi\0\0\0", "Fa\0\0\0", "Sol\0\0", "La\0\0\0", "Xi\0\0\0",
};


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        switch (rx_status) {
            case RX_IDLE: {
                if (rx_data == '\x1a') {
                    memset(rx_buffer, 0, sizeof(rx_buffer));
                    rx_idx = 0;
                    rx_status = RX_DATA;

                    idle_counter = 10000;
                } else if (rx_data == '\xaa') {
                    rx_status = RX_MUSIC;
                }
                break;
            }
            case RX_DATA: {
                if (rx_data == 0x0a) {
                    refresh_flag = 1;
                    rx_status = RX_IDLE;
                    break;
                }
                rx_buffer[rx_idx] = rx_data;
                rx_idx++;
                if (rx_idx >= sizeof(rx_buffer)) {
                    rx_idx = 0;
                }

                break;
            }
            case RX_MUSIC: {
                if (rx_data == 0x0a) {
                    rx_status = RX_IDLE;
                    break;
                }
                switch (rx_data) {
                    case 0x00: {
                        // 停止 PWM
                        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
                        memset(&rx_buffer, 0, sizeof(rx_buffer));
                        refresh_flag = 1;
                        break;
                    }
                    case 0x01:  // C6
                    case 0x02:  // D6
                    case 0x03:  // E6
                    case 0x04:  // F6
                    case 0x05:  // G6
                    case 0x06:  // A6
                    case 0x07: {// B6
                        // 统一处理逻辑
                        // 使用 rx_data - 1 作为数组索引 (0x01 -> 0, 0x07 -> 6)
                        uint32_t index = rx_data - 1;

                        memcpy(rx_buffer, note_str_table[index], 5);
                        uint32_t arr = note_arr_table[index];
                        // 设置 ARR 和 CCR
                        __HAL_TIM_SET_AUTORELOAD(&htim1, arr - 1);
                        __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, arr >> 1);// 50% 占空比

                        // 启动 PWM
                        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

                        refresh_flag = 1;
                        break;
                    }
                    default: {
                        rx_status = RX_IDLE;
                        memset(&rx_buffer, 0, sizeof(rx_buffer));
                        // 可选：处理无效的 rx_data 值，例如停止 PWM
                        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);

                        refresh_flag = 1;
                        break;
                    }
                }
                break;
            }
        }

        HAL_UART_Receive_IT(&huart2, &rx_data, 1);
    }
}
void user_main(void) {
    // HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_GPIO_WritePin(OLED_RESET_GPIO_Port, OLED_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(OLED_RESET_GPIO_Port, OLED_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
    ssd1306_init();

    // 2. 初始全清屏 (仅一次)
    ssd1306_clear(0);

    ssd1306_printf(0, 24, 1, 1, "baud: %d", huart2.Init.BaudRate);

    ssd1306_refresh();
    ssd1306_refresh();

    HAL_UART_Receive_IT(&huart2, &rx_data, 1);


    // 3. 主循环
    while (1) {
        if (refresh_flag) {
            ssd1306_clear_page(0, 0);
            ssd1306_clear_page(1, 0);
            ssd1306_clear_page(2, 0);
            ssd1306_draw_string_12_24(0, 0, rx_buffer, 1, 0);
            ssd1306_refresh();
            ssd1306_refresh();
            refresh_flag = 0;
        }
        if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(5);
            if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET) {
                baud_rate_idx++;
                if (baud_rate_idx >= 9) {
                    baud_rate_idx = 0;
                }
                huart2.Init.BaudRate = baud_rates[baud_rate_idx];
                HAL_UART_Init(&huart2);
                HAL_UART_Receive_IT(&huart2, &rx_data, 1);
                ssd1306_clear_page(3, 0);
                ssd1306_printf(0, 24, 1, 0, "baud: %d", huart2.Init.BaudRate);
                ssd1306_refresh();
                ssd1306_refresh();
                while (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET) {
                }
            }
        }
        if (idle_counter > 0) {
            idle_counter--;
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
            // HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
        } else {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
            // HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
        }
    }
}
