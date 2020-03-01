#pragma once

#include "mqtt_client.h"

typedef esp_err_t (*aqm_mqtt_event_handler)(esp_mqtt_event_handle_t event);
void aqm_mqtt_client_init(aqm_mqtt_event_handler mqtt_event_handler);
