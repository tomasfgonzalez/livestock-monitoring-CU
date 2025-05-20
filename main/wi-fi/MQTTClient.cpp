/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : MQTTClient.cpp
  * @brief          : MQTT client implementation
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
#include "MQTTClient.h"

/* Defines --------------------------------------------------------------- */
static const char *MQTT_TAG = "MQTT";
static  esp_mqtt_client_handle_t client = NULL;

/* Function implementations -------------------------------------------------*/
piral::MQTTClient::MQTTClient(const char *broker_uri, const char *client_id)
  : idf::mqtt::Client(make_cfg(broker_uri, client_id)) {}

void piral::MQTTClient::on_connected(const esp_mqtt_event_handle_t) {
  ESP_LOGI(MQTT_TAG, "MQTT connected ✅");
  publish_data("piral/ecu/online", {"true"});
}

void piral::MQTTClient::on_data(const esp_mqtt_event_handle_t evt) {
  ESP_LOGI(MQTT_TAG, "RX %.*s → %.*s", evt->topic_len, evt->topic, evt->data_len, evt->data);
}

inline void piral::MQTTClient::publish_data(const std::string &topic, const std::string &data) {
  publish(topic, data.begin(), data.end(),
    idf::mqtt::QoS::AtLeastOnce,
    idf::mqtt::Retain::NotRetained
  );
}
