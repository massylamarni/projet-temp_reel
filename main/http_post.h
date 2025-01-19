#ifndef HTTP_POST_H
#define HTTP_POST_H

#include <stdio.h>
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

#define TESTING true

void send_http_post(char *route, char *json_data);

void post_rfid_capture(void *arg, esp_event_base_t base, int32_t event_id, void *data);

void post_rfid_simulation(void *pvParameter);

void post_temp_capture(void *pvParameter);

void post_gas_capture(void *pvParameter);

void post_movement_capture(void *pvParameter);

#endif