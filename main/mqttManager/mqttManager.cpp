#include "mqttManager.h"
#include "esp_log.h"

static const char *TAG = "MQTT_TCP";

static esp_mqtt_client_handle_t global_mqtt_client = NULL;

static void log_mqtt_event(const char *event_description) {
    ESP_LOGI(TAG, "%s", event_description);
}

static void log_mqtt_event_with_id(const char *event_description, int msg_id) {
    ESP_LOGI(TAG, "%s, msg_id=%d", event_description, msg_id);
}

static void handle_mqtt_data_event(esp_mqtt_event_handle_t event) {
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("\nTOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            log_mqtt_event("MQTT_EVENT_CONNECTED");
            break;
        case MQTT_EVENT_DISCONNECTED:
            log_mqtt_event("MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
        case MQTT_EVENT_UNSUBSCRIBED:
        case MQTT_EVENT_PUBLISHED:
            log_mqtt_event_with_id("MQTT_EVENT_SUBSCRIBED", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            handle_mqtt_data_event(event);
            break;
        case MQTT_EVENT_ERROR:
            log_mqtt_event("MQTT_EVENT_ERROR");
            break;
        default:
            log_mqtt_event_with_id("Other event id", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
    mqtt_event_handler_cb((esp_mqtt_event_handle_t) event_data);
}

static const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
                .address = {
                        .hostname = CONFIG_MQTT_HOSTNAME,
                        .transport = MQTT_TRANSPORT_OVER_TCP,
                        .port = CONFIG_MQTT_PORT
                }
        },
        .credentials = {
                .username = CONFIG_MQTT_USERNAME,
                .authentication = {
                        .password = CONFIG_MQTT_PASSWORD
                }
        }
};

void mqtt_app_start(void) {
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client.");
        return;
    }

    global_mqtt_client = client;
    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t) ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void mqtt_publish(const char *topic, const char *data) {
    if (global_mqtt_client) {
        esp_mqtt_client_publish(global_mqtt_client, topic, data, 0, 1, 0);
    } else {
        ESP_LOGE(TAG, "MQTT client not initialized.");
    }
}
