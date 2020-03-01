#pragma once

#include "aqm_device_controler.h"

#define MAX_SENSORS 1

void aqm_thermometer_init(aqm_device_controler_t* therm_controler, aqm_device_task_callback_t device_task);
