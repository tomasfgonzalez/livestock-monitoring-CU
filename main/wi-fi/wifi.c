/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : wifi.c
  * @brief          : Wi-Fi connection handler
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 Tomas Gonzalez & Brian Morris
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  */

#include "wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "display/status.h"

static const char *WIFI_TAG = "Wi-Fi";
static EventGroupHandle_t wifi_event_group;
static esp_netif_t *sta_netif = NULL;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  ESP_LOGI(WIFI_TAG, "Event: %s, ID: %ld", event_base, event_id);
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    esp_wifi_connect();
    update_wifi_status("Offline");
    wifi_event_sta_disconnected_t *disc = (wifi_event_sta_disconnected_t*) event_data;
    ESP_LOGW("Wi-Fi", "Disconnected, reason=%d", disc->reason);
    ESP_LOGI(WIFI_TAG, "Retrying connection…");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = event_data;
    ESP_LOGI(WIFI_TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    update_wifi_status("Online");
    xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

void wifi_init_sta(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

  wifi_config_t wifi_config = {
    .sta = {
      .ssid = "Brimix",
      .password = "Holis420",
      .threshold.authmode = WIFI_AUTH_WPA2_PSK,
      .pmf_cfg = {
        .capable = true,
        .required = false
      },
    },
  };

  ESP_LOGI(WIFI_TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(WIFI_TAG, "wifi_init_sta finished.");
}

void wifi_start() {
  EventBits_t bits = xEventGroupWaitBits(
    wifi_event_group,
    WIFI_CONNECTED_BIT,
    pdFALSE,
    pdFALSE,
    portMAX_DELAY
  );
  
  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(WIFI_TAG, "We are online 🎉");
    update_wifi_status("Online");
  } else {
    ESP_LOGE(WIFI_TAG, "We are offline 💀");
    update_wifi_status("Offline");
  }
}
