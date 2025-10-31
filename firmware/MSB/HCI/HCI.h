//
// Created by maodie on 2025/10/31.
//

#ifndef MSB_HCI_H
#define MSB_HCI_H

#include "bits_button.h"
#include "main.h"

#define KEY1_GPIO_PORT GPIOC
#define KEY1_GPIO_PIN GPIO_PIN_13

// Add your macro definitions and declarations here
#define  BUTTON_1 0x01

uint8_t read_button_pin(uint8_t button_id);
void HCI_init(void);
void Handle_btn_event(void);

extern struct button_obj_t ecbt;
#endif //MSB_HCI_H
