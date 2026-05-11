#ifndef I2C_H_
#define I2C_H_

#include "stdint.h"

void init_I2C();

void i2c_write(uint8_t *data, uint8_t len);
uint8_t i2c_write_read(uint8_t data);
uint8_t* i2c_trancive_burst(uint8_t reg_start, uint8_t length);

#endif
