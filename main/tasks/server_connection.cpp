#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wi-fi/wifi.h"
#include "wi-fi/mqtt_api.h"

static const char *CONNECTIVITY_TASK_TAG = "Connectivity";

void server_connection_task(void *arg) {
  ESP_LOGI(CONNECTIVITY_TASK_TAG, "Connectivity task started");

  // Initialize WiFi
  wifi_init_sta();
  wifi_start();

  // Initialize MQTT
  mqtt_api_init();

  ESP_LOGI(CONNECTIVITY_TASK_TAG, "Connectivity task completed");
  vTaskDelete(NULL);  // Delete the current task
}
