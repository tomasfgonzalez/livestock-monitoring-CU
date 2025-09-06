/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : lsu_nvs_persistence.h
  * @brief          : NVS persistence module for LSUManager data
  ******************************************************************************
  */

#ifndef LSU_NVS_PERSISTENCE_H
#define LSU_NVS_PERSISTENCE_H

/* Includes ------------------------------------------------------------------*/
#include <vector>
#include <cstdint>
#include "LSUManager.h"

/* Macros -------------------------------------------------------------------*/
#define NVS_NAMESPACE "lsu_manager"
#define NVS_LSU_COUNT_KEY "lsu_count"
#define NVS_LSU_DATA_KEY "lsu_data"
#define NVS_TIMESTAMP_KEY "timestamp_us"

/* Function declarations -----------------------------------------------------*/

/**
 * @brief Saves LSUManager data to NVS
 * @param manager Reference to LSUManager instance
 * @return true if save was successful, false otherwise
 */
bool lsu_nvs_save(LSUManager& manager);

/**
 * @brief Loads LSUManager data from NVS
 * @param manager Reference to LSUManager instance
 * @return true if load was successful, false otherwise
 */
bool lsu_nvs_load(LSUManager& manager);

/**
 * @brief Clears all LSU data from NVS
 * @return true if clear was successful, false otherwise
 */
bool lsu_nvs_clear();

/**
 * @brief Tests NVS functionality by saving and loading data
 * @param manager Reference to LSUManager instance
 * @return true if test passed, false otherwise
 */
bool lsu_nvs_test(LSUManager& manager);

#endif /* LSU_NVS_PERSISTENCE_H */
