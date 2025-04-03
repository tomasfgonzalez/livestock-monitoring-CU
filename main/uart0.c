/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : uart0.c
  * @brief          : Source for uart0 file
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

/* Includes ------------------------------------------------------------------*/
#include "uart0.h"

/* Private variables ---------------------------------------------------------*/
void uart0_init(uint16_t buf_size) {
  uart_config_t uart_config = {
        .baud_rate = UART0_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

  ESP_ERROR_CHECK(uart_driver_install(UART0_PORT_NUM, buf_size * 2, 0, 0, NULL, intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(UART0_PORT_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART0_PORT_NUM, UART0_TXD, UART0_RXD, UART0_RTS, UART0_CTS));
}

void uart0_send(uint8_t *data, uint16_t length) {
  uart_write_bytes(UART0_PORT_NUM, (const char *) data, length);
}

uint16_t uart0_receive(uint8_t *data, uint16_t length) {
  return uart_read_bytes(UART0_PORT_NUM, data, length, 2);
}
