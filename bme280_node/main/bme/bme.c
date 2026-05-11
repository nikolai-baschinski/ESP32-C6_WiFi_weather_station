#include <string.h>
#include "BME.h"
#include "I2C.h"

const float MAX_TEMPERATURE = 85.0f;
const float MIN_TEMPERATURE = -40.0f;
const uint16_t MAX_PRESSURE = 1100;
const uint16_t MIN_PRESSURE = 300;
const uint8_t MAX_HUMIDITY = 100;
const uint8_t MIN_HUMIDITY = 0;

struct Calibration_T {
  // The calibration values are individual for each sensor and have to be read
  // The data types are from the table 16 in BME280 data sheet
  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;
};

struct Calibration_P {
  uint16_t dig_P1;
  int16_t dig_P2;
  int16_t dig_P3;
  int16_t dig_P4;
  int16_t dig_P5;
  int16_t dig_P6;
  int16_t dig_P7;
  int16_t dig_P8;
  int16_t dig_P9;
};

struct Calibration_H {
  uint8_t dig_H1;
  int16_t dig_H2;
  uint8_t dig_H3;
  int16_t dig_H4;
  int16_t dig_H5;
  int8_t dig_H6;
};

struct ADC_T {
  uint8_t T_msb;
  uint8_t T_lsb;
  uint8_t T_xlsb;
  int32_t adc_T;
};

struct ADC_H {
  uint8_t H_msb;
  uint8_t H_lsb;
  int32_t adc_H;
};

struct ADC_P {
  uint8_t P_msb;
  uint8_t P_lsb;
  uint8_t P_xlsb;
  int32_t adc_P;
};

struct BME {
  struct Calibration_T cal_T;
  struct Calibration_P cal_P;
  struct Calibration_H cal_H;
  struct ADC_T Adc_T;
  struct ADC_P Adc_P;
  struct ADC_H Adc_H;
  int32_t t_fine;
  int32_t T;
  uint32_t H;
  uint32_t P;
};

struct BME bme;

void bme_get_sensor_data(struct BME280_for_LCD* p_bme_data)
{
  int32_t t = bme.T;
  int32_t rounded = (t >= 0) ? (t + 5) / 10 : (t - 5) / 10;
  p_bme_data->temperature = rounded / 10.0f;
  if(p_bme_data->temperature < MIN_TEMPERATURE) {
    p_bme_data->temperature = MIN_TEMPERATURE;
  }

  if(p_bme_data->temperature > MAX_TEMPERATURE) {
    p_bme_data->temperature = MAX_TEMPERATURE;
  }

  float tmp_P = bme.P / 25600.0f;
  uint16_t tmp_P_uint16 = tmp_P + 0.5f;
  p_bme_data->pressure = tmp_P_uint16;
  if(p_bme_data->pressure < MIN_PRESSURE) {
    p_bme_data->pressure = MIN_PRESSURE;
  }
  if(p_bme_data->pressure > MAX_PRESSURE) {
    p_bme_data->pressure = MAX_PRESSURE;
  }

  float tmp_H = bme.H / 1024.0f;
  uint8_t tmp_H_uint8 = tmp_H + 0.5f;
  p_bme_data->humidity = tmp_H_uint8;
  if(p_bme_data->humidity < MIN_HUMIDITY) {
    p_bme_data->humidity = MIN_HUMIDITY;
  }
  if(p_bme_data->humidity > MAX_HUMIDITY) {
    p_bme_data->humidity = MAX_HUMIDITY;
  }
}

uint8_t bme_read(uint8_t data)
{
  uint8_t rv = 0;
  rv = i2c_write_read(data);
  return rv;
}

void bme_fetch_compensation_data(struct BME* bme)
{
  uint8_t dig_T1_LSB = bme_read(0x88); // address from BOSCH data sheet
  uint8_t dig_T1_MSB = bme_read(0x89);
  bme->cal_T.dig_T1 = dig_T1_LSB | (dig_T1_MSB << 8);

  uint8_t dig_T2_LSB = bme_read(0x8A);
  uint8_t dig_T2_MSB = bme_read(0x8B);
  bme->cal_T.dig_T2 = dig_T2_LSB | (dig_T2_MSB << 8);

  uint8_t dig_T3_LSB = bme_read(0x8C);
  uint8_t dig_T3_MSB = bme_read(0x8D);
  bme->cal_T.dig_T3 = dig_T3_LSB | (dig_T3_MSB << 8);

  uint8_t dig_P1_LSB = bme_read(0x8E);
  uint8_t dig_P1_MSB = bme_read(0x8F);
  bme->cal_P.dig_P1 = dig_P1_LSB | (dig_P1_MSB << 8);

  uint8_t dig_P2_LSB = bme_read(0x90);
  uint8_t dig_P2_MSB = bme_read(0x91);
  bme->cal_P.dig_P2 = dig_P2_LSB | (dig_P2_MSB << 8);

  uint8_t dig_P3_LSB = bme_read(0x92);
  uint8_t dig_P3_MSB = bme_read(0x93);
  bme->cal_P.dig_P3 = dig_P3_LSB | (dig_P3_MSB << 8);

  uint8_t dig_P4_LSB = bme_read(0x94);
  uint8_t dig_P4_MSB = bme_read(0x95);
  bme->cal_P.dig_P4 = dig_P4_LSB | (dig_P4_MSB << 8);

  uint8_t dig_P5_LSB = bme_read(0x96);
  uint8_t dig_P5_MSB = bme_read(0x97);
  bme->cal_P.dig_P5 = dig_P5_LSB | (dig_P5_MSB << 8);

  uint8_t dig_P6_LSB = bme_read(0x98);
  uint8_t dig_P6_MSB = bme_read(0x99);
  bme->cal_P.dig_P6 = dig_P6_LSB | (dig_P6_MSB << 8);

  uint8_t dig_P7_LSB = bme_read(0x9A);
  uint8_t dig_P7_MSB = bme_read(0x9B);
  bme->cal_P.dig_P7 = dig_P7_LSB | (dig_P7_MSB << 8);

  uint8_t dig_P8_LSB = bme_read(0x9C);
  uint8_t dig_P8_MSB = bme_read(0x9D);
  bme->cal_P.dig_P8 = dig_P8_LSB | (dig_P8_MSB << 8);

  uint8_t dig_P9_LSB = bme_read(0x9E);
  uint8_t dig_P9_MSB = bme_read(0x9F);
  bme->cal_P.dig_P9 = dig_P9_LSB | (dig_P9_MSB << 8);

  uint8_t dig_H1 = bme_read(0xA1);
  bme->cal_H.dig_H1 = dig_H1;

  uint8_t dig_H2_LSB = bme_read(0xE1);
  uint8_t dig_H2_MSB = bme_read(0xE2);
  bme->cal_H.dig_H2 = dig_H2_LSB | (dig_H2_MSB << 8);

  uint8_t dig_H3 = bme_read(0xE3);
  bme->cal_H.dig_H3 = dig_H3;

  uint8_t e4 = bme_read(0xE4);
  uint8_t e5 = bme_read(0xE5);
  uint8_t e6 = bme_read(0xE6);

  bme->cal_H.dig_H4 = (int16_t)((e4 << 4) | (e5 & 0x0F));
  bme->cal_H.dig_H5 = (int16_t)((e6 << 4) | (e5 >> 4));
  if (bme->cal_H.dig_H4 & 0x800) bme->cal_H.dig_H4 |= 0xF000;
  if (bme->cal_H.dig_H5 & 0x800) bme->cal_H.dig_H5 |= 0xF000;

  uint8_t dig_H6 = bme_read(0xE7);
  bme->cal_H.dig_H6 = (int8_t)dig_H6;
}

void compensate_P(struct BME* p_bme)
{
  int64_t var1, var2, p;

  p_bme->Adc_P.adc_P = ((int32_t)p_bme->Adc_P.P_msb << 12) | ((int32_t)p_bme->Adc_P.P_lsb << 4) | ((int32_t)p_bme->Adc_P.P_xlsb >> 4);

  var1 = ((int64_t)p_bme->t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)p_bme->cal_P.dig_P6;
  var2 = var2 + ((var1 * (int64_t)p_bme->cal_P.dig_P5) << 17);
  var2 = var2 + (((int64_t)p_bme->cal_P.dig_P4) << 35);

  var1 = ((var1 * var1 * (int64_t)p_bme->cal_P.dig_P3) >> 8) + ((var1 * (int64_t)p_bme->cal_P.dig_P2) << 12);

  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)p_bme->cal_P.dig_P1) >> 33;

  if (var1 == 0) {
    p_bme->P = 0;
    return;
  }

  p = 1048576 - p_bme->Adc_P.adc_P;
  p = (((p << 31) - var2) * 3125) / var1;

  var1 = (((int64_t)p_bme->cal_P.dig_P9) * (p >> 13) * (p >> 13)) >> 25;

  var2 = (((int64_t)p_bme->cal_P.dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)p_bme->cal_P.dig_P7) << 4);

  p_bme->P = (uint32_t)p;
}

void compensate_H(struct BME* p_bme)
{
  int32_t v_x1;

  p_bme->Adc_H.adc_H = ((int32_t)p_bme->Adc_H.H_msb << 8) | (int32_t)p_bme->Adc_H.H_lsb;

  v_x1 = p_bme->t_fine - 76800;

  v_x1 = (((((p_bme->Adc_H.adc_H << 14) -
      (((int32_t)p_bme->cal_H.dig_H4) << 20) -
      (((int32_t)p_bme->cal_H.dig_H5) * v_x1)) +
      16384) >> 15) *
      (((((((v_x1 * (int32_t)p_bme->cal_H.dig_H6) >> 10) *
      (((v_x1 * (int32_t)p_bme->cal_H.dig_H3) >> 11) +
      32768)) >> 10) +
      2097152) *
      (int32_t)p_bme->cal_H.dig_H2 + 8192) >> 14));

  v_x1 = v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * (int32_t)p_bme->cal_H.dig_H1) >> 4);

  if (v_x1 < 0) v_x1 = 0;
  if (v_x1 > 419430400) v_x1 = 419430400;

  p_bme->H = (uint32_t)(v_x1 >> 12);
}

void compensate_T(struct BME* p_bme)
{
  p_bme->Adc_T.adc_T = ((int32_t)p_bme->Adc_T.T_msb << 12) | ((int32_t)p_bme->Adc_T.T_lsb << 4) | ((int32_t)p_bme->Adc_T.T_xlsb >> 4);

  int32_t var1, var2;
  var1 = ((((p_bme->Adc_T.adc_T >> 3) - ((int32_t)p_bme->cal_T.dig_T1 << 1))) * ((int32_t)p_bme->cal_T.dig_T2)) >> 11;

  var2 = (((((p_bme->Adc_T.adc_T >> 4) - ((int32_t)p_bme->cal_T.dig_T1)) * ((p_bme->Adc_T.adc_T >> 4) - ((int32_t)p_bme->cal_T.dig_T1))) >> 12) * ((int32_t)p_bme->cal_T.dig_T3)) >> 14;

  p_bme->t_fine = var1 + var2;

  p_bme->T = (p_bme->t_fine * 5 + 128) >> 8;
}

void bme_compensate()
{
  compensate_T(&bme);
  compensate_H(&bme);
  compensate_P(&bme);
}

void bme_get_raw_sensor_data()
{
  uint8_t* p_burst_rcv_buffer = i2c_trancive_burst(0xF7, 8);

  bme.Adc_P.P_msb  = p_burst_rcv_buffer[0];
  bme.Adc_P.P_lsb  = p_burst_rcv_buffer[1];
  bme.Adc_P.P_xlsb = p_burst_rcv_buffer[2];
  bme.Adc_T.T_msb  = p_burst_rcv_buffer[3];
  bme.Adc_T.T_lsb  = p_burst_rcv_buffer[4];
  bme.Adc_T.T_xlsb = p_burst_rcv_buffer[5];
  bme.Adc_H.H_msb  = p_burst_rcv_buffer[6];
  bme.Adc_H.H_lsb  = p_burst_rcv_buffer[7];
}

void cyclic_BME(struct BME280_for_LCD* p_bme_data)
{
  bme_get_raw_sensor_data();
  bme_compensate();
  bme_get_sensor_data(p_bme_data);
}

void init_BME()
{
  memset(&bme, 0, sizeof(bme));
  bme_fetch_compensation_data(&bme);

  #define LENGTH_BME_CONF 2
  uint8_t control_bytes_F2[LENGTH_BME_CONF] = {0xF2, 0x01};
  uint8_t control_bytes_F4[LENGTH_BME_CONF] = {0xF4, 0x27};
  
  i2c_write(control_bytes_F2, LENGTH_BME_CONF);
  i2c_write(control_bytes_F4, LENGTH_BME_CONF);
}
