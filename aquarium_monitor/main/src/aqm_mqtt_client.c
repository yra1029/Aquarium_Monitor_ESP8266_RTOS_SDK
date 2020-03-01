#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_log.h"

#include "aqm_mqtt_client.h"
#include "aqm_device_controler.h"

static const char *TAG = "AQM_MQTT_CLIENT";

#if CONFIG_BROKER_CERTIFICATE_OVERRIDDEN == 1
static const uint8_t cloudmqtt_pem_start[]  = "-----BEGIN CERTIFICATE-----\n" CONFIG_BROKER_CERTIFICATE_OVERRIDE "\n-----END CERTIFICATE-----";
#else
extern const uint8_t cloudmqtt_pem_start[]   asm("_binary_cloudmqtt_pem_start");
#endif
extern const uint8_t cloudmqtt_pem_end[]   asm("_binary_cloudmqtt_pem_start");

static SemaphoreHandle_t publish_mutex;
static bool              publish_enabled;

#define PUBLISH_TASK_NAME        "aqm_publish_task"
#define PUBLISH_TASK_STACK_SIZE  2048
#define PUBLISH_TASK_PRIORITY    5
#define PUBLISH_VALUE_BUFFER_LEN 10

static void aqm_publish_task(void* Tclient) {
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)Tclient;
    int msg_id;

    aqm_device_controler_t* devices = aqm_get_devices();
    char value_to_send[PUBLISH_VALUE_BUFFER_LEN];

    while (1) {
        xSemaphoreTake(publish_mutex, portMAX_DELAY);

        if (!publish_enabled) {
            xSemaphoreGive(publish_mutex);
            break;
        }

        xSemaphoreGive(publish_mutex);

        for (int i = 0; i < MAX_DEV_NUM;i++) {

            if (devices[i].device_mutex != NULL) {

                xSemaphoreTake(devices[i].device_mutex, portMAX_DELAY);

                sprintf(value_to_send, "%f", (*(float*)devices[i].last_value));

                ESP_LOGI(TAG, "value to publish = %f", (*(float*)devices[i].last_value));

                msg_id = esp_mqtt_client_publish(client, devices[i].mqtt_topic, value_to_send, devices[i].value_len, 0, 0);

                ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

                xSemaphoreGive(devices[i].device_mutex);
            }
        }
        vTaskDelay(CONFIG_MQTT_PUBLISH_INTERVAL / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

static inline void aqm_start_publish(esp_mqtt_client_handle_t client) {
    xTaskCreate(aqm_publish_task,
                  PUBLISH_TASK_NAME,
                  PUBLISH_TASK_STACK_SIZE,
                  (void*)client,
                  PUBLISH_TASK_PRIORITY,
                  NULL);
}

static esp_err_t aqm_mqtt_default_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

            msg_id = esp_mqtt_client_subscribe(client, CONFIG_MQTT_SUB_TOPIC, 0);

            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            publish_enabled = true;

            aqm_start_publish(client);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");

            xSemaphoreTake(publish_mutex, portMAX_DELAY);

            publish_enabled = false;

            xSemaphoreGive(publish_mutex);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

void aqm_mqtt_client_init(aqm_mqtt_event_handler mqtt_event_handler)
{
    if (!mqtt_event_handler) {
        mqtt_event_handler = aqm_mqtt_default_event_handler;
    }

    const esp_mqtt_client_config_t mqtt_cfg = {
        .host = CONFIG_MQTT_BROKER,
        .port = CONFIG_MQTT_PORT,
        .username = CONFIG_MQTT_USERNAME,
        .password = CONFIG_MQTT_PASSWORD,
        .lwt_qos = CONFIG_DEFAULT_MQTT_SUB_QOS,
        .event_handle = mqtt_event_handler,
        .cert_pem = (const char *)cloudmqtt_pem_start,
        .transport = MQTT_TRANSPORT_OVER_SSL
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);

    aqm_device_controler_init(THERM_ENABLE|LUMEN_ENABLE);

    publish_mutex = xSemaphoreCreateMutex();

    if (!publish_mutex) {
        ESP_LOGE(TAG, "Could not create the mutex");
    }
}
