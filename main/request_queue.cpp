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

/* Private variables --------------------------------------------------------- */
static const char *REQUEST_QUEUE_TAG = "RQ_QUEUE";
std::queue<Request*> RequestQueue;

/* Functions ------------------------------------------------------------ */
void post_request(std::string data) {
  // Validate the format of the data
  size_t first_comma = data.find(',');
  if (first_comma == std::string::npos) {
    ESP_LOGE(REQUEST_QUEUE_TAG, "Invalid format, no first comma");
    return;
  }
  size_t second_comma = data.find(',', first_comma + 1);
  if (second_comma == std::string::npos) {
    ESP_LOGE(REQUEST_QUEUE_TAG, "Invalid format, no second comma");
    return;
  }

  // TODO: Add error handling try/catch

  uint32_t from_id = std::stoi(data.substr(0, first_comma));
  std::string payload = data.substr(second_comma + 1);

  Request* request = new Request({
    from_id,
    (payload.starts_with("SYNC")) ? REQUEST_TYPE_SYNC : REQUEST_TYPE_DATA,
    payload
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
