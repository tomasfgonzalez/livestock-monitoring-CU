#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/mpu_wrappers.h>
#include <freertos/projdefs.h>
#include "uart.h"
#include "rylr998.h"
#include "rx_buff.h"
#include "esp_log.h"


static void rx_task(void *arg) {
  static const char *RX_TASK_TAG = "RX_TASK";
  ESP_LOGI(RX_TASK_TAG, "RX task started");

  uint8_t* rx_buff = rx_buff_get();
  while (1) {
    const int rxBytes = uart_read_bytes(UART_NUM_1, rx_buff, RX_BUFF_SIZE, 1000 / portTICK_PERIOD_MS);
    if (rxBytes > 0) {
      rx_buff[rxBytes] = 0;
      rylr998_SetInterruptFlag(true);
      ESP_LOGI(RX_TASK_TAG, "Read %d bytes: %s", rxBytes, rx_buff);
      // ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, rx_buff, rxBytes, ESP_LOG_INFO);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

extern "C" void app_main(void) {
  static const char *MAIN_TAG = "MAIN_RUN";

  ESP_LOGI(MAIN_TAG, "Starting main");
  uart_init(RX_BUFF_SIZE, UART1_PORT_NUM);

  xTaskCreate(rx_task, "uart_rx_task", 1024 * 4, NULL, configMAX_PRIORITIES - 1, NULL);
  rylr998_setChannel(1, 0x01);

  while (1) {
    CU_sendConfigPackage();
    ESP_LOGI(MAIN_TAG, "Config package sent");
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}