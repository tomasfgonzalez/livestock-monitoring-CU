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
#define UART1_PORT_NUM       UART_NUM_1
#define UART1_TXD             10
#define UART1_RXD             9

#define UART2_PORT_NUM       UART_NUM_2
#define UART2_TXD             17
#define UART2_RXD             16

#define UART_RTS             UART_PIN_NO_CHANGE
#define UART_CTS             UART_PIN_NO_CHANGE

#define UART_BAUD_RATE       115200
#define UART_TASK_STACK_SIZE 2048

/* Function prototypes -------------------------------------------------------*/
void uart_init(uint16_t buf_size, uint8_t port_num);

void uart1_send(uint8_t *data, uint16_t length);
uint16_t uart1_receive(uint8_t *data, uint16_t length);

void uart2_send(uint8_t *data, uint16_t length);
uint16_t uart2_receive(uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __UART0_H */
