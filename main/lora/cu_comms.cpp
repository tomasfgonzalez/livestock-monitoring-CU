/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : cu_comms.c
  * @brief          : Interface for Central Unit to send messages to LSU
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

/* Includes ------------------------------------------------------------ */
#include "cu_comms.h"

#include <string>
#include <string.h>
#include "rylr998.h"
#include "esp_log.h"

#define TX_BUFF_SIZE 128

/* Private variables ----------------------------------------------------- */
static const char *CU_COMMS_TAG = "CU_Communications"; 

static char tx_buff[TX_BUFF_SIZE];
std::string config_payload;

/* Private functions ----------------------------------------------------- */
void create_config_payload(LSU_config_package_t *config_package) {
  config_payload = "CONFIG";
  config_payload += "-" + std::to_string(config_package->lsu_id);
  config_payload += "-" + std::to_string(config_package->period_ms);
  config_payload += "-" + std::to_string(config_package->time_slot_ms);
}

/* Public functions ----------------------------------------------------- */
void CU_sendTest() {
  memset(tx_buff, 0, TX_BUFF_SIZE);
  snprintf(tx_buff, TX_BUFF_SIZE, AT "SEND=1,4,TEST" END);

  rylr998_sendCommand(tx_buff);
  vTaskDelay(pdMS_TO_TICKS(500));
  rylr998_getCommand(RYLR_OK);
}

void CU_sendConfigPackage(LSU_config_package_t *config_package, uint32_t destination) {
  create_config_payload(config_package);
  if (config_payload.length() >= TX_BUFF_SIZE) {
    ESP_LOGE(CU_COMMS_TAG, "Config message too long");
    return;
  }

  memset(tx_buff, 0, TX_BUFF_SIZE);
  snprintf(tx_buff, TX_BUFF_SIZE, AT"SEND=%lu,%u,%s"END, destination, config_payload.length(), config_payload.c_str());
  
  ESP_LOGI(CU_COMMS_TAG, "Sending config package: %s", tx_buff);
  rylr998_sendCommand(tx_buff);
  vTaskDelay(pdMS_TO_TICKS(500));
  rylr998_getCommand(RYLR_OK);
}
