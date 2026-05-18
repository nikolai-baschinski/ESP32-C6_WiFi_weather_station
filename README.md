## This is a weather station project with ESP32-C6, BME280 and ILI9341 LCD.

![](/doc/IMG_5289.jpg)

![](/doc/IMG_5290.jpg)

![](/doc/block_diagramm.png)


Connections (https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c6/esp32-c6-devkitc-1/user_guide.html#getting-started)


| LCD Pin | ESP32-C6 GPIO | Function     |
| ------- | --------------| ------------ |
| VCC     | 3.3V          | Vcc          |
| GND     | GND           | GND          |
| DIN     | GPIO7         | MOSI         |
| CLK     | GPIO6         | SCLK         |
| CS      | GPIO10        | Chip Select  |
| DC      | GPIO2         | Data/Command |
| RST     | GPIO3         | Reset        |
| BL      | 3.3V          | Backlight    |



| BME280   | ESP32-C6 GPIO | Function     |
|----------|---------------|--------------|
| VCC      | 3,3V          | Vcc          |
| GND      | GND           | GND          |
| SCL      | GPIO4         | Clock        |
| SDA      | GPIO5         | Data         |
| ADDR     | not conn.     |              |
| CS       | 3,3V          | Vcc          |


Hardware
BME280: https://seengreat.com/product/207/bme280-environmental-sensor?srsltid=AfmBOorvlymsT9w0Ea-JBnftBbgADYcXMKpadnPHUyHl7X1wOO5TTgUa

LCD: https://www.waveshare.com/wiki/2.4inch_LCD_Module?srsltid=AfmBOoqtv3bq-mZfPtsi2BxiewwQnIkomXrloIzpVwGw_HnrOcmvQZar

ESP32-C6-DevKitC-1: https://www.reichelt.de/de/de/shop/produkt/entwicklungsboard_esp32-c6-wroom-1_u-380385

1. Clone
2. Import the bme280-node and the lcd projects separately in the Espressif IDE
3. Change the Router SSID, the passwort and the LCD node IP in the wifi.c
4. Build
5. Flash
6. Ping both nodes from your PC
7. Test with POST request (test folder)
