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

#ifndef REQUEST_QUEUE_H
#define REQUEST_QUEUE_H

/* Includes ------------------------------------------------------------ */
#include <queue>
#include <string>

#include "uart.h"

/* Structs ------------------------------------------------------------- */
enum RequestType {
  REQUEST_TYPE_SYNC,
  REQUEST_TYPE_DATA,
};

struct Request {
  uint16_t from_id;
  RequestType type;
  std::string data;
  uint32_t timestamp;
  UartPort_t sourcePort;
};

/* Public API ---------------------------------------------------------- */
/**
 * @brief Post a request to the queue
 * 
 * @param data 
 * @param from_id
 * @param sourcePort
 */
void post_request(std::string data, uint16_t from_id, UartPort_t sourcePort);

/**
 * @brief Get a request from the queue
 * 
 * @return Request* 
 */ 
Request* get_request();

#endif /* REQUEST_QUEUE_H */
