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

ESP_EVENT_DEFINE_BASE(READ_TEMP_EVENTS);

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
    // wifi_init_connection();
    setup();

    esp_event_loop_args_t read_temp_event_loop_args = {
        .queue_size = 15,
        .task_name = "temp_task",
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 5000,
        .task_core_id = tskNO_AFFINITY
    };

    esp_event_loop_handle_t event_loop_handle;

    esp_event_loop_create(&read_temp_event_loop_args, &event_loop_handle);

    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(event_loop_handle, READ_TEMP_EVENTS, 10, read_temp_event_handler, event_loop_handle, NULL));
    
    struct Temp_reading measurement = { 0 , 0 , 0 , 0, 0 };

    // xTaskCreate(temp_task, "Temp reading task", 5000, (void *) &measurement, 20, NULL);

    clear_shift_register();

    uint8_t i = 0;
    while(1) {
        vTaskDelay(500);
        display_on_seven_seg(i);

        esp_event_post_to(event_loop_handle, READ_TEMP_EVENTS, 10, &measurement, sizeof(measurement), portMAX_DELAY);

        // display_on_seven_seg(measurement.temp_sig);
        // vTaskDelay(500);
        // display_on_seven_seg(measurement.hum_sig);
        i++;
        if(i > 100){
            i = 0;
        }
    }
}