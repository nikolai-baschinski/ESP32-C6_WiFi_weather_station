#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi.h"
#include "../ProcessImage.h"

#define WIFI_SSID      "SSID" // TODO
#define WIFI_PASSWORD  "PASSWORD"  // TODO
#define MY_HOST_NAME   "esp-bme280-node"
#define LCD_NODE_URL   "http://192.168.178.34/environment" // TODO

struct ProcessImage* p_pi = nullptr;

static const char *TAG = "WIFI";

void send_environment_data(void)
{
  char json_response[128];

  snprintf(
    json_response,
    sizeof(json_response),
    "{"
    "\"temperature\":%.1f,"
    "\"pressure\":%u,"
    "\"humidity\":%u"
    "}",
    p_pi->bme280.temperature,
    p_pi->bme280.pressure,
    p_pi->bme280.humidity);

  ESP_LOGI(TAG, "Sending JSON: %s", json_response);

  esp_http_client_config_t config = {
    .url = LCD_NODE_URL,
    .method = HTTP_METHOD_POST,
  };

  esp_http_client_handle_t client = esp_http_client_init(&config);

  esp_http_client_set_header(client, "Content-Type", "application/json");

  esp_http_client_set_post_field(client, json_response, strlen(json_response));

  esp_err_t err = esp_http_client_perform(client);

  if (err == ESP_OK) {
    ESP_LOGI(TAG, "HTTP POST successful");
    ESP_LOGI(TAG, "HTTP status = %d", esp_http_client_get_status_code(client));
  }
  else {
    ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(err));
  }

  esp_http_client_cleanup(client);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  }
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    ESP_LOGI(TAG, "WiFi disconnected, reconnect...");
    esp_wifi_connect();
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {

    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

    ESP_LOGI(TAG, "IP received:");
    ESP_LOGI(TAG, IPSTR, IP2STR(&event->ip_info.ip));

    send_environment_data();
  }
}

static void wifi_init_sta(void)
{
  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_t *netif = esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  /* Register the event handler */
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

  ESP_ERROR_CHECK(esp_netif_set_hostname(netif, MY_HOST_NAME));

  /* WiFi config */
  wifi_config_t wifi_config = {
    .sta = {
      .ssid = WIFI_SSID,
      .password = WIFI_PASSWORD,
      .threshold.authmode = WIFI_AUTH_WPA2_PSK,
    },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "wifi_init_sta finished");
}

void init_wifi(struct ProcessImage* p_pi_param)
{
  p_pi = p_pi_param;

  esp_err_t ret = nvs_flash_init();

  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }

  ESP_ERROR_CHECK(ret);

  wifi_init_sta();
}