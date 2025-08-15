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

/* Private variables ----------------------------------------------------- */
char wifi_status[32] = "Wi-Fi: --";
char mqtt_status[32] = "MQTT: --";
char lsu_count[32] = "LSUs: 0";
bool heartbeat_active = false;

/* Public functions ----------------------------------------------------- */
void push_status_to_oled() {
  oled_status(wifi_status, mqtt_status, lsu_count, heartbeat_active);
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
