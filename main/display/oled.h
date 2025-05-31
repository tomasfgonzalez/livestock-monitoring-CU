/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : oled.h
  * @brief          : Interface for OLED display
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
#ifndef __OLED_H__
#define __OLED_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Defines ------------------------------------------------------------------- */
#define I2C_BUS_PORT  0

#define LCD_PIXEL_CLOCK_HZ    (50 * 1000)
#define LCD_PIN_NUM_SDA       21
#define LCD_PIN_NUM_SCL       22
#define LCD_PIN_NUM_RST       -1
#define LCD_I2C_HW_ADDR       0x3C

#define LCD_H_RES              128
#define LCD_V_RES              64
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

/* Public functions ---------------------------------------------------------- */

/**
 * @brief Initialize the OLED display
 * @param None
 * @return None
 */
void oled_init();

/**
 * @brief Display the welcome message
 * @param None
 * @return None
 */
void oled_welcome();

/**
 * @brief Display the status of the system
 * @param wifi_status: The status of the WiFi connection
 * @param mqtt_status: The status of the MQTT connection
 * @param lsu_status: The status of the LSU connection
 * @return None
 */
void oled_status(char *wifi_status, char *mqtt_status, char *lsu_status);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H__ */
