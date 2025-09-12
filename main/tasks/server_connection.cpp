#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wi-fi/wifi.h"
#include "wi-fi/mqtt_api.h"
#include "display/status.h"

static const char *CONNECTIVITY_TASK_TAG = "Connectivity";

void server_connection_task(void *arg) {
  ESP_LOGI(CONNECTIVITY_TASK_TAG, "Connectivity task started");

  uint32_t total_timeout_ms = 30000;  // 30 seconds total timeout
  uint32_t start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

  // Initialize WiFi with reasonable timeout (30 seconds)
  wifi_init_sta();
  wifi_start(30000);  // 30 seconds for WiFi connection

  // Check if we still have time for MQTT
  uint32_t elapsed_time = (xTaskGetTickCount() * portTICK_PERIOD_MS) - start_time;
  if (elapsed_time >= total_timeout_ms) {
    ESP_LOGW(CONNECTIVITY_TASK_TAG, "WiFi connection took too long - proceeding without MQTT");
    update_mqtt_status("Offline");
    ESP_LOGI(CONNECTIVITY_TASK_TAG, "Connectivity task completed");
    vTaskDelete(NULL);
    return;
  }

  // Initialize MQTT
  mqtt_api_init();

  // Wait for MQTT connection with remaining time
  while (!mqtt_api_is_connected()) {
    vTaskDelay(pdMS_TO_TICKS(5000));  // Check every 5 seconds
    
    elapsed_time = (xTaskGetTickCount() * portTICK_PERIOD_MS) - start_time;
    if (elapsed_time >= total_timeout_ms) {
      ESP_LOGW(CONNECTIVITY_TASK_TAG, "Connection timeout - stopping MQTT and proceeding without it");
      mqtt_api_deinit();
      break;
    }
  }

  if (mqtt_api_is_connected()) {
    ESP_LOGI(CONNECTIVITY_TASK_TAG, "Both WiFi and MQTT connected successfully");
  } else {
    ESP_LOGW(CONNECTIVITY_TASK_TAG, "Proceeding without server connectivity");
  }

  ESP_LOGI(CONNECTIVITY_TASK_TAG, "Connectivity task completed");
  vTaskDelete(NULL);  // Delete the current task
}
