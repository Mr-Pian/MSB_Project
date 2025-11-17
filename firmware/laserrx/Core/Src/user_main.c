#include "main.h"// 假设包含HAL库和外设定义
#include "ssd1306.h"
#include "usart.h"// 假设定义了huart2

#include <string.h>

uint8_t rx_data;
uint8_t rx_buffer[128];
uint8_t rx_idx = 0;
typedef enum {
    RX_IDLE = 0,
    RX_DATA = 1,
} rx_status_t;
uint8_t refresh_flag = 0;
rx_status_t rx_status = RX_IDLE;

uint32_t baud_rates[9] = {9600, 7200, 4800, 3600, 2400, 1800, 1200, 600, 300};
uint8_t baud_rate_idx = 0;

uint32_t idle_counter = 0;


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        switch (rx_status) {
            case RX_IDLE: {
                if (rx_data == '\x1a') {
                    memset(rx_buffer, 0, sizeof(rx_buffer));
                    rx_idx = 0;
                    rx_status = RX_DATA;

                    idle_counter = 10000;
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
        }

        HAL_UART_Receive_IT(&huart2, &rx_data, 1);
    }
}
void user_main(void) {
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
        } else {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        }
    }
}
