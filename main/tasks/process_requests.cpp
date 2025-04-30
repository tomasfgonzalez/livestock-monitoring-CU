/**
 * ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : process_requests.cpp
  * @brief          : Task - Processes requests from the request queue
  * ******************************************************************************
  */

/* Includes ------------------------------------------------------------ */
#include "process_requests.h"

#include "LSUManager.h"
#include "request_queue.h"
#include "esp_log.h"
#include "cu_comms.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Private variables --------------------------------------------------------- */
static const char *PROCESS_REQUEST_TASK_TAG = "PROCESS_REQUEST_TASK";

/* Private functions --------------------------------------------------------- */
void process_sync_request(Request* request, LSUManager& manager) {
  auto [lsu, success] = manager.createLSU();
  if (success) {
    ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "LSU created successfully");
    uint32_t lsu_id = lsu->getId();             // ID assigned to the LSU
    uint32_t lsu_id_to_send = request->from_id; // old ID of the sender, loses meaning after sync

    // TODO: Set actual params
    LSU_config_package_t config_package(
      lsu_id,
      5400,
      1800000
    );
    CU_sendConfigPackage(&config_package, lsu_id_to_send);
  } else {
    ESP_LOGW(PROCESS_REQUEST_TASK_TAG, "Failed to create LSU");
  }
}

/* Functions ------------------------------------------------------------ */
void process_requests_task(void *arg) {
  ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "Request processing task started");

  LSUManager manager;

  while (1) {
    Request* request = get_request();
    if (request != NULL) {
      ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "Processing request from ID: %u, Type: %d", request->from_id, request->type);
      
      // All enum values are handled in this switch statement
      switch (request->type) {
        case REQUEST_TYPE_SYNC: {
          ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "Sync request received");
          process_sync_request(request, manager);
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
