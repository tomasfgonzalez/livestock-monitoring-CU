/**
  ******************************************************************************
  * @file           : rx_buff.c
  * @brief          : Source for rx buffer management
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rx_buff.h"
#include <string.h>

/* Private variables ---------------------------------------------------------*/
static uint8_t rx_buff[RX_BUFF_SIZE + 1];
static uint16_t rx_head = 0;
static uint16_t rx_tail = 0;
static uint16_t rx_count = 0;

/* Function implementations --------------------------------------------------*/
void rx_buff_init(void) {
    rx_head = 0;
    rx_tail = 0;
    rx_count = 0;
    memset(rx_buff, 0, RX_BUFF_SIZE);
}

uint8_t* rx_buff_get(void) {
    return rx_buff;
}

uint16_t rx_buff_write(const uint8_t *data, uint16_t length) {
    uint16_t bytes_written = 0;
    
    while (bytes_written < length && rx_count < RX_BUFF_SIZE) {
        rx_buff[rx_head] = data[bytes_written];
        rx_head = (rx_head + 1) % RX_BUFF_SIZE;
        rx_count++;
        bytes_written++;
    }
    
    return bytes_written;
}

uint16_t rx_buff_read(uint8_t *data, uint16_t length) {
    uint16_t bytes_read = 0;
    
    while (bytes_read < length && rx_count > 0) {
        data[bytes_read] = rx_buff[rx_tail];
        rx_tail = (rx_tail + 1) % RX_BUFF_SIZE;
        rx_count--;
        bytes_read++;
    }
    
    return bytes_read;
}

void rx_buff_clear(void) {
    rx_head = 0;
    rx_tail = 0;
    rx_count = 0;
    memset(rx_buff, 0, RX_BUFF_SIZE);
}

uint16_t rx_buff_available(void) {
    return rx_count;
}
