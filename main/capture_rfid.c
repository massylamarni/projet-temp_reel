#include "capture_rfid.h"

static const char *TAG = "rc522-read-uid";//tag of rc522

rfid_data_t *rfid_data;

static void on_picc_state_changed(void *arg, esp_event_base_t base, int32_t event_id, void *data)
{
    rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
    rc522_picc_t *picc = event->picc;

    if (picc->state != RC522_PICC_STATE_ACTIVE) {
        return;
    }

    // Afficher l'UID de la carte détectée
    rc522_picc_uid_to_str(&picc->uid, rfid_data->uid, sizeof(rfid_data->uid));
    ESP_LOGI(TAG, "Card detected: %s", rfid_data->uid);

    // Comparer l'UID détecté avec l'UID attendu
    if (strcmp(rfid_data->uid, EXPECTED_UID) == 0) {
        ESP_LOGI(TAG, "Access granted. Door opened.");
        rfid_data->is_valid = true;  // Modifier l'état de la porte (ouverte)
    } else {
        ESP_LOGI(TAG, "Access denied. Door closed.");
        rfid_data->is_valid = false;   // Modifier l'état de la porte (fermée)
    }
}

void start_rfid(void) {
    rfid_data = malloc(sizeof(rfid_data_t));
    rc522_spi_create(&driver_config, &driver);
    rc522_driver_install(driver);

    rc522_config_t scanner_config = {
        .driver = driver,
    };
    // Initialiser le scanner RFID
    rc522_create(&scanner_config, &scanner);
    //rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED, on_picc_state_changed, NULL);
    rc522_start(scanner);
    ESP_LOGI(TAG, "RC522 scanner started. Waiting for cards..."); 
}

void capture_rfid(void (*task_pointer)(void *arg, esp_event_base_t base, int32_t event_id, void *data)) {
    rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED, task_pointer, NULL);
}

rfid_data_t* simulate_rfid(void) {
    const char *hex_chars = "0123456789ABCDEF";

    char uid[13];  // 12 characters for the UID + null terminator

    for (size_t i = 0; i < sizeof(uid) - 1; i++) {
        uid[i] = hex_chars[esp_random() % 16];  // Random hex character
    }
    uid[sizeof(uid) - 1] = '\0';  // Null-terminate the string

    strncpy(rfid_data->uid, uid, sizeof(rfid_data->uid) - 1);
    rfid_data->uid[sizeof(rfid_data->uid) - 1] = '\0'; 
    rfid_data->is_valid = (esp_random() % 2) ? "1" : 0;

    return rfid_data;
}