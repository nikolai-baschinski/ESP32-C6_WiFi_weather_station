#include "I2C.h"
#include <string.h>
#include "driver/i2c_master.h"
#include "soc/gpio_num.h"
#include "esp_log.h"

#define SLAVE_ADDRESS 0x76
#define MAX_RECV_BURST 8

uint8_t burst_rcv_buffer[MAX_RECV_BURST]= {0};
i2c_master_dev_handle_t dev_handle;

void init_I2C()
{
  esp_err_t esp_rv;
  i2c_master_bus_handle_t i2c_bus;

  i2c_master_bus_config_t i2c_master_bus_config = {
    .i2c_port = I2C_NUM_0,
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .scl_io_num = GPIO_NUM_4,
    .sda_io_num = GPIO_NUM_5,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
  };

  esp_rv = i2c_new_master_bus(&i2c_master_bus_config, &i2c_bus);
  if(esp_rv == ESP_OK) {
    ESP_LOGI("", "I2C master initialized successfully");
  } else {
    ESP_LOGE("", "I2C master initialization failed with %d", esp_rv);
    while(1);
  }

  i2c_device_config_t dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = 0x76,
    .scl_speed_hz = 100000,
  };

  esp_rv = i2c_master_bus_add_device(i2c_bus, &dev_cfg, &dev_handle);
  if(esp_rv == ESP_OK) {
    ESP_LOGI("", "I2C slave initialized successfully");
  } else {
    ESP_LOGE("", "I2C slave initialization failed with %d", esp_rv);
    while(1);
  }
}

uint8_t* i2c_trancive_burst(uint8_t reg_start, uint8_t length)
{
  memset(burst_rcv_buffer, 0, MAX_RECV_BURST);
  esp_err_t err = i2c_master_transmit_receive(dev_handle, &reg_start, 1, burst_rcv_buffer, MAX_RECV_BURST, -1);
  if(err != ESP_OK) {
    ESP_LOGE("", "I2C sending failed with %d", err);
    while(1);
  }
  return burst_rcv_buffer;
}

uint8_t i2c_write_read(uint8_t data)
{
  uint8_t rv = 0;
  esp_err_t err = i2c_master_transmit_receive(dev_handle, &data, 1, &rv, 1, -1);
  if(err != ESP_OK) {
    ESP_LOGE("", "I2C sending failed with %d", err);
    while(1);
  }
  return rv;
}

void i2c_write(uint8_t *data, uint8_t len)
{
    esp_err_t err = i2c_master_transmit(dev_handle, data, len, -1);
    if(err != ESP_OK) {
      ESP_LOGE("", "I2C sending failed with %d", err);
      while(1);
    }
}
