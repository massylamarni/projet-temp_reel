#ifndef IDIR_WIFI_MODULE_H
#define IDIR_WIFI_MODULE_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "driver/gpio.h"
#include "esp_freertos_hooks.h"
#include "esp_mac.h"
#include "dhcpserver/dhcpserver.h"
#include "esp_http_server.h"

// Variable globale

// Handlers d'événements WiFi et IP
void handler(void* hand, esp_event_base_t event, int32_t id_event, void* event_data);
void tache_scan(void*par);
// Initialisation du WiFi
void init_wifi(void);

#endif // IDIR_WIFI_MODULE_H
