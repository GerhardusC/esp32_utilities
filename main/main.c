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
#include "wifi.h"

#include "lwip/err.h"
#include "lwip/sys.h"

// ESP_EVENT_DEFINE_BASE(READ_TEMP_EVENTS);

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

static void read_temp_event_handler(void* args, esp_event_base_t event_base, int32_t event_id, void* measurement){
    struct Temp_reading* meas = (struct Temp_reading*) measurement;
    read_temp(meas);
    if(!meas->err){
        ESP_LOGI("Results:", "Humidity: %d, Temp: %d", meas->hum_sig, meas->temp_sig);
    } else {
        ESP_LOGI("ERROR:", "Thermometer error");
    }
}

void setup() {

    setup_shift_register();
    setup_thermometer();
}


void app_main(void) {
    wifi_init_connection();
    setup();

    // esp_event_loop_args_t read_temp_event_loop_args = {
    //     .queue_size = 15,
    //     .task_name = "temp_task",
    //     .task_priority = 10,
    //     .task_stack_size = 5000,
    //     .task_core_id = 1
    // };

    // esp_event_loop_handle_t event_loop_handle;

    // esp_event_loop_create(&read_temp_event_loop_args, &event_loop_handle);

    // ESP_ERROR_CHECK(esp_event_handler_instance_register_with(event_loop_handle, READ_TEMP_EVENTS, 10, read_temp_event_handler, event_loop_handle, NULL));
    

    struct Temp_reading *measurement = malloc(sizeof(struct Temp_reading));
    xTaskCreatePinnedToCore(temp_task, "Temp reading task", 4000, (void *) measurement, 10, NULL, 1);

    clear_shift_register();

    // int i = 0;
    while(1) {
        // esp_event_post_to(event_loop_handle, READ_TEMP_EVENTS, 10, (void *) measurement, sizeof(struct Temp_reading), portMAX_DELAY);
        vTaskDelay(100);
        display_on_seven_seg(measurement->temp_sig);
        vTaskDelay(100);
        display_on_seven_seg(measurement->hum_sig);
        // vTaskDelay(200);
        // read_temp(measurement);
        // if(!measurement->err){
        //     ESP_LOGI("Results:", "Humidity: %d, Temp: %d", measurement->hum_sig, measurement->temp_sig);
        // } else {
        //     ESP_LOGI("ERROR:", "Thermometer error");
        // }
        // display_on_seven_seg(i % 100);
        // i++;
    }
}