#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"
// Logging: ESP_LOGI("", "", "");
#include "esp_event.h"
#include "nvs_flash.h"

#include "dht.h"
#include "shift_register.h"
#include "seven_seg.h"
#include "utils.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_SSID CONFIG_WIFI_SSID;
#define WIFI_PWD CONFIG_WIFI_PASSWORD;
/* Wifi connect signal group*/
static EventGroupHandle_t s_wifi_event_group;

/* From the ESP docs re. WiFi: The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// TODO!
void setup_wifi() {
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&config);
}

void temp_task(void *measurement) {
    struct Temp_reading* meas = (struct Temp_reading*) measurement;
    while(1){
        read_temp(meas);
        if(!meas->err){
            ESP_LOGI("Results:", "Humidity: %d, Temp: %d", meas->hum_sig, meas->temp_sig);
        } else {
            ESP_LOGI("ERROR:", "Thermometer error");
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void setup() {
    setup_shift_register();
    setup_thermometer();
}

void app_main(void) {
    setup();

    struct Temp_reading measurement = { 0 , 0 , 0 , 0, 0 };

    xTaskCreate(temp_task, "Temp reading task", 5000, (void *) &measurement, 1, NULL);
    clear_shift_register();

    int i = 0;
    while(1) {
        vTaskDelay(500);
        display_on_seven_seg(measurement.temp_sig);
        vTaskDelay(500);
        display_on_seven_seg(measurement.hum_sig);
    }
}