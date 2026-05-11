#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "lcd.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define LCD_RST  GPIO_NUM_3
#define LCD_DC   GPIO_NUM_2

const uint16_t SPI_MAX_TX_BUFFER_SIZE = 4092; // https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32c6/api-reference/peripherals/spi_master.html#_CPPv4N16spi_bus_config_t15max_transfer_szE

// SPI handle global
spi_device_handle_t lcd_spi;

void spi_lcd_init(void)
{
  esp_err_t rv;

  // 1. config the bus
  spi_bus_config_t spi_bus_cfg = {
    .mosi_io_num = GPIO_NUM_7,// DIN (MOSI)
    .miso_io_num = -1,        // not used
    .sclk_io_num = GPIO_NUM_6,// CLK
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = SPI_MAX_TX_BUFFER_SIZE
  };

  rv = spi_bus_initialize(SPI2_HOST, &spi_bus_cfg, SPI_DMA_CH_AUTO);
  if (rv == ESP_OK) {
    ESP_LOGI("", "SPI bus initialization successfully");
  } else {
    ESP_LOGE("", "SPI bus initialization failed with %d", rv);
    while(1);
  }

  // 2. config the device
  spi_device_interface_config_t devcfg = {
    .clock_speed_hz = 4 * 1000 * 1000,
    .mode = 0,
    .spics_io_num = GPIO_NUM_10, // CS Pin
    .queue_size = 1,
    .flags = 0,
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0
  };

  rv = spi_bus_add_device(SPI2_HOST, &devcfg, &lcd_spi);
  if (rv == ESP_OK) {
    ESP_LOGI("", "SPI added device successfully");
  } else {
    ESP_LOGE("", "SPI adding device failed with %d", rv);
    while(1);
  }
}

void lcd_gpio_init(void)
{
  gpio_config_t io_conf = {
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = (1ULL << LCD_RST) | (1ULL << LCD_DC)
  };
  gpio_config(&io_conf);

  gpio_set_level(LCD_RST, 1);  // no reset state
  gpio_set_level(LCD_DC, 1);   // data
}

void lcd_RST_set()
{
  gpio_set_level(LCD_RST, 1);
}

void lcd_RST_reset()
{
  gpio_set_level(LCD_RST, 0);
}

void lcd_DC_set_data()
{
  gpio_set_level(LCD_DC, 1);
}

void lcd_DC_set_command()
{
  gpio_set_level(LCD_DC, 0);
}

void spi_send(uint8_t byte)
{
  spi_transaction_t t = {
    .length = 8,
    .tx_buffer = &byte,
    .rx_buffer = NULL
  };

  spi_device_transmit(lcd_spi, &t);
}

void lcd_send_c(uint8_t byte)
{
  lcd_DC_set_command();
  spi_send(byte);
}

void lcd_send_d(uint8_t byte)
{
  lcd_DC_set_data();
  spi_send(byte);
}

void lcd_set_window(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend)
{
  lcd_send_c(0x2A);
  lcd_send_d(Xstart >> 8);
  lcd_send_d(Xstart & 0xFF);
  lcd_send_d((Xend - 1) >> 8);
  lcd_send_d((Xend - 1) & 0xFF);

  lcd_send_c(0x2B);
  lcd_send_d(Ystart >> 8);
  lcd_send_d(Ystart & 0xFF);
  lcd_send_d((Yend - 1) >> 8);
  lcd_send_d((Yend - 1) & 0xFF);

  lcd_send_c(0x2C);
}

void lcd_send_d_word(uint16_t data)
{
  lcd_DC_set_data();
  spi_send((data>>8) & 0xff);
  spi_send(data);
}

void lcd_clear_display(uint16_t color)
{
  lcd_set_window(0, 0, LCD_2IN4_WIDTH, LCD_2IN4_HEIGHT);
  lcd_DC_set_data();

  uint8_t* buffer = malloc(SPI_MAX_TX_BUFFER_SIZE);

  for(int i = 0; i < SPI_MAX_TX_BUFFER_SIZE; i+=2) {
    buffer[i] = (color>>8) & 0xff;
    buffer[i+1] = color;
  }

  // For each pixel 2 bytes have to be transfered, so WIDTH * HEIGHT * 2 bytes
  // Each send action can transmit SPI_MAX_TX_BEFFER_SIZE bytes.
  size_t total_bytes = LCD_2IN4_WIDTH * LCD_2IN4_HEIGHT * 2;
  size_t sent = 0;

  while (sent < total_bytes) {
    size_t chunk = SPI_MAX_TX_BUFFER_SIZE;

    if (total_bytes - sent < chunk) {
      chunk = total_bytes - sent;
    }

    spi_transaction_t t = {
      .length = chunk * 8,
      .tx_buffer = buffer,
    };

    spi_device_transmit(lcd_spi, &t);

    sent += chunk;
  }
  free(buffer);
}

void lcd_display_image(uint8_t *image)
{
  lcd_set_window(0, 0, LCD_2IN4_WIDTH, LCD_2IN4_HEIGHT);

  lcd_DC_set_data();
  for(int i = 0; i < LCD_2IN4_WIDTH; i++){
    for(int j = 0; j < LCD_2IN4_HEIGHT; j++){
      lcd_send_d_word(*(image+i*LCD_2IN4_WIDTH+j));
    }
  }
}

void lcd_set_cursor(uint16_t X, uint16_t Y)
{
  lcd_send_c(0x2A);
  lcd_send_d(X >> 8);
  lcd_send_d(X);
  lcd_send_d(X >> 8);
  lcd_send_d(X);

  lcd_send_c(0x2B);
  lcd_send_d(Y >> 8);
  lcd_send_d(Y);
  lcd_send_d(Y >> 8);
  lcd_send_d(Y);

  lcd_send_c(0x2C);
}

void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
  lcd_set_cursor(x, y);
  lcd_send_d_word(color);
}

void lcd_clear_window(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t color)
{
  lcd_set_window(Xstart, Ystart, Xend, Yend);
  for(int i = Ystart; i <= Yend; i++) {
    for(int j = Xstart; j <= Xend; j++) {
      lcd_send_d_word(color);
    }
  }
}

void lcd_reset()
{
  lcd_RST_reset();
   vTaskDelay(pdMS_TO_TICKS(100));
  lcd_RST_set();
  vTaskDelay(pdMS_TO_TICKS(100));
}

void lcd_init()
{
  lcd_send_c(0x11); // SLEEP OUT: COMMAND
  vTaskDelay(pdMS_TO_TICKS(120));
  lcd_send_c(0xCF); // POWER CONTROL B: COMMAND
  lcd_send_d(0x00); // POWER CONTROL B: DATA
  lcd_send_d(0xC1); // POWER CONTROL B: DATA
  lcd_send_d(0x30); // POWER CONTROL B: DATA
  lcd_send_c(0xED); // POWER-ON SEQUENCE CONTROL: COMMAND
  lcd_send_d(0x64); // SOFT START CONTROL: DATA
  lcd_send_d(0x03); // POWER-ON SEQUENCE CONTROL: DATA
  lcd_send_d(0x12); // POWER-ON SEQUENCE CONTROL: DATA
  lcd_send_d(0x81); // DDVDH ENHANCE MODE: DATA
  lcd_send_c(0xE8);
  lcd_send_d(0x85);
  lcd_send_d(0x00);
  lcd_send_d(0x79);
  lcd_send_c(0xCB);
  lcd_send_d(0x39);
  lcd_send_d(0x2C);
  lcd_send_d(0x00);
  lcd_send_d(0x34);
  lcd_send_d(0x02);
  lcd_send_c(0xF7);
  lcd_send_d(0x20);
  lcd_send_c(0xEA);
  lcd_send_d(0x00);
  lcd_send_d(0x00);
  lcd_send_c(0xC0);
  lcd_send_d(0x1D);
  lcd_send_c(0xC1);
  lcd_send_d(0x12);
  lcd_send_c(0xC5);
  lcd_send_d(0x33);
  lcd_send_d(0x3F);
  lcd_send_c(0xC7);
  lcd_send_d(0x92);
  lcd_send_c(0x3A);
  lcd_send_d(0x55);
  lcd_send_c(0x36);
  lcd_send_d(0x08);
  lcd_send_c(0xB1);
  lcd_send_d(0x00);
  lcd_send_d(0x12);
  lcd_send_c(0xB6);
  lcd_send_d(0x0A);
  lcd_send_d(0xA2);
  lcd_send_c(0x44);
  lcd_send_d(0x02);
  lcd_send_c(0xF2);
  lcd_send_d(0x00);
  lcd_send_c(0x26);
  lcd_send_d(0x01);
  lcd_send_c(0xE0);
  lcd_send_d(0x0F);
  lcd_send_d(0x22);
  lcd_send_d(0x1C);
  lcd_send_d(0x1B);
  lcd_send_d(0x08);
  lcd_send_d(0x0F);
  lcd_send_d(0x48);
  lcd_send_d(0xB8);
  lcd_send_d(0x34);
  lcd_send_d(0x05);
  lcd_send_d(0x0C);
  lcd_send_d(0x09);
  lcd_send_d(0x0F);
  lcd_send_d(0x07);
  lcd_send_d(0x00);
  lcd_send_c(0xE1);
  lcd_send_d(0x00);
  lcd_send_d(0x23);
  lcd_send_d(0x24);
  lcd_send_d(0x07);
  lcd_send_d(0x10);
  lcd_send_d(0x07);
  lcd_send_d(0x38);
  lcd_send_d(0x47);
  lcd_send_d(0x4B);
  lcd_send_d(0x0A);
  lcd_send_d(0x13);
  lcd_send_d(0x06);
  lcd_send_d(0x30);
  lcd_send_d(0x38);
  lcd_send_d(0x0F);
  lcd_send_c(0x29);
}

// write the data to the LCD
void cyclic_LCD(struct ProcessImage* p_pi)
{
  // convert float to integer for comparison
  int8_t g = (int8_t)p_pi->bme280.temperature;
  int8_t n = (int8_t)((p_pi->bme280.temperature - g) * 10);
  int8_t g_mem = (int8_t)p_pi->bme280_memory.temperature;
  int8_t n_mem = (int8_t)((p_pi->bme280_memory.temperature - g_mem) * 10);

  if(g != g_mem || n != n_mem) {
    ESP_LOGI("", "Task_LCD: Writing temperature Old value: %d,%d. New value: %d,%d.\n", g_mem, n_mem, g, n);
    Paint_ClearWindows(180, 30, 180+17*5, 50, WHITE);
    Paint_DrawFloatNum(180, 30, p_pi->bme280.temperature, 1, &Font24, WHITE, BLACK);
  }

  if(p_pi->bme280.pressure != p_pi->bme280_memory.pressure) {
    ESP_LOGI("", "Task_LCD: Writing air pressure. Old value: %d. New value: %d\n", p_pi->bme280_memory.pressure, p_pi->bme280.pressure);
    Paint_ClearWindows(180, 60, 180+17*4, 80, WHITE);
    Paint_DrawNum(180, 60, p_pi->bme280.pressure, &Font24, WHITE, BLACK);
  }

  if(p_pi->bme280.humidity != p_pi->bme280_memory.humidity) {
    ESP_LOGI("", "Task_LCD: Writing humidity Old value: %d. New value: %d\n", p_pi->bme280_memory.humidity, p_pi->bme280.humidity);
    Paint_ClearWindows(180, 90, 180+17*3, 110, WHITE);
    Paint_DrawNum(180, 90, p_pi->bme280.humidity, &Font24, WHITE, BLACK);
  }

  p_pi->bme280_memory.temperature = p_pi->bme280.temperature;
  p_pi->bme280_memory.pressure = p_pi->bme280.pressure;
  p_pi->bme280_memory.humidity = p_pi->bme280.humidity;
}

void init_LCD()
{
  lcd_gpio_init();
  spi_lcd_init();
  lcd_reset();
  lcd_init();
  lcd_clear_display(WHITE);

  Paint_NewImage(LCD_2IN4_WIDTH, LCD_2IN4_HEIGHT, ROTATE_270, WHITE);
  Paint_SetClearFuntion(lcd_clear_display);
  Paint_SetDisplayFuntion(lcd_draw_point);

  Paint_DrawString_EN(10, 30, "Temperat:      C", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(10, 60, "Pressure:      hPa", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(10, 90, "Humidity:      %", &Font24, WHITE, BLACK);
}