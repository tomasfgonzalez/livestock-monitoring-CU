/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : wifi.h
  * @brief          : Header file for the wifi event handler
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
#ifndef WIFI_EVENT_HANDLER_H
#define WIFI_EVENT_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes -------------------------------------------------------------- */
#include "esp_event.h"

/* Defines --------------------------------------------------------------- */
#define WIFI_CONNECTED_BIT BIT0

/* Functions ------------------------------------------------------------- */

/**
 * @brief Wifi init
 */
void wifi_init_sta(void);

/**
 * @brief Wifi start
 */
void wifi_start(void);

#ifdef __cplusplus
}
#endif

#endif /* WIFI_EVENT_HANDLER_H */
