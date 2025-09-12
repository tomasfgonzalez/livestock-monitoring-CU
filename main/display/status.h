/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : status.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STATUS_H
#define STATUS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Public functions ---------------------------------------------------------- */
/**
 * @brief Initialize the display mutex for thread-safe operations
 * @param None
 * @return None
 */
void init_display_mutex();

/**
 * @brief Push the status to the OLED display
 * @param None
 * @return None
 */
void push_status_to_oled();

/**
 * @brief Update the WiFi status
 * @param status: The status of the WiFi connection
 * @return None
 */
void update_wifi_status(char *status);

/**
 * @brief Update the MQTT status
 * @param status: The status of the MQTT connection
 * @return None
 */
void update_mqtt_status(char *status);

/**
 * @brief Update the LSU count
 * @param count: The number of LSUs
 * @return None
 */
void update_lsu_count(int count);

/**
 * @brief Update the heartbeat status
 * @param is_active: Boolean indicating if heartbeat is active
 * @return None
 */
void update_heartbeat_status(bool is_active);

/**
 * @brief Set the display ready state
 * @param ready: Boolean indicating if display is ready for updates
 * @return None
 */
void set_display_ready(bool ready);

#ifdef __cplusplus
}
#endif

#endif /* STATUS_H */
