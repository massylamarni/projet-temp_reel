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

#include "capture_gaz.h"
#include "capture_temp.h"
#include "capture_mov.h"
#include "capture_rfid.h"
#include "wifi_app.h"

#define TESTING true

extern void send_http_post(char *route, char *sensor_data);

static const char *TAG = "MAIN" ;

void post_rfid_capture(void *arg, esp_event_base_t base, int32_t event_id, void *data) {
    if (data == NULL) {
        void (*post_rfid_pointer)(void *arg, esp_event_base_t base, int32_t event_id, void *data) = post_rfid_capture;
        capture_rfid(post_rfid_pointer);
    } else {
        rfid_data_t* rfid_data = malloc(sizeof(rfid_data_t));

        rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
        rc522_picc_t *picc = event->picc;

        if (picc->state != RC522_PICC_STATE_ACTIVE) {
            return;
        }

        rc522_picc_uid_to_str(&picc->uid, rfid_data->uid, sizeof(rfid_data->uid));

        // Comparer l'UID détecté avec l'UID attendu
        if (strcmp(rfid_data->uid, EXPECTED_UID) == 0) {
            rfid_data->is_valid = true;
        } else {
            rfid_data->is_valid = false;
        }
        
        char json_data[128];    
        snprintf(json_data, sizeof(json_data), "{\"data\":{\"uid\":\"%s\", \"is_valid\":\"%s\"}}", rfid_data->uid, rfid_data->is_valid ? "1" : "0");
        send_http_post("/api/post/rfid", json_data);
    }
}
void post_rfid_simulation(void *pvParameter) {
    rfid_data_t* rfid_data = malloc(sizeof(rfid_data_t));
    char json_data[128];
    while (1) {
        rfid_data = simulate_rfid();
        snprintf(json_data, sizeof(json_data), "{\"data\":{\"uid\":\"%s\", \"is_valid\":\"%s\"}}", rfid_data->uid, rfid_data->is_valid ? "1" : "0");

        send_http_post("/api/post/rfid", json_data);
        vTaskDelay(pdMS_TO_TICKS((esp_random() % 15001) + 5000));
    }
}

void post_temp_capture(void *pvParameter) {
    char sensor_data[16];
    char json_data[128];
    while (1) {
        snprintf(sensor_data, sizeof(sensor_data), "%.2f", TESTING ? simulate_temp() : get_temp());
        snprintf(json_data, sizeof(json_data), "{\"data\":\"%s\"}", sensor_data);
        send_http_post("/api/post/temperature", json_data);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void post_gas_capture(void *pvParameter) {
    char sensor_data[16];
    char json_data[128];
    while (1) {
        snprintf(sensor_data, sizeof(sensor_data), "%.2f", TESTING ? simulate_gaz() : get_gas());
        snprintf(json_data, sizeof(json_data), "{\"data\":\"%s\"}", sensor_data);
        send_http_post("/api/post/gas", json_data);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void post_movement_capture(void *pvParameter) {
    char sensor_data[16];
    char json_data[128];
    while (1) {
        snprintf(sensor_data, sizeof(sensor_data), "%.2f", (float)(TESTING ? simulate_movement() : get_movement()));
        snprintf(json_data, sizeof(json_data), "{\"data\":\"%s\"}", sensor_data);
        send_http_post("/api/post/movement", json_data);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

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
    if (0) {
        start_rfid();
        temperature_sensor_init();
        gas_sensor_init();
        movement_sensor_init();
        post_rfid_capture(NULL, NULL, 0, NULL);
    } else {
        xTaskCreate(post_rfid_simulation, "post_rfid_simulation", 4096, NULL, 1, NULL);
    }
    xTaskCreate(post_temp_capture, "post_temp_capture", 4096, NULL, 1, NULL);
    xTaskCreate(post_gas_capture, "post_gas_capture", 4096, NULL, 1, NULL);
    xTaskCreate(post_movement_capture, "post_movement_capture", 4096, NULL, 1, NULL);
}