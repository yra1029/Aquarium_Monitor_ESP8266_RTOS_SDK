#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/adc.h"

#include "aqm_photoresistor.h"

static const char *TAG = "AQM_PHOTORESISTOR";

static const int DEVISION_CLOCK   = 8; // ADC sample collection clock = 80MHz/clk_div = 10MHz
static const size_t LUX_VALUE_LEN = 3;

static const float VIN  = 3.3; // V power voltage
static const int RESISTANCE = 10000; //ohm resistance value

float lux_value;

static inline float aqm_avalue_to_lumen(int raw) {

    float Vout = (float)raw * (VIN / (float)1023);// Conversion analog to voltage
    float RLDR = (RESISTANCE * (VIN - Vout)) / Vout; // Conversion voltage to resistance
    int phys = 500 / (RLDR / 1000); // Conversion resitance to lumen
    return phys;
}

static void aqm_photoresistor_default_task(void *pvParameter)
{
    ESP_LOGI(TAG, "aqm_photoresistor_default_task call");

    uint16_t adc_data[100];

    while (1) {
        if (ESP_OK == adc_read(&adc_data[0])) {
            //lux_value = (float)adc_data[0];
            vTaskDelay(1 / portTICK_PERIOD_MS);
            lux_value = aqm_avalue_to_lumen(adc_data[0]);
            ESP_LOGI(TAG, "The light sensor detects value: %d\r\n", adc_data[0]);
        }

        vTaskDelay(LOOP_DELAY_MS / portTICK_PERIOD_MS);
    }
}

static adc_config_t* get_default_adc_config() {
    static adc_config_t adc_config;

    // Depend on menuconfig->Component config->PHY->vdd33_const value
    // When measuring system voltage(ADC_READ_VDD_MODE), vdd33_const must be set to 255.
    adc_config.mode = ADC_READ_TOUT_MODE;
    adc_config.clk_div = DEVISION_CLOCK; // ADC sample collection clock = 80MHz/clk_div = 10MHz

    ESP_LOGI(TAG, "get_default_adc_config call");
    return &adc_config;
}

void aqm_photoresistor_init(aqm_device_controler_t* pres_controler, aqm_device_task_callback_t device_task, 
                                adc_config_t* adc_config) {

    ESP_LOGI(TAG, "aqm_photoresistor_init call");

    if (!device_task) {
        device_task = aqm_photoresistor_default_task;
    }

    pres_controler->task_name = "lighting_measure_task";
    pres_controler->mqtt_topic = CONFIG_MQTT_PUB_TOPIC_LUMIN;
    pres_controler->last_value = &lux_value;

    pres_controler->type = LUMIN_DEV;
    pres_controler->device_task = device_task;

    if (!adc_config) {
        adc_config = get_default_adc_config();
    }

    ESP_LOGI(TAG, "aqm_photoresistor_init before adc_init");
    ESP_ERROR_CHECK(adc_init(adc_config));
    ESP_LOGI(TAG, "aqm_photoresistor_init after adc_init");
    pres_controler->device_mutex = xSemaphoreCreateMutex();

    if (!pres_controler->device_mutex) {
        ESP_LOGE(TAG, "Could not create the mutex");
    }

    pres_controler->value_len = LUX_VALUE_LEN;
}
