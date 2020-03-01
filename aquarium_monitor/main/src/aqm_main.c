#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "aqm_wifi_controler.h"
#include "aqm_mqtt_client.h"

static const char *TAG = "AQM_MAIN";

void app_main()
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    //ESP_LOGI(TAG, "[APP] certificate content: %s", (const char *)cloudmqtt_pem_start);

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    nvs_flash_init();

    aqm_wifi_init(NULL);

    aqm_mqtt_client_init(NULL);
}
