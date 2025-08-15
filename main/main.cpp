#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/mpu_wrappers.h>
#include <freertos/projdefs.h>
#include "esp_log.h"

#include "general_config.h"
#include "uart.h"
#include "nvs_flash.h"

#include "display/oled.h"
#include "display/status.h"

#include "lora/rylr998.h"

#include "tasks/rx_channel.h"
#include "tasks/process_requests.h"
#include "tasks/server_connection.h"
#include "tasks/heartbeat.h"

#include "wi-fi/wifi.h"
#include "wi-fi/mqtt_api.h"
#include "display/oled.h"

extern "C" void app_main(void) {
  static const char *MAIN_TAG = "App";
  ESP_LOGI(MAIN_TAG, "Starting CU");
  nvs_flash_init();
  oled_init();
  oled_welcome();
  uart_init();

  vTaskDelay(pdMS_TO_TICKS(3000));
  push_status_to_oled();

  // Init connectivity
  xTaskCreate(server_connection_task, "server_connection_task", 1024 * 8, NULL, configMAX_PRIORITIES - 1, NULL);

  // Start tasks
  static UartPort_t main_port = UART_PORT_MAIN;
  static UartPort_t aux_port = UART_PORT_AUX;
  xTaskCreate(rx_channel_task, "uart_main_rx_task", 1024 * 2, &main_port, configMAX_PRIORITIES - 2, NULL);
  xTaskCreate(rx_channel_task, "uart_aux_rx_task", 1024 * 2, &aux_port, configMAX_PRIORITIES - 2, NULL);
  xTaskCreate(process_requests_task, "process_request_task", 1024 * 4, NULL, configMAX_PRIORITIES - 2, NULL);
  xTaskCreate(heartbeat_task, "heartbeat_task", 1024 * 4, NULL, configMAX_PRIORITIES - 3, NULL);

  // Configure channels
  rylr998_setChannel(1, CU_ADDRESS, main_port);
  rylr998_setChannel(0, CU_ADDRESS, aux_port);
  while (1) {
    printf("--------------------------------\n");
    // ESP_LOGI(MAIN_TAG, "Sending config package");
    // CU_sendTest();
    // mqtt_api_publish("demo/room1", "Test!");
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}
