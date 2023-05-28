#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "Adafruit_Stemma_soil_sensor.h"

#define I2C_MASTER_NUM 0
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define DELAY_TIME_BETWEEN_READINGS_MS 100

static const char *TAG = "Plant Monitoring Sensor AIO";

extern "C" void app_main(void)
{
    int ret = ESP_OK;
    uint16_t moisture_value = 0;
    float temperature_value = 0;

    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_ERROR_CHECK(adafruit_stemma_soil_sensor_init(I2C_MASTER_NUM, I2C_SDA_PIN, I2C_SCL_PIN));

    while (1)
    {
        ret = adafruit_stemma_soil_sensor_read_moisture(I2C_MASTER_NUM, &moisture_value);

        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "Adafruit Stemma sensor value: =%u", moisture_value);
        }

        ret = adafruit_stemma_soil_sensor_read_temperature(I2C_MASTER_NUM, &temperature_value);

        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "Adafruit Stemma sensor value: =%f", temperature_value);
        }
        
    
        vTaskDelay((DELAY_TIME_BETWEEN_READINGS_MS) / portTICK_PERIOD_MS);
    }
}
