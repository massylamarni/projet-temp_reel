#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "driver/gpio.h"
#include "lwip/sys.h"
#include "lwip/err.h"
#include "string.h"
#include "esp_http_client.h"
#include "lwip/ip4_addr.h"

#include "wifi_app.h"
#include "http_post.h"

extern void send_http_post(char *route, char *sensor_data);

static const char *TAG = "MAIN" ;

void app_main(void) {
    //Init NVS (Non Volatile Storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_app_start();
    //TODO free rfid_data_t* rfid_data = malloc(sizeof(rfid_data_t));
    /*
    if (0) {
        temperature_sensor_init();
        gas_sensor_init();
        movement_sensor_init();
        start_rfid();
        capture_rfid(post_rfid_capture);
        post_rfid_capture(NULL, NULL, 0, NULL);
    } else {
        xTaskCreate(post_rfid_simulation, "post_rfid_simulation", 4096, NULL, 1, NULL);
    }
    xTaskCreate(post_temp_capture, "post_temp_capture", 4096, NULL, 1, NULL);
    xTaskCreate(post_gas_capture, "post_gas_capture", 4096, NULL, 1, NULL);
    xTaskCreate(post_movement_capture, "post_movement_capture", 4096, NULL, 1, NULL);
    */
}