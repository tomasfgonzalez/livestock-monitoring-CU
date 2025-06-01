/**
 ********************************************************************************
 * @authors        : Tomas Gonzalez & Brian Morris
 * @file           : cu_comms.h
 * @brief          : Interface for Central Unit to send messages to LSU
 * 
 * This file contains the declarations for the functions used to send messages to LSU
 * 
 * @attention
 *
 * Copyright (c) 2025 Tomas Gonzalez & Brian Morris
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 */

/* Header guard ---------------------------------------------------------- */
#ifndef CU_COMMS_H
#define CU_COMMS_H

/* Includes ------------------------------------------------------------ */
#include <stdint.h>

/* Structs ------------------------------------------------------------ */

/**
 * @brief Struct for the config package
 */
typedef struct {
  uint32_t lsu_id;        /**< The ID assigned to the LSU */
  uint32_t period_ms;     /**< The period of time (in ms) in which the LSU will send data */
  uint32_t now_ms;        /**< The current time (in ms) inside the period */
  uint32_t time_slot_ms;  /**< The time slot (in ms) assigned to the LSU for transmission within the period */
} LSU_config_package_t;


/* Public functions ----------------------------------------------------- */

/**
 * @brief Test sending a message to check the module is working
 */
void CU_sendTest();

/**
 * @brief Send a config package to the LSU
 * @param destination: The destination address of the LSU
 * @param lsu_id: The ID of the LSU
 */
void CU_sendConfigPackage(LSU_config_package_t *config_package, uint32_t destination);

/**
 * @brief Send a data acknowledgement to the LSU
 * @param destination: The destination address of the LSU
 */
void CU_sendDataAck(uint32_t destination);

#endif /* CU_COMMS_H */
