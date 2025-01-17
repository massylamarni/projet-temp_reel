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


static const char *TAG = "HTTP_POST" ;
// Fonction pour envoyer une requête HTTP POST
void send_http_post(char *route, char *json_data) {
    char url[256];

    // Crée la chaîne JSON formatée
    snprintf(url, sizeof(url), "https://node-base-86c5.onrender.com%s", route);
    
    printf("%s {\"data\":%s}\n", url, json_data);

    esp_http_client_config_t config = {
        .url = url,
        .username = "user",
        .password = "passwd",
        .auth_type = HTTP_AUTH_TYPE_NONE,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_data, strlen(json_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST envoyé, code réponse = %d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "Erreur lors de l'envoi du POST : %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    
}