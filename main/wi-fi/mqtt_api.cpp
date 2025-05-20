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
static const char *BROKER_URI = "mqtt://" BROKER_IP ":" BROKER_PORT;

/* Variables ------------------------------------------------------------- */
static piral::MQTTClient* mqtt = nullptr;

/* Function implementations -------------------------------------------------*/
void mqtt_api_init() {
  mqtt = new piral::MQTTClient(BROKER_URI);
  mqtt->begin();
}

void mqtt_api_publish(const char *topic, const char *payload) {
  ESP_LOGI(MQTT_API_TAG, "Publishing message to topic: %s", topic);
  mqtt->publish_data(topic, payload);
}
