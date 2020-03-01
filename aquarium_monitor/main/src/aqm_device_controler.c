#include <stdint.h>
#include <stdlib.h>

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

#include "aqm_thermometer.h"
#include "aqm_photoresistor.h"

static const char *TAG = "AQM_DEVICE_CONTROLER";

#define DEVICE_TASK_PRIORITY 4
#define DEVICE_TASK_STACK_SIZE configMINIMAL_STACK_SIZE * 2

static aqm_device_controler_t devices[MAX_DEV_NUM];

static inline void aqm_start_task(aqm_device_controler_t* device) {
    static int counter = 0;
    counter++;
    ESP_LOGI(TAG, "aqm_start_task before task start count %d", counter);
    ESP_LOGI(TAG, "aqm_start_task before task start count %d", device->value_len);
    xTaskCreate(device->device_task,
                      device->task_name,
                      DEVICE_TASK_STACK_SIZE,
                      NULL,
                      DEVICE_TASK_PRIORITY,
                      NULL);
    ESP_LOGI(TAG, "aqm_start_task after task start count %d", counter);
}

aqm_device_controler_t* aqm_get_devices() {
    return devices;
}

void aqm_device_controler_init(uint32_t enabled_dev) {
    memset(devices, 0, sizeof(aqm_device_controler_t) * MAX_DEV_NUM);

    if (enabled_dev & THERM_ENABLE) {
        aqm_thermometer_init(&devices[THERM_DEV], NULL);
    }

    if (enabled_dev & LUMEN_ENABLE) {
        ESP_LOGI(TAG, "aqm_device_controler_init before phtotresistor init");
        aqm_photoresistor_init(&devices[LUMIN_DEV], NULL, NULL);
        ESP_LOGI(TAG, "aqm_device_controler_init after phtotresistor init");

    }

    for (int i = 0; i < MAX_DEV_NUM; i++) {
        if (devices[i].task_name != NULL) {
            aqm_start_task(&devices[i]);
        }
    }
}
