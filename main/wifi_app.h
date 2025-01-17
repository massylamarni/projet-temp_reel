#ifndef MAIN_WIFI_APP_H_
#define MAIN_WIFI_APP_H_

#include "esp_netif.h"

#define WIFI_AP_SSID                "ESP32_AP"
#define WIFI_AP_PASSWORD            "PASSWORD"
#define WIFI_AP_CHANNEL             1
#define WIFI_AP_SSID_HIDDEN         0
#define WIFI_AP_MAX_CONNECTIONS     5
#define WIFI_AP_BEACON_INTERVAL     100
#define WIFI_AP_IP                  "192.168.0.1"
#define WIFI_AP_GATEWAY             "192.168.0.1"
#define WIFI_AP_NETMASK             "255.255.255.0"
#define WIFI_AP_BANDWIDTH           WIFI_BW_HT20    //20Mhz (40Mhz is an other option)

#define WIFI_STA_POWER_SAVE         WIFI_PS_NONE
#define WIFI_STA_SSID               "ESP32_STA"
#define WIFI_STA_PASSWORD           "PASSWORD"
#define MAX_SSID_LENGTH             32
#define MAX_PASSWORD_LENGTH         64
#define MAX_CONNECTION_RETRIES      50

#define WIFI_APP_TASK_STACK_SIZE    4096
#define WIFI_APP_TASK_PRIORITY      5
#define WIFI_APP_TASK_CORE_ID       0

extern esp_netif_t* esp_netif_sta;
extern esp_netif_t* esp_netif_ap;

typedef enum wifi_app_message {
    WIFI_APP_MSG_START_HTTP_SERVER = 0,
    WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER,
    WIFI_APP_MSG_STA_CONNECTED_GOT_IP,
} wifi_app_message_e;

typedef struct wifi_app_queue_message {
    wifi_app_message_e msgID;
} wifi_app_queue_message_t;

BaseType_t wifi_app_send_message(wifi_app_message_e msgID);

void wifi_app_start(void);
void http_server_start(void);

#endif
