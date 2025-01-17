#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "lwip/netdb.h"

//#include "rgb_led.h"
#include "wifi_app.h"
#include "http_server.h"

static const char TAG [] = "wifi_app";

static QueueHandle_t wifi_app_queue_handle;

esp_netif_t* esp_netif_sta = NULL;
esp_netif_t* esp_netif_ap = NULL;

static int wifi_connection_retry_count = 0;
wifi_mode_t wifi_mode = WIFI_MODE_APSTA;

static void wifi_app_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_AP_START:
                ESP_LOGI(TAG, "WIFI_EVENT_AP_START");
                break;
            case WIFI_EVENT_AP_STOP:
                ESP_LOGI(TAG, "WIFI_EVENT_AP_STOP");
                break;
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
                wifi_connection_retry_count = 0;
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
                //Retry connection
                if (wifi_connection_retry_count < MAX_CONNECTION_RETRIES) {
                    ESP_LOGI(TAG, "Reconnecting to Wi-Fi... Attempt %d of %d", wifi_connection_retry_count + 1, MAX_CONNECTION_RETRIES);
                    wifi_connection_retry_count++;
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    esp_wifi_connect();
                } else {
                    ESP_LOGE(TAG, "Failed to connect to Wi-Fi after %d attempts", MAX_CONNECTION_RETRIES);
                }
                break;
            case WIFI_EVENT_AP_STACONNECTED:
                ESP_LOGI(TAG, "WIFI_EVENT_AP_STACONNECTED");
                http_server_start();
                break;
            case WIFI_EVENT_AP_STADISCONNECTED:
                ESP_LOGI(TAG, "WIFI_EVENT_AP_STADISCONNECTED");
                break;
        }
    }
    else if (event_base == IP_EVENT) {
        switch(event_id) {
            case IP_EVENT_STA_GOT_IP:
                ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
                wifi_connection_retry_count = 0;
                break;
        }
    }
}

static void wifi_app_event_handler_init(void) {
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_event_handler_instance_t instance_wifi_event;
    esp_event_handler_instance_t instance_ip_event;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_wifi_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_ip_event));
}

static void wifi_app_default_wifi_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    esp_netif_sta = esp_netif_create_default_wifi_sta();
    esp_netif_ap = esp_netif_create_default_wifi_ap();
}

static void wifi_app_soft_ap_config(void) {
    wifi_config_t ap_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .password = WIFI_AP_PASSWORD,
            .channel = WIFI_AP_CHANNEL,
            .ssid_hidden = WIFI_AP_SSID_HIDDEN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .max_connection = WIFI_AP_MAX_CONNECTIONS,
            .beacon_interval = WIFI_AP_BEACON_INTERVAL,
        },
    };

    //DHCP config
    esp_netif_ip_info_t ap_ip_info;
    memset(&ap_ip_info, 0x00, sizeof(ap_ip_info));
    esp_netif_dhcps_stop(esp_netif_ap);

    //Assing AP static IP, GW & NM
    inet_pton(AF_INET, WIFI_AP_IP, &ap_ip_info.ip);
    inet_pton(AF_INET, WIFI_AP_GATEWAY, &ap_ip_info.gw);
    inet_pton(AF_INET, WIFI_AP_NETMASK, &ap_ip_info.netmask);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_AP, WIFI_AP_BANDWIDTH));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_STA_POWER_SAVE));

}

static void wifi_app_station_config(void) {
    wifi_config_t sta_config = {
        .sta = {
            .ssid = WIFI_STA_SSID,
            .password = WIFI_STA_PASSWORD,
            .bssid_set = false,             // Do not specify a BSSID (allow auto-selection of AP)
            .listen_interval = 3,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));
}

static void wifi_app_apsta_config(void) {
    wifi_config_t ap_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .password = WIFI_AP_PASSWORD,
            .channel = WIFI_AP_CHANNEL,
            .ssid_hidden = WIFI_AP_SSID_HIDDEN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .max_connection = WIFI_AP_MAX_CONNECTIONS,
            .beacon_interval = WIFI_AP_BEACON_INTERVAL,
        },
    };

    wifi_config_t sta_config = {
        .sta = {
            .ssid = WIFI_STA_SSID,
            .password = WIFI_STA_PASSWORD,
            .bssid_set = false,             // Do not specify a BSSID (allow auto-selection of AP)
            .listen_interval = 3,
        },
    };

    //DHCP config
    esp_netif_ip_info_t ap_ip_info;
    memset(&ap_ip_info, 0x00, sizeof(ap_ip_info));
    esp_netif_dhcps_stop(esp_netif_ap);

    //Assing AP static IP, GW & NM
    inet_pton(AF_INET, WIFI_AP_IP, &ap_ip_info.ip);
    inet_pton(AF_INET, WIFI_AP_GATEWAY, &ap_ip_info.gw);
    inet_pton(AF_INET, WIFI_AP_NETMASK, &ap_ip_info.netmask);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_AP, WIFI_AP_BANDWIDTH));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_STA_POWER_SAVE));
}


static void wifi_app_task(void *pvParameters) {
    ESP_LOGI(TAG, "WIFI TASK STARTED");
    wifi_app_queue_message_t msg;

    wifi_app_event_handler_init();

    //TCP/IP stack config
    wifi_app_default_wifi_init();

    //SoftAP config
    if (wifi_mode == WIFI_MODE_AP) wifi_app_soft_ap_config();

    //Station config
    if (wifi_mode == WIFI_MODE_STA) wifi_app_station_config();

    //APSta config
    if (wifi_mode == WIFI_MODE_APSTA) wifi_app_apsta_config();

    ESP_ERROR_CHECK(esp_wifi_start());
    if (wifi_mode == WIFI_MODE_STA || wifi_mode == WIFI_MODE_APSTA) ESP_ERROR_CHECK(esp_wifi_connect());
    ESP_LOGI(TAG, "Connecting to Wi-Fi AP: %s", "ESP_32_EXT");
    wifi_app_send_message(WIFI_APP_MSG_START_HTTP_SERVER);

    for(;;) {
        if (xQueueReceive(wifi_app_queue_handle, &msg, portMAX_DELAY)) {
            switch(msg.msgID) {
                case WIFI_APP_MSG_START_HTTP_SERVER:
                    ESP_LOGI(TAG, "WIFI_APP_MSG_START_HTTP_SERVER");
                    http_server_start();
                    break;
                case WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER:
                    ESP_LOGI(TAG, "WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER");
                    break;
                case WIFI_APP_MSG_STA_CONNECTED_GOT_IP:
                    ESP_LOGI(TAG, "WIFI_APP_MSG_STA_CONNECTED_GOT_IP");
                    //rgb_led_wifi_connected();
                    break;
                default:
                    break;
            }
        }
    }
}

BaseType_t wifi_app_send_message(wifi_app_message_e msgID) {
    wifi_app_queue_message_t msg;
    msg.msgID = msgID;
    return xQueueSend(wifi_app_queue_handle, &msg, portMAX_DELAY);
}

void wifi_app_start(void) {
    ESP_LOGI(TAG, "STARTING WIFI APPLICATION");

    //rgb_led_wifi_app_started();
    //esp_log_level_set("wifi", ESP_LOG_NONE);

    wifi_app_queue_handle = xQueueCreate(3, sizeof(wifi_app_queue_message_t));

    xTaskCreatePinnedToCore(&wifi_app_task, "wifi_app_task", WIFI_APP_TASK_STACK_SIZE, NULL, WIFI_APP_TASK_PRIORITY, NULL, WIFI_APP_TASK_CORE_ID);
}