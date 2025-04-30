/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : uart.c
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
#include "uart.h"
#include "esp_log.h"
/* Private variables ---------------------------------------------------------*/
static uint8_t rx_buff[2][UART_RX_BUFF_SIZE + 1];

static const uint8_t ports[2] = {
  UART1_PORT_NUM,
  UART2_PORT_NUM
};

/* Public functions ----------------------------------------------------------*/
void uart_init(void) {
  uart_config_t uart_config = {
    .baud_rate = UART_BAUD_RATE,
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

  /* Initialize UART1 */
  ESP_ERROR_CHECK(uart_driver_install(UART1_PORT_NUM, UART_RX_BUFF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(UART1_PORT_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART1_PORT_NUM, UART1_TXD, UART1_RXD, UART_RTS, UART_CTS));

  /* Initialize UART2 */
  ESP_ERROR_CHECK(uart_driver_install(UART2_PORT_NUM, UART_RX_BUFF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(UART2_PORT_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART2_PORT_NUM, UART2_TXD, UART2_RXD, UART_RTS, UART_CTS));
}

void uart_send(const char *data, uint16_t length, UartPort_t portIndex) {
  uint8_t port = ports[portIndex];
  uart_write_bytes(port, data, length);
}

uint16_t uart_receive(UartPort_t portIndex) {
  uint8_t port = ports[portIndex];
  return uart_read_bytes(port, rx_buff[portIndex], UART_RX_BUFF_SIZE, 2);
}

uint8_t* uart_get_rx_buff(UartPort_t portIndex) {
  return rx_buff[portIndex];
}
