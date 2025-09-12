/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : mqtt_api.cpp
  * @brief          : MQTT API implementation
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

/* Includes -------------------------------------------------------------- */
#include "mqtt_api.h"
#include "MQTTClient.h"
#include "esp_log.h"

/* Defines --------------------------------------------------------------- */
static const char *MQTT_API_TAG = "MQTT_API";
static const char *MQTT_BROKER_URI = "mqtt://" MQTT_BROKER_IP ":" MQTT_BROKER_PORT;

/* Variables ------------------------------------------------------------- */
static piral::MQTTClient* mqtt = nullptr;
static bool mqtt_connected = false;

/* Function implementations -------------------------------------------------*/
void mqtt_api_init() {
  mqtt = new piral::MQTTClient(MQTT_BROKER_URI);
  mqtt->begin();
}

void mqtt_api_deinit() {
  if (mqtt) {
    mqtt->end();
    delete mqtt;
    mqtt = nullptr;
  }
  mqtt_connected = false;
}

void mqtt_api_publish(const char *topic, const char *payload) {
  if (!mqtt_connected) return;
  ESP_LOGI(MQTT_API_TAG, "Publishing message to topic: %s", topic);
  mqtt->publish_data(topic, payload);
}

bool mqtt_api_is_connected() {
  return mqtt_connected;
}

void mqtt_api_set_connected(bool connected) {
  mqtt_connected = connected;
}
