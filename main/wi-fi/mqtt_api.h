/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : mqtt-api.h
  * @brief          : MQTT API
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

/* Prevent recursive inclusion ----------------------------------------------*/
#ifndef MQTT_API_H
#define MQTT_API_H

/* Defines --------------------------------------------------------------- */
#define MQTT_BROKER_IP "172.24.255.70"
#define MQTT_BROKER_PORT "1883"

/* Function declarations ----------------------------------------------------*/
void mqtt_api_init();

void mqtt_api_deinit();

void mqtt_api_publish(const char *topic, const char *payload);

bool mqtt_api_is_connected();

void mqtt_api_set_connected(bool connected);

#endif // MQTT_API_H
