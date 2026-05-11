# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS main.c bme/bme.c bme/I2C.c wifi/wifi.c     # list the source files of this component
    INCLUDE_DIRS bme wifi    # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES esp_driver_i2c esp_driver_gpio esp_wifi
        esp_event
        esp_netif
        nvs_flash
        esp_http_server   # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
