//
// Created by pickaxehit on 2025/11/12.
//

#ifndef MSB_LASER_CONTROL_H
#define MSB_LASER_CONTROL_H

typedef enum
{
    LASER_MODE_GPIO = 0,
    LASER_MODE_UART = 1,
} laser_mode_t;

void laser_mode_set(laser_mode_t laser_mode);
void laser_set_level(GPIO_PinState level);
void laser_transmit_data(uint8_t *data, size_t len);

extern laser_mode_t laser_mode_status;

#endif //MSB_LASER_CONTROL_H