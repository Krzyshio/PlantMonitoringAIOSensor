#include "string.h"
#include "esp_system.h"
#include "esp_log.h"
#include "Adafruit_Stemma_soil_sensor.h"
#include "ssd1306.h"
#include "sdkconfig.h"

#define I2C_MASTER_NUM   I2C_NUM_0
#define I2C_SDA_PIN      CONFIG_PIN_SDA
#define I2C_SCL_PIN      CONFIG_PIN_SCL
#define DHT11_PIN        CONFIG_PIN_DHT11
#define SSD1306_DISPLAY_ADDRESS    CONFIG_SSD1306_DISPLAY_ADDRESS
#define SSD1306_DISPLAY_WIDTH      CONFIG_SSD1306_DISPLAY_WIDTH
#define SSD1306_DISPLAY_HEIGHT     CONFIG_SSD1306_DISPLAY_HEIGHT
#define DELAY_TIME_BETWEEN_READINGS_MS 1000

static const char *TAG = "Plant Monitoring Sensor AIO";

extern "C" void app_main(void)
{
    int ret = ESP_OK;
    uint16_t moisture_value = 0;
    float temperature_value = 0;

    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_ERROR_CHECK(adafruit_stemma_soil_sensor_init(I2C_MASTER_NUM, I2C_SDA_PIN, I2C_SCL_PIN));

    SSD1306_t dev;
    dev._address = SSD1306_DISPLAY_ADDRESS;
    ssd1306_init(&dev, SSD1306_DISPLAY_WIDTH, SSD1306_DISPLAY_HEIGHT);

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
            ESP_LOGI(TAG, "Adafruit Stemma sensor value: =%.2f", temperature_value);
        }

        ssd1306_clear_screen(&dev, false);

        char buf[100];
        sprintf(buf, "Moisture: %u", moisture_value);
        ssd1306_display_text(&dev, 0, "Soil Sensor", 11, true);
        ssd1306_display_text(&dev, 1, buf, strlen(buf), false);
        sprintf(buf, "Temp: %.2f", temperature_value);
        ssd1306_display_text(&dev, 2, buf, strlen(buf), false);

        vTaskDelay((DELAY_TIME_BETWEEN_READINGS_MS) / portTICK_PERIOD_MS);
    }
}
