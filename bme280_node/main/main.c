#include <stdbool.h>
#include <unistd.h>
#include "esp_log.h"
#include "bme.h"
#include "I2C.h"
#include "wifi.h"
#include "ProcessImage.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct ProcessImage pi={0};

void app_main(void)
{
  init_I2C();
  init_BME();
  init_wifi(&pi);

  while(1) {
    cyclic_BME(&pi.bme280);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}