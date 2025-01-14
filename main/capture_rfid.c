#include <esp_log.h>
#include <string.h>
#include <time.h>
#include <rc522.h>
#include "driver/rc522_spi.h"
#include "picc/rc522_mifare.h"
#include "driver/gpio.h"
static const char *TAG = "rc522-read-uid";//tag of rc522
//gpio_pulldown_en(2);
//part rfid code /////////////////////////
// Configuration des broches SPI pour le RC522
#define RC522_SPI_BUS_GPIO_MISO    (19)
#define RC522_SPI_BUS_GPIO_MOSI    (23)
#define RC522_SPI_BUS_GPIO_SCLK    (18)
#define RC522_SPI_SCANNER_GPIO_SDA (5)
#define RC522_SCANNER_GPIO_RST     (-1) // soft-reset

bool doorState;
char uid_str[RC522_PICC_UID_STR_BUFFER_SIZE_MAX];

// UID prédéfini pour la comparaison (exemple : "12345678")
#define EXPECTED_UID "A2 B8 E8 A9" //ajouter l'un des uid pour simuler l'overture de la parte 
static rc522_spi_config_t driver_config = {
    .host_id = SPI3_HOST,
    .bus_config = &(spi_bus_config_t){
        .miso_io_num = RC522_SPI_BUS_GPIO_MISO,
        .mosi_io_num = RC522_SPI_BUS_GPIO_MOSI,
        .sclk_io_num = RC522_SPI_BUS_GPIO_SCLK,
    },
    .dev_config = {
        .spics_io_num = RC522_SPI_SCANNER_GPIO_SDA,
    },
    .rst_io_num = RC522_SCANNER_GPIO_RST,
};

static rc522_driver_handle_t driver;
static rc522_handle_t scanner;
// Fonction appelée lors du changement d'état de la carte
static void on_picc_state_changed(void *arg, esp_event_base_t base, int32_t event_id, void *data)
{
    rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
    rc522_picc_t *picc = event->picc;

    if (picc->state != RC522_PICC_STATE_ACTIVE) {
        return;
    }

    // Afficher l'UID de la carte détectée
    rc522_picc_uid_to_str(&picc->uid, uid_str, sizeof(uid_str));
    ESP_LOGI(TAG, "Card detected: %s", uid_str);

    // Comparer l'UID détecté avec l'UID attendu
    if (strcmp(uid_str, EXPECTED_UID) == 0) {
        ESP_LOGI(TAG, "Access granted. Door opened.");
        doorState = true;  // Modifier l'état de la porte (ouverte)
    } else {
        ESP_LOGI(TAG, "Access denied. Door closed.");
        doorState = false;   // Modifier l'état de la porte (fermée)
    }
}

static void capture_rfid() {
    rc522_spi_create(&driver_config, &driver);
    rc522_driver_install(driver);

    rc522_config_t scanner_config = {
        .driver = driver,
    };
    // Initialiser le scanner RFID
    rc522_create(&scanner_config, &scanner);
    rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED,on_picc_state_changed, NULL);
    rc522_start(scanner);
    ESP_LOGI(TAG, "RC522 scanner started. Waiting for cards..."); 
}