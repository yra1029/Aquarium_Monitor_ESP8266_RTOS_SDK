#pragma once
#include "freertos/semphr.h"
/* This file includes general definitions of the device data structres*/

static const uint32_t LOOP_DELAY_MS = CONFIG_MQTT_PUBLISH_INTERVAL / 2;

typedef enum {
    THERM_DEV = 0,
    LUMIN_DEV,

    MAX_DEV_NUM
} aqm_device_t;

// #define MAX_DEV_NUM LAST_DEV+1

#define ENABLE_DEV_FLAG   (uint32_t)0x00000001
#define THERM_ENABLE      (uint32_t)(ENABLE_DEV_FLAG<<THERM_DEV)
#define LUMEN_ENABLE      (uint32_t)(ENABLE_DEV_FLAG<<LUMIN_DEV)

typedef void (*aqm_device_task_callback_t)(void *pvParameter);

typedef struct {
    char* task_name;
    char* mqtt_topic;
    void* last_value;
    aqm_device_task_callback_t device_task;
    SemaphoreHandle_t device_mutex;
    aqm_device_t type;
    size_t value_len;
} aqm_device_controler_t;

void aqm_device_controler_init(uint32_t enabled_dev);

aqm_device_controler_t* aqm_get_devices();
