#include <stdbool.h>
#include <unistd.h>
#include "esp_log.h"
#include "lcd.h"
#include "wifi.h"
#include "ProcessImage.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct ProcessImage pi={0};

void app_main(void)
{
  init_LCD();
  init_wifi(&pi);

  while(1) {
    cyclic_LCD(&pi);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}