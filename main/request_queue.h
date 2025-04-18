/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : request_queue.h
  * @brief          : Header for single interface to manage requests, can be
  *                   accessed and modified by different tasks
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
#include <queue>
#include <string>

/* Structs ------------------------------------------------------------- */
enum RequestType {
  REQUEST_TYPE_SYNC,
  REQUEST_TYPE_DATA,
};

struct Request {
  uint32_t from_id;
  RequestType type;
  std::string data;
};

/* Public API ---------------------------------------------------------- */
/**
 * @brief Post a request to the queue
 * 
 * @param data 
 */
void post_request(std::string data);

/**
 * @brief Get a request from the queue
 * 
 * @return Request* 
 */ 
Request* get_request();
