#ifndef BME_H_
#define BME_H_

#include "stdint.h"

struct BME280_for_LCD {
  float temperature;
  uint16_t pressure;
  uint8_t humidity;
};

void init_BME();
void cyclic_BME(struct BME280_for_LCD* p_bme_data);

#endif /* BME_H_ */
