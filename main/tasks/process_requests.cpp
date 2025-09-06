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
#include "lsu_nvs_persistence.h"
#include "request_queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "cu_comms.h"
#include "general_config.h"
#include "wi-fi/mqtt_api.h"

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
    uint32_t lsu_time_slot = lsu->getTimeSlotInPeriod();
    uint32_t lsu_id_to_send = request->from_id; // old ID of the sender, loses meaning after sync

    int64_t time_since_boot_ms = esp_timer_get_time() / 1000;
    uint32_t now_ms = time_since_boot_ms % TIME_PERIOD_MS;

    LSU_config_package_t config_package(
      lsu_id,
      TIME_PERIOD_MS,
      now_ms,
      lsu_time_slot
    );
    CU_sendConfigPackage(&config_package, lsu_id_to_send);
    
    // Publish device linking notification to MQTT
    std::string topic = "livestock/" + std::to_string(lsu_id);
    std::string payload = "Device linked with ID: " + std::to_string(lsu_id) + 
                          ", Time slot: " + std::to_string(lsu_time_slot) + 
                          ", Period: " + std::to_string(TIME_PERIOD_MS) + "ms";
    mqtt_api_publish(topic.c_str(), payload.c_str());
    
    ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "Published device link notification to MQTT topic: %s", topic.c_str());
    
    // Save to NVS after creating LSU
    lsu_nvs_save(manager);
  } else {
    ESP_LOGW(PROCESS_REQUEST_TASK_TAG, "Failed to create LSU");
  }
}

void process_data_request(Request* request, LSUManager& manager) {
  uint32_t lsu_id = request->from_id;

  CU_sendDataAck(lsu_id);
  manager.keepaliveLSU(lsu_id);
  ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "Received data from LSU %lu: %s", lsu_id, request->data.c_str());

  std::string topic = "livestock/" + std::to_string(lsu_id);
  mqtt_api_publish(topic.c_str(), request->data.c_str());
  
  // Save to NVS after updating LSU connection time
  lsu_nvs_save(manager);
}

/* Functions ------------------------------------------------------------ */
void process_requests_task(void *arg) {
  ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "Request processing task started");

  LSUManager manager;
  
  // Small delay to ensure display is ready
  vTaskDelay(pdMS_TO_TICKS(1000));
  
  // Load LSU data from NVS on boot
  if (lsu_nvs_load(manager)) {
    ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "Successfully restored LSU data from NVS");
  } else {
    ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "No LSU data found in NVS or failed to load");
  }
  
  uint32_t last_timeout_check_ms = 0;
  const uint32_t TIMEOUT_CHECK_INTERVAL_MS = 10000; // Check every 10 seconds

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
          ESP_LOGI(PROCESS_REQUEST_TASK_TAG, "Data request received");
          process_data_request(request, manager);
          break;
        }

        default:
          [[deprecated]];
          ESP_LOGW(PROCESS_REQUEST_TASK_TAG, "Unknown request type: %d", request->type);
          break;
      }
    }

    // Check for LSU timeouts every 10 seconds
    uint32_t current_time_ms = esp_timer_get_time() / 1000; // Convert to milliseconds
    if (current_time_ms - last_timeout_check_ms >= TIMEOUT_CHECK_INTERVAL_MS) {
      manager.processTimeouts();
      // Save to NVS after processing timeouts (in case LSUs were removed)
      lsu_nvs_save(manager);
      last_timeout_check_ms = current_time_ms;
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
