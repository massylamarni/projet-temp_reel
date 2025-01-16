#include "capture_gaz.h"
#include "capture_temp.h"
#include "capture_mouv.h"
#include <stdio.h>
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


#define BIT_CONNECTED BIT0
#define BIT_DISCONNECTED BIT1

extern void capture_rfid(void);
extern void simulate_rfid(void *pvParameters);

static const char *TAG = "MAIN" ;


ip_event_got_ip_t *id_reseau;

 EventGroupHandle_t wifi_event_group;

// Fonction à appeler lors de l'enregistrement des events wifi


static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) { //s'il recoit un event de type wifi:
        if (event_id == WIFI_EVENT_STA_START) { //id de l'event=== type de l'event
            esp_wifi_connect();
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            printf("Wi-Fi déconnecté. Tentative de reconnexion...\n");
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, BIT_CONNECTED);
            xEventGroupSetBits(wifi_event_group, BIT_DISCONNECTED);
        }
    } else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            printf("Wi-Fi connecté. appelons tache_cliente()...\n");
            id_reseau=(ip_event_got_ip_t *)event_data;
            xEventGroupSetBits(wifi_event_group, BIT_CONNECTED);
            xEventGroupClearBits(wifi_event_group, BIT_DISCONNECTED);
        }
    }else if(event_id == WIFI_EVENT_STA_CONNECTED){
        printf("Connecté\n");
    }
}

// Tâche cliente réseau
void tache_cliente(void *pvParameters) {
    // Attendre que l'ESP32 soit connecté
    xEventGroupWaitBits(wifi_event_group, BIT_CONNECTED, false, true, portMAX_DELAY);

    printf("Tâche cliente démarrée (ESP32 connecté)\n");


    printf("Adresse IP obtenue : " IPSTR, IP2STR(&id_reseau->ip_info.ip));
    printf("\n");
    printf("Masque de sous-réseau : " IPSTR, IP2STR(&id_reseau->ip_info.netmask));
    printf("\n");
    printf("Passerelle : " IPSTR, IP2STR(&id_reseau->ip_info.gw));
    printf("\n");
    int nom=0;
    while (1) {
        
        EventBits_t bits = xEventGroupGetBits(wifi_event_group);

        if ((bits & BIT_CONNECTED)) {
         printf("ESP32 toujours connecté. Exécution de la tâche cliente...\n");

        } 
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// Initialisation du Wi-Fi
void wifi_init_sta() {
    wifi_event_group = xEventGroupCreate();

    // Initialiser NVS
    //tcpip_adapter_init();
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());

    


    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Créer l'interface Wi-Fi
    esp_netif_create_default_wifi_sta();


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Enregistrement des gestionnaires d'événements
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "DJAWEB",
            .password = "12345678",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    
    // Set Google DNS (8.8.8.8)
esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
esp_netif_dns_info_t dns_info = {
    .ip = {
        .type = IPADDR_TYPE_V4,
        .u_addr.ip4.addr = PP_HTONL(LWIP_MAKEU32(8, 8, 8, 8)) // Google DNS
    }
};
ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns_info));

    

    printf("Wi-Fi initialisé. Connexion à SSID : Test\n");
}

void app_main(void) {

    wifi_init_sta();
    xTaskCreate(tache_cliente, "tache_cliente", 4096, NULL, 5, NULL);

    if (1) {
        xTaskCreate(simulate_temp, "simulate_temp", 4096, NULL, 1, NULL);
        xTaskCreate(simulate_gaz, "simulate_gaz", 4096, NULL, 1, NULL);
        xTaskCreate(simulate_mouvement, "simulate_mouvement", 4096, NULL, 1, NULL);
        xTaskCreate(simulate_rfid, "simulate_rfid", 4096, NULL, 1, NULL);
    } else {
        xTaskCreate(capture_temp, "capture_temp", 4096, NULL, 1, NULL);
        xTaskCreate(capture_gaz, "capture_gaz", 4096, NULL, 1, NULL);
        xTaskCreate(capture_mouvement, "capture_mouvement", 4096, NULL, 1, NULL);
        capture_rfid();
    }
}