#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "dht.h"
#include "shift_register.h"
#include "seven_seg.h"
#include "wifi.h"
#include "secret.h"
// In secrets.h:
// #define MY_WIFI_PASSWORD
// #define MY_WIFI_SSID
// #define MY_BROKER_IP

#include "lwip/err.h"
#include "lwip/sys.h"

void temp_task(void *measurement) {
    struct Temp_reading* meas = (struct Temp_reading*) measurement;
    while(1){
        read_temp(meas);
        if(!meas->err){
            ESP_LOGI("Results:", "Humidity: %d, Temp: %d", meas->hum_sig, meas->temp_sig);
        } else {
            ESP_LOGI("ERROR:", "Thermometer error");
        }
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

static esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = MY_BROKER_IP,
};

void post_messages_task(void *measurement) {
    struct Temp_reading* meas = (struct Temp_reading*) measurement;

    esp_mqtt_client_handle_t mqtthandle =  esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(mqtthandle);

    uint8_t old_temp_sig = 0;
    uint8_t old_temp_dec = 0;
    uint8_t old_hum_sig = 0;
    uint8_t old_hum_dec = 0;

    while(1){
        if(old_hum_sig != meas->hum_sig || old_hum_dec != meas->hum_dec){
            char humidity[16];
            sprintf(humidity, "%d.%d", meas->hum_sig, meas->hum_dec);
            esp_mqtt_client_publish(mqtthandle, "/home/humidity", humidity, 0, 1, 0);
            old_hum_sig = meas->hum_sig;
            old_hum_dec = meas->hum_dec;
            ESP_LOGI("MQTT", "Humidity published %d.%d", old_hum_sig, old_hum_dec);
        }
        if(old_temp_sig != meas->temp_sig || old_temp_dec != meas->temp_dec){
            char temperature[16];
            sprintf(temperature, "%d.%d", meas->temp_sig, meas->temp_dec);
            esp_mqtt_client_publish(mqtthandle, "/home/temperature", temperature, 0, 1, 0);
            old_temp_sig = meas->temp_sig;
            old_temp_dec = meas->temp_dec;
            ESP_LOGI("MQTT", "Temp published %d.%d", old_temp_sig, old_temp_dec);
        }
        if(meas->err == 1){
            esp_mqtt_client_publish(mqtthandle, "/errors", "misread", 0, 1, 0);
        } else {
            esp_mqtt_client_publish(mqtthandle, "/errors", "successful read", 0, 1, 0);

        }
        vTaskDelay(500);
    }
}

void display_on_seven_seg_task(void *measurement){
    struct Temp_reading* meas = (struct Temp_reading*) measurement;
    while(1){
        vTaskDelay(1000);
        display_on_seven_seg(meas->temp_sig);
        vTaskDelay(1000);
        display_on_seven_seg(meas->hum_sig);
    }
}

void setup() {
    setup_shift_register();
    setup_thermometer();
}


void app_main(void) {
    setup();
    wifi_init_connection();

    struct Temp_reading *measurement = malloc(sizeof(struct Temp_reading));
    measurement->err = 0;
    measurement->hum_sig = 0;
    measurement->hum_dec = 0;
    measurement->temp_sig = 0;
    measurement->temp_dec = 0;

    xTaskCreatePinnedToCore(temp_task, "Temp reading task", 6000, (void *) measurement, 24, NULL, 1);
    xTaskCreate(post_messages_task, "Temp posting task", 4000, (void *) measurement, 1, NULL);
    xTaskCreate(display_on_seven_seg_task, "Display on seven seg temp task", 4000, (void *) measurement, 1, NULL);

    clear_shift_register();

    while(1) {
        vTaskDelay(10000);
        ;;
    }
}
