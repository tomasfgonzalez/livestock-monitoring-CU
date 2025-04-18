#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/mpu_wrappers.h>
#include <freertos/projdefs.h>

#include "uart.h"
#include "rylr998.h"
#include "rx_buff.h"
#include "esp_log.h"
#include <string>
#include "request_queue.h"
#include "LSUManager.h"
#include "cu_comms.h"

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

      // Create a string from the received bytes
      std::string received_data(reinterpret_cast<char*>(rx_buff), rxBytes);
      ESP_LOGI(RX_TASK_TAG, "Received data as string: %s", received_data.c_str());
      
      // Check if this is a request that needs to be parsed
      if (received_data.starts_with("+RCV=")) {
        received_data = received_data.substr(5);
        post_request(received_data);
        ESP_LOGI(RX_TASK_TAG, "Request parsed and added to queue");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

static void process_request_task(void *arg) {
  static const char *PROCESS_REQUEST_TASK_TAG = "PROCESS_REQUEST_TASK";
  ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "Request processing task started");

  LSUManager manager;

  while (1) {
    Request* request = get_request();
    if (request != NULL) {
      ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "Processing request from ID: %lu, Type: %d", request->from_id, request->type);
      
      // All enum values are handled in this switch statement
      switch (request->type) {
        case REQUEST_TYPE_SYNC: {
          ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "Sync request received");
          auto [lsu, success] = manager.createLSU();
          if (success) {
            ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "LSU created successfully");
            
            uint32_t lsu_id = lsu->getId();
            uint32_t lsu_id_to_send = request->from_id;
            LSU_config_package_t config_package(
              lsu_id,
              5400,
              1800000
            );
            CU_sendConfigPackage(&config_package, lsu_id_to_send);
          } else {
            ESP_LOGW(PROCESS_REQUEST_TASK_TAG, "Failed to create LSU");
          }
          break;
        }
        case REQUEST_TYPE_DATA: {
          ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "Data request received: %s", request->data.c_str());
          // Handle data request
          break;
        }

        default:
          [[deprecated]];
          ESP_LOGW(PROCESS_REQUEST_TASK_TAG, "Unknown request type: %d", request->type);
          break;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}


extern "C" void app_main(void) {
  static const char *MAIN_TAG = "MAIN_RUN";

  ESP_LOGI(MAIN_TAG, "Starting main");
  uart_init(RX_BUFF_SIZE, UART1_PORT_NUM);

  xTaskCreate(rx_task, "uart_rx_task", 1024 * 4, NULL, configMAX_PRIORITIES - 1, NULL);
  xTaskCreate(process_request_task, "process_request_task", 1024 * 4, NULL, configMAX_PRIORITIES - 1, NULL);
  rylr998_setChannel(1, 0x01);

  while (1) {
    CU_sendTest();
    ESP_LOGI(MAIN_TAG, "Config package sent");
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}
