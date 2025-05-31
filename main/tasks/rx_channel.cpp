/**
  *******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : rx_channel.cpp
  * @brief          : Task - Receives data from some LoRa module at defined port
  *******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 Tomas Gonzalez & Brian Morris
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  * ******************************************************************************
  */

/* Includes ------------------------------------------------------------ */
#include "rx_channel.h"

#include "rylr998.h"
#include "request_queue.h"
#include "esp_log.h"
#include <string.h>

/* Private variables --------------------------------------------------------- */
static const std::string RX_CHANNEL_TASK_TAG_PREFIX = "RX_CHANNEL_TASK";

/* Functions ------------------------------------------------------------ */
void rx_channel_task(void *arg) {
  UartPort_t uart_port = *(UartPort_t*)arg;

  const std::string suffix = uart_port == UART_PORT_MAIN ? "MAIN" : "AUX";
  const std::string RX_CHANNEL_TASK_TAG_FULL = RX_CHANNEL_TASK_TAG_PREFIX + "_" + suffix;
  const char *RX_CHANNEL_TASK_TAG = RX_CHANNEL_TASK_TAG_FULL.c_str();

  ESP_LOGI(RX_CHANNEL_TASK_TAG, "RX task started for port %d", uart_port);

  uint8_t* rx_buff = uart_get_rx_buff(uart_port);
  while (1) {
    const int rxBytes = uart_receive(uart_port);
    if (rxBytes > 0) {
      rx_buff[rxBytes] = 0;
      ESP_LOGI(RX_CHANNEL_TASK_TAG, "[%d] %s", rxBytes, rx_buff);
      rylr998_SetInterruptFlag(true, uart_port);

      // Create a string from the received bytes
      std::string received_data(reinterpret_cast<char*>(rx_buff), rxBytes);
      
      // Check if this is a request that needs to be parsed
      if (received_data.starts_with("+RCV=")) {
        RYLR_RX_data_t* rcv_data = rylr998_getCommand(RYLR_RCV, uart_port);
        ESP_LOGI(RX_CHANNEL_TASK_TAG, "RCV data: %s", rcv_data->data);

        // Create a string from the received data in rcv_data->data
        std::string data(rcv_data->data);
        ESP_LOGI(RX_CHANNEL_TASK_TAG, "Converted to string: %s", data.c_str());
        post_request(data, rcv_data->id);
        ESP_LOGI(RX_CHANNEL_TASK_TAG, "Request parsed and added to queue");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}