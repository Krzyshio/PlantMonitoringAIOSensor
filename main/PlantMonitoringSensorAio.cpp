#include "esp_system.h"
#include "esp_log.h"
#include "Adafruit_Stemma_soil_sensor.h"
#include "ssd1306.h"
#include "dht11.h"
#include "sdkconfig.h"
#include "nvs_flash.h"
#include "wifiManager.h"
#include "mqttManager.h"

#define BUFFER_SIZE 3600

static const char *TAG = "Plant Monitoring Sensor AIO";

static float avg_moisture = 0.0f;
static float avg_stemma_temp = 0.0f;
static float avg_dht_temp = 0.0f;
static float avg_dht_humidity = 0.0f;
static float moisture_buffer[BUFFER_SIZE] = {0};
static float stemma_temp_buffer[BUFFER_SIZE] = {0};
static float dht_temp_buffer[BUFFER_SIZE] = {0};
static float dht_humidity_buffer[BUFFER_SIZE] = {0};
static int buffer_index = 0;
static SemaphoreHandle_t dataSemaphore;
static bool data_published = false;

static void read_soil_sensor_values(uint16_t *moisture, float *temperature) {
    if (adafruit_stemma_soil_sensor_read_moisture(I2C_NUM_0, moisture) == ESP_OK) {
        ESP_LOGI(TAG, "Adafruit Stemma sensor moisture value: =%u", *moisture);
    }

    if (adafruit_stemma_soil_sensor_read_temperature(I2C_NUM_0, temperature) == ESP_OK) {
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

static void update_average() {
    avg_moisture = 0;
    avg_stemma_temp = 0;
    avg_dht_temp = 0;
    avg_dht_humidity = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        avg_moisture += moisture_buffer[i];
        avg_stemma_temp += stemma_temp_buffer[i];
        avg_dht_temp += dht_temp_buffer[i];
        avg_dht_humidity += dht_humidity_buffer[i];
    }
    avg_moisture /= BUFFER_SIZE;
    avg_stemma_temp /= BUFFER_SIZE;
    avg_dht_temp /= BUFFER_SIZE;
    avg_dht_humidity /= BUFFER_SIZE;
}

static void data_collection_task(void *pvParameters) {
    while (1) {
        uint16_t moisture_value = 0;
        float stemma_temp_value = 0.0f;
        float dht_temp = 0.0f;
        float dht_humidity = 0.0f;

        read_soil_sensor_values(&moisture_value, &stemma_temp_value);
        read_dht11_values(&dht_temp, &dht_humidity);

        xSemaphoreTake(dataSemaphore, portMAX_DELAY);
        moisture_buffer[buffer_index] = moisture_value;
        stemma_temp_buffer[buffer_index] = stemma_temp_value;
        dht_temp_buffer[buffer_index] = dht_temp;
        dht_humidity_buffer[buffer_index] = dht_humidity;
        buffer_index = (buffer_index + 1) % BUFFER_SIZE;
        if (buffer_index == 0) {
            update_average();
        }
        xSemaphoreGive(dataSemaphore);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void publish_average_values(const char *uuid) {
    char topic[64];
    char payload[16];

    sprintf(topic, "soil_moisture.%s", uuid);
    sprintf(payload, "%.2f", avg_moisture);
    mqtt_publish(topic, payload);

    sprintf(topic, "temperature.%s", uuid);
    sprintf(payload, "%.2f", avg_dht_temp);
    mqtt_publish(topic, payload);

    sprintf(topic, "humidity.%s", uuid);
    sprintf(payload, "%.2f", avg_dht_humidity);
    mqtt_publish(topic, payload);
}

static void mqtt_publish_task(void *pvParameters) {
    char uuid[] = CONFIG_DEVICE_UUID;
    while (1) {
        xSemaphoreTake(dataSemaphore, portMAX_DELAY);

        update_average();
        publish_average_values(uuid);

        data_published = true;
        xSemaphoreGive(dataSemaphore);
        vTaskDelay(3600000 / portTICK_PERIOD_MS);
    }
}

static void display_values_on_screen(SSD1306_t *dev) {
    char buf[100];

    strcpy(buf, "  Plant Sensor  ");
    ssd1306_display_text(dev, 0, buf, 16, true);

    sprintf(buf, "Moisture: %.2f", avg_moisture);
    ssd1306_display_text(dev, 2, buf, strlen(buf), false);

    sprintf(buf, "Temp: %.2fC", avg_dht_temp);
    ssd1306_display_text(dev, 3, buf, strlen(buf), false);

    sprintf(buf, "Humidity: %.2f%%", avg_dht_humidity);
    ssd1306_display_text(dev, 4, buf, strlen(buf), false);
}

static void display_published_message(SSD1306_t *dev) {
    char msg[] = " Data published";
    ssd1306_display_text(dev, 4, msg, strlen(msg), true);
}

static void display_task(void *pvParameters) {
    SSD1306_t dev = {
            ._address = CONFIG_SSD1306_DISPLAY_ADDRESS,
    };
    ssd1306_init(&dev, CONFIG_SSD1306_DISPLAY_WIDTH, CONFIG_SSD1306_DISPLAY_HEIGHT);

    while (1) {
        if (data_published) {
            display_published_message(&dev);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            data_published = false;
        } else {
            display_values_on_screen(&dev);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ssd1306_clear_screen(&dev, false);
    }
}

extern "C" void app_main(void) {
    nvs_flash_init();
    wifi_connection();

    esp_log_level_set("*", ESP_LOG_INFO);

    adafruit_stemma_soil_sensor_init(I2C_NUM_0, CONFIG_PIN_SDA, CONFIG_PIN_SCL);
    DHT11_init((gpio_num_t) CONFIG_PIN_DHT11);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    mqtt_app_start();

    dataSemaphore = xSemaphoreCreateMutex();
    xTaskCreate(&data_collection_task, "data_collection_task", 4096, NULL, 5, NULL);
    xTaskCreate(&mqtt_publish_task, "mqtt_publish_task", 4096, NULL, 5, NULL);
    xTaskCreate(&display_task, "display_task", 4096, NULL, 5, NULL);
}
