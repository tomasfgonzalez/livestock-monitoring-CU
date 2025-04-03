#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/mpu_wrappers.h>
#include <freertos/projdefs.h>
#include "uart0.h"

#define BUF_SIZE 256

extern "C" void app_main(void) {
  uart0_init(BUF_SIZE);

  uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
  while (true) {
    int len = uart0_receive(data, BUF_SIZE);

    // Write data back to the UART
    uart0_send(data, len);
    if (len) {
      data[len] = '\0';
      // printf("LAPUTAMADRE! str: %s", (char *) data);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}