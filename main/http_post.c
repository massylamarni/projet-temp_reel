#include "http_post.h"

static const char *TAG = "HTTP_POST" ;

// Fonction pour envoyer une requête HTTP POST
void send_http_post(char *route, char *json_data) {
    char url[256];

    // Crée la chaîne JSON formatée
    snprintf(url, sizeof(url), "https://node-base-86c5.onrender.com%s", route);
    
    printf("%s JSON: %s\n", url, json_data);

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

void post_rfid_capture(void *arg, esp_event_base_t base, int32_t event_id, void *data) {
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

void post_rfid_simulation(void *pvParameter) {
    rfid_data_t* rfid_data = malloc(sizeof(rfid_data_t));
    char json_data[128];
    while (1) {
        rfid_data = get_random_rfid();
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
