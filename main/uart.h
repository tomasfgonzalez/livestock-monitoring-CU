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
#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "driver/uart.h"

/* Macros --------------------------------------------------------------------*/
#define UART1_PORT_NUM       UART_NUM_1
#define UART1_TXD             17
#define UART1_RXD             16

#define UART2_PORT_NUM       UART_NUM_2
#define UART2_TXD            27
#define UART2_RXD            26

#define UART_RTS             UART_PIN_NO_CHANGE
#define UART_CTS             UART_PIN_NO_CHANGE

#define UART_BAUD_RATE       115200
#define UART_TASK_STACK_SIZE 2048

#define UART_RX_BUFF_SIZE    128

/* Structs -------------------------------------------------------------------*/
typedef enum {
  UART_PORT_MAIN = 0,
  UART_PORT_AUX = 1,
} UartPort_t;

/* Function prototypes -------------------------------------------------------*/
void uart_init(void);

void uart_send(const char *data, uint16_t length, UartPort_t port);

uint16_t uart_receive(UartPort_t port);

uint8_t* uart_get_rx_buff(UartPort_t port);

#ifdef __cplusplus
}
#endif

#endif /* __UART_H */
