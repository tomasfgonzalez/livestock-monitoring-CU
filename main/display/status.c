/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : status.c
  * @brief          : Handles the status shown on the OLED display
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 Tomas Gonzalez & Brian Morris
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  */

/* Includes ------------------------------------------------------------ */
#include "status.h"

#include <stdio.h>
#include "oled.h"
#include "esp_log.h"
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/* Private variables ----------------------------------------------------- */
char wifi_status[32] = "Wi-Fi: --";
char mqtt_status[32] = "MQTT: --";
char lsu_count[32] = "LSUs: 0";
bool heartbeat_active = false;
bool display_ready = false;

// Semaphore to signal display update needed
static SemaphoreHandle_t display_update_semaphore = NULL;

/* Public functions ----------------------------------------------------- */
void init_display_mutex() {
  // Create semaphore for display updates
  if (display_update_semaphore == NULL) {
    display_update_semaphore = xSemaphoreCreateBinary();
    if (display_update_semaphore == NULL) {
      ESP_LOGE("STATUS", "Failed to create display update semaphore");
    }
  }
}

// LVGL timer callback for safe display updates
static void display_update_timer_cb(lv_timer_t * timer) {
  if (display_ready) {
    oled_status(wifi_status, mqtt_status, lsu_count, heartbeat_active);
  }
  // Delete the timer after one execution
  lv_timer_del(timer);
}

void push_status_to_oled() {
  if (display_ready) {
    // Create a one-shot timer to update display from LVGL task context
    lv_timer_t * timer = lv_timer_create(display_update_timer_cb, 1, NULL);
    if (timer != NULL) {
      lv_timer_set_repeat_count(timer, 1); // Execute only once
    }
  }
}

void update_wifi_status(char *status) {
  sprintf(wifi_status, "Wi-Fi: %s", status);
  push_status_to_oled();
}

void update_mqtt_status(char *status) {
  sprintf(mqtt_status, "MQTT: %s", status);
  push_status_to_oled();
}

void update_lsu_count(int count) {
  sprintf(lsu_count, "LSUs: %d", count);
  push_status_to_oled();
}

void update_heartbeat_status(bool is_active) {
  heartbeat_active = is_active;
  push_status_to_oled();
}

void set_display_ready(bool ready) {
  display_ready = ready;
  if (ready) {
    push_status_to_oled(); // Update display when it becomes ready
  }
}
