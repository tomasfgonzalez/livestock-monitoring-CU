#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "display/status.h"

static const char *HEARTBEAT_TASK_TAG = "Heartbeat";

void heartbeat_task(void *arg) {
  ESP_LOGI(HEARTBEAT_TASK_TAG, "Heartbeat task started");

  while (1) {
    // Turn heartbeat ON for 1 second
    update_heartbeat_status(true);
    ESP_LOGD(HEARTBEAT_TASK_TAG, "Heartbeat");
    vTaskDelay(pdMS_TO_TICKS(1000));  // 1 second delay

    // Turn heartbeat OFF for 3 seconds
    update_heartbeat_status(false);
    vTaskDelay(pdMS_TO_TICKS(3000));  // 3 seconds delay
  }
}
