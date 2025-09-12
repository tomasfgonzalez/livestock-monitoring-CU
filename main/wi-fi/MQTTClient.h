/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : MQTTClient.h
  * @brief          : Header file for the MQTT client
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

/* Define to prevent recursive inclusion --------------------------------- */
#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

/* Includes -------------------------------------------------------------- */
#include "esp_log.h"
#include <esp_mqtt.hpp>


/* MQTT Client Class ----------------------------------------------------- */
namespace piral {

class MQTTClient : public idf::mqtt::Client {
  private:
    static esp_mqtt_client_config_t make_cfg(const char *uri, const char *id) {
      esp_mqtt_client_config_t c{};
      c.broker.address.uri    = uri;
      c.credentials.client_id = id;
      return c;
    }

  protected:
    /**
     * @brief Called when MQTT client connects to broker
     * @param evt MQTT event handle (unused)
     * @details Publishes online status message when connection established
     */
    void on_connected(const esp_mqtt_event_handle_t) override;

    /**
     * @brief Called when MQTT message is received
     * @param evt MQTT event handle containing topic and payload data
     * @details Logs received message topic and payload
     */
    void on_data(const esp_mqtt_event_handle_t evt) override;

    /**
     * @brief Called when MQTT client disconnects from broker
     * @param evt MQTT event handle (unused)
     * @details Updates connection status when disconnection occurs
     */
    void on_disconnected(const esp_mqtt_event_handle_t) override;

  public:
    MQTTClient(const char *broker_uri, const char *client_id = "cu-01");

    inline void begin() {
      if (handler) {
        esp_mqtt_client_start(handler.get());
      }
    }

    inline void end() {
      if (handler) {
        esp_mqtt_client_stop(handler.get());
      }
    }

    /* helper for the rest of the firmware */
    inline void publish_data(const std::string &topic, const std::string &data);
};

} /* namespace piral */

#endif /* MQTT_CLIENT_H */
