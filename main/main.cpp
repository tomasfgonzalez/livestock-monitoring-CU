#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/mpu_wrappers.h>
#include <freertos/projdefs.h>

extern "C" void app_main(void) {
  while (true) {
    printf("Hello, World!\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}