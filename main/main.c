#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_log.h"
// Logging: ESP_LOGI("", "", "");
#include "esp_event.h"
#include "nvs_flash.h"

#include "dht.h"
#include "shift_register.h"
#include "seven_seg.h"
#include "utils.h"
#include "wifi.h"

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
    xTaskCreate(temp_task, "Temp reading task", 5000, (void *) &measurement, 2, NULL);
    wifi_init_connection();

    clear_shift_register();

    int i = 0;
    while(1) {
        vTaskDelay(500);
        // display_on_seven_seg(measurement.temp_sig);
        vTaskDelay(500);
        // display_on_seven_seg(measurement.hum_sig);
    }
}