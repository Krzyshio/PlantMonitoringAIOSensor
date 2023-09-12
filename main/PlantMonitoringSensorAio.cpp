#include "esp_system.h"
#include "esp_log.h"
#include "Adafruit_Stemma_soil_sensor.h"
#include "ssd1306.h"
#include "dht11.h"
#include "sdkconfig.h"
#include "nvs_flash.h"
#include "wifiManager.h"
#include "mqttManager.h"

#define I2C_MASTER_NUM   I2C_NUM_0
#define I2C_SDA_PIN      CONFIG_PIN_SDA
#define I2C_SCL_PIN      CONFIG_PIN_SCL
#define DHT11_PIN        CONFIG_PIN_DHT11
#define SSD1306_DISPLAY_ADDRESS    CONFIG_SSD1306_DISPLAY_ADDRESS
#define SSD1306_DISPLAY_WIDTH      CONFIG_SSD1306_DISPLAY_WIDTH
#define SSD1306_DISPLAY_HEIGHT     CONFIG_SSD1306_DISPLAY_HEIGHT
#define DELAY_TIME_BETWEEN_READINGS_MS 1000
#define DEVICE_UUID CONFIG_DEVICE_UUID

static const char *TAG = "Plant Monitoring Sensor AIO";

static void read_soil_sensor_values(uint16_t *moisture, float *temperature) {
    int ret;

    ret = adafruit_stemma_soil_sensor_read_moisture(I2C_MASTER_NUM, moisture);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Adafruit Stemma sensor moisture value: =%u", *moisture);
    }

    ret = adafruit_stemma_soil_sensor_read_temperature(I2C_MASTER_NUM, temperature);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Adafruit Stemma sensor temperature value: =%.2f", *temperature);
    }
}

static void read_dht11_values(float *temp, float *humidity) {
    struct dht11_reading reading = DHT11_read();

    if (reading.status == DHT11_OK) {
        *humidity = reading.humidity;
        *temp = reading.temperature;
        ESP_LOGI(TAG, "DHT11 - Temp: %.2f Humidity: %.2f", *temp, *humidity);
    } else {
        const char *error = (reading.status == DHT11_CRC_ERROR) ? "CRC Error" : "Timeout Error";
        ESP_LOGE(TAG, "Error reading from DHT11: %s", error);
    }
}

extern "C" void app_main(void) {
    uint16_t moisture_value = 0;
    float stemma_temp_value = 0.0f;
    float dht_temp = 0.0f;
    float dht_humidity = 0.0f;
    char payload[16];
    char topic[64];
    char buf[100];

    nvs_flash_init();
    wifi_connection();

    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_ERROR_CHECK(adafruit_stemma_soil_sensor_init(I2C_MASTER_NUM, I2C_SDA_PIN, I2C_SCL_PIN));

    SSD1306_t dev;
    dev._address = SSD1306_DISPLAY_ADDRESS;
    ssd1306_init(&dev, SSD1306_DISPLAY_WIDTH, SSD1306_DISPLAY_HEIGHT);

    DHT11_init((gpio_num_t) DHT11_PIN);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    mqtt_app_start();

    while (1) {
        read_soil_sensor_values(&moisture_value, &stemma_temp_value);
        read_dht11_values(&dht_temp, &dht_humidity);

        sprintf(topic, "soil_moisture.%s", DEVICE_UUID);
        sprintf(payload, "%u", moisture_value);
        mqtt_publish(topic, payload);

        sprintf(topic, "temperature.%s", DEVICE_UUID);
        sprintf(payload, "%.2f", dht_temp);
        mqtt_publish(topic, payload);

        sprintf(topic, "humidity.%s", DEVICE_UUID);
        sprintf(payload, "%.2f", dht_humidity);
        mqtt_publish(topic, payload);

        strcpy(buf, "Soil Sensor");
        ssd1306_display_text(&dev, 0, buf, 11, true);
        sprintf(buf, "Moisture: %u", moisture_value);
        ssd1306_display_text(&dev, 1, buf, strlen(buf), false);
        sprintf(buf, "Temp: %.2f°C", stemma_temp_value);
        ssd1306_display_text(&dev, 2, buf, strlen(buf), false);

        sprintf(buf, "DHT11 Temp: %.2f°C", dht_temp);
        ssd1306_display_text(&dev, 3, buf, strlen(buf), false);
        sprintf(buf, "Humidity: %.2f%%", dht_humidity);
        ssd1306_display_text(&dev, 4, buf, strlen(buf), false);

        vTaskDelay((DELAY_TIME_BETWEEN_READINGS_MS) / portTICK_PERIOD_MS);

        ssd1306_clear_screen(&dev, false);
    }
}
