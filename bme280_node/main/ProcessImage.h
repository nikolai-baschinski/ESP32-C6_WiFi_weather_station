// The process image contains data of the process
// There can't exist loose data- Each data belongs logically to a module
#ifndef PROCESSIMAGE_H_
#define PROCESSIMAGE_H_

#include "bme/BME.h"

struct ProcessImage {
  struct BME280_for_LCD bme280;
  struct BME280_for_LCD bme280_memory;
};

#endif /* PROCESSIMAGE_H_ */
