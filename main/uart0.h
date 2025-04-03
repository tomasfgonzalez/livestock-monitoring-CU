/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : uart0.h
  * @brief          : Header for uart0 file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART0_H
#define __UART0_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "driver/uart.h"

/* Macros --------------------------------------------------------------------*/
#define UART0_TXD             1
#define UART0_RXD             3
#define UART0_RTS             UART_PIN_NO_CHANGE
#define UART0_CTS             UART_PIN_NO_CHANGE

#define UART0_PORT_NUM        1
#define UART0_BAUD_RATE       9600
#define UART0_TASK_STACK_SIZE 2048

/* Function prototypes -------------------------------------------------------*/
void uart0_init(uint16_t buf_size);
void uart0_send(uint8_t *data, uint16_t length);
uint16_t uart0_receive(uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __UART0_H */
