//
// Created by pickaxehit on 2025/11/12.
//

#include "usart.h"
#include "gpio.h"
#include "laser_control.h"

#define LASER_GPIO_PORT GPIOA
#define LASER_GPIO_PIN GPIO_PIN_12
#define LASER_UART &huart4

laser_mode_t laser_mode_status = LASER_MODE_UART;

void laser_mode_set(laser_mode_t laser_mode) {
    if (laser_mode == laser_mode_status) {
        return;
    }
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (laser_mode == LASER_MODE_UART) {
        GPIO_InitStruct.Pin = LASER_GPIO_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
        HAL_GPIO_Init(LASER_GPIO_PORT, &GPIO_InitStruct);
    } else if (laser_mode == LASER_MODE_GPIO) {
        GPIO_InitStruct.Pin = LASER_GPIO_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(LASER_GPIO_PORT, &GPIO_InitStruct);
    }
    laser_mode_status = laser_mode;
}

void laser_set_level(GPIO_PinState level) {
    laser_mode_set(LASER_MODE_GPIO);
    HAL_GPIO_WritePin(LASER_GPIO_PORT, LASER_GPIO_PIN, level);
}

void laser_transmit_data(uint8_t *data, size_t len) {
    laser_mode_set(LASER_MODE_UART);
    HAL_UART_Transmit(LASER_UART, data, len,HAL_MAX_DELAY);
}


