#pragma once

#include "esp_event.h"
#include "event_groups.h"

typedef esp_err_t (*aqm_wifi_init_handler_cb_t)(void *ctx, system_event_t *event);

EventGroupHandle_t* get_aqm_wifi_event_group();

void aqm_wifi_init(aqm_wifi_init_handler_cb_t wifi_event_handler);
