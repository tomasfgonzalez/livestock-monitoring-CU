/**
  ******************************************************************************
  * @file           : rx_buff.h
  * @brief          : Header for rx buffer management
  ******************************************************************************
  */

#ifndef __RX_BUFF_H
#define __RX_BUFF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>

/* Defines -------------------------------------------------------------------*/
#define RX_BUFF_SIZE 128

/* Function prototypes -------------------------------------------------------*/
/**
 * @brief  Initialize the RX buffer
 * @retval None
 */
void rx_buff_init(void);

/**
 * @brief  Get the RX buffer
 * @retval Pointer to the RX buffer
 */
uint8_t* rx_buff_get(void);

/**
 * @brief  Write data to the RX buffer
 * @param  data: Pointer to data to be written
 * @param  length: Length of data to be written
 * @retval Number of bytes written
 */
uint16_t rx_buff_write(const uint8_t *data, uint16_t length);

/**
 * @brief  Read data from the RX buffer
 * @param  data: Pointer to store read data
 * @param  length: Maximum length of data to read
 * @retval Number of bytes read
 */
uint16_t rx_buff_read(uint8_t *data, uint16_t length);

/**
 * @brief  Clear the RX buffer
 * @retval None
 */
void rx_buff_clear(void);

/**
 * @brief  Get the number of bytes available in the buffer
 * @retval Number of bytes available
 */
uint16_t rx_buff_available(void);

#ifdef __cplusplus
}
#endif

#endif /* __RX_BUFF_H */
