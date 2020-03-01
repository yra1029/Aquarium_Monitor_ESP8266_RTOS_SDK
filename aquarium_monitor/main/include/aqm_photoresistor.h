#pragma once

#include "driver/adc.h"

#include "aqm_device_controler.h"

void aqm_photoresistor_init(aqm_device_controler_t* pres_controler, aqm_device_task_callback_t device_task,
                                adc_config_t* adc_config);
