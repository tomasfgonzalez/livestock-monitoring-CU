/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : request_queue.cpp
  * @brief          : Single interface to manage requests, can be accessed and
  *                   modified by different tasks
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
#include "request_queue.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

/* Private variables --------------------------------------------------------- */
static const char *REQUEST_QUEUE_TAG = "RQ_QUEUE";
std::queue<Request*> RequestQueue;

/* Functions ------------------------------------------------------------ */
void post_request(std::string data, uint16_t from_id) {
  // Validate the format of the data

  // TODO: Add error handling try/catch

  Request* request = new Request({
    from_id,
    (data.starts_with("SYNC")) ? REQUEST_TYPE_SYNC : REQUEST_TYPE_DATA,
    data,
    xTaskGetTickCount()
  });
  RequestQueue.push(request);
}

Request* get_request() {
  if (RequestQueue.empty()) {
    return NULL;
  }
  Request* request = RequestQueue.front();
  RequestQueue.pop();
  return request;
}
