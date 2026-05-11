#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_http_server.h"
#include "esp_netif_ip_addr.h"

#include "wifi.h"
#include "../ProcessImage.h"

#define WIFI_SSID      "FRITZ!Box 7530 UTR" // TODO
#define WIFI_PASSWORD  "54960167273"  // TODO

struct ProcessImage* p_pi = nullptr;
static const char *TAG = "HTTP_SERVER";
static httpd_handle_t server = NULL;

static esp_err_t root_get_handler(httpd_req_t *req)
{
  const char *response = "ESP32-C6 HTTP Server OK";
  httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
  ESP_LOGI(TAG, "HTTP response to: %s, %i", req->uri, req->method);
  return ESP_OK;
}

static esp_err_t environment_post_handler(httpd_req_t *req)
{
  char content[128];

  int recv_len = httpd_req_recv(req, content, sizeof(content) - 1);

  if (recv_len <= 0) {
    ESP_LOGE(TAG, "Failed to receive POST data");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  content[recv_len] = '\0';

  ESP_LOGI(TAG, "Received JSON: %s", content);

  float temperature = 0.0f;
  uint16_t pressure = 0;
  unsigned int humidity = 0;

  int matched = sscanf(content, "{\"temperature\":%f,\"pressure\":%hu,\"humidity\":%u}", &temperature, &pressure, &humidity);

  if (matched != 3) {
    ESP_LOGE(TAG, "JSON parse failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  p_pi->bme280.temperature = temperature;
  p_pi->bme280.pressure = pressure;
  p_pi->bme280.humidity = (uint8_t)humidity;

  ESP_LOGI(TAG, "Temperature: %.1f", p_pi->bme280.temperature);
  ESP_LOGI(TAG, "Pressure: %u", p_pi->bme280.pressure);
  ESP_LOGI(TAG, "Humidity: %u", p_pi->bme280.humidity);

  httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);

  return ESP_OK;
}

static httpd_handle_t start_webserver(void)
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  if (httpd_start(&server, &config) == ESP_OK) {

    httpd_uri_t root = {
      .uri      = "/",
      .method   = HTTP_GET,
      .handler  = root_get_handler,
      .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &root);

    httpd_uri_t environment = {
      .uri      = "/environment",
      .method   = HTTP_POST,
      .handler  = environment_post_handler,
      .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &environment);

    ESP_LOGI(TAG, "HTTP server started");
    return server;
  }

  ESP_LOGE(TAG, "HTTP server couldn't be started");
  return NULL;
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  }
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    ESP_LOGI(TAG, "WiFi disconnected, reconnect...");
    esp_wifi_connect();
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "IP received:");
    ESP_LOGI(TAG, IPSTR, IP2STR(&event->ip_info.ip));
    if (server == NULL) {
      server = start_webserver();
    }
  }
}

static void wifi_init_sta(void)
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_t *netif = esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  /* Register the event handler*/
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL,NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

  ESP_ERROR_CHECK(esp_netif_set_hostname(netif, "esp-lcd-node"));

  /* WiFi config */
  wifi_config_t wifi_config = {
    .sta = {
      .ssid = WIFI_SSID,
      .password = WIFI_PASSWORD,
      .threshold.authmode = WIFI_AUTH_WPA2_PSK,
    },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI(TAG, "wifi_init_sta finished.");
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
