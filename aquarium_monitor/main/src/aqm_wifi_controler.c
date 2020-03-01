#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp8266/eagle_soc.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "aqm_wifi_controler.h"

static const char *TAG = "AQM_WIFI_CONTROLER";

static EventGroupHandle_t aqm_wifi_event_group;
const static int CONNECTED_BIT = BIT0;

EventGroupHandle_t* get_aqm_wifi_event_group() {
    return &aqm_wifi_event_group;
}

static esp_err_t aqm_default_wifi_event_handler(void *ctx, system_event_t *event) {
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;

    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(aqm_wifi_event_group, CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGE(TAG, "Disconnect reason : %d", info->disconnected.reason);
            if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
                /*Switch to 802.11 bgn mode */
                esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCAL_11B | WIFI_PROTOCAL_11G | WIFI_PROTOCAL_11N);
            }
            esp_wifi_connect();
            xEventGroupClearBits(aqm_wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void aqm_wifi_init(aqm_wifi_init_handler_cb_t wifi_event_handler) {
    tcpip_adapter_init();

    aqm_wifi_event_group = xEventGroupCreate();

    if (!wifi_event_handler) {
        wifi_event_handler = aqm_default_wifi_event_handler;
    }

    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    ESP_LOGI(TAG, "start the WIFI SSID:[%s]", CONFIG_WIFI_SSID);

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Waiting for wifi");
    xEventGroupWaitBits(aqm_wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}
