#ifndef CAPTURE_RFID_H
#define CAPTURE_RFID_H

#include <esp_log.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <rc522.h>
#include "driver/rc522_spi.h"
#include "picc/rc522_mifare.h"
#include "driver/gpio.h"
#include "esp_random.h"

#define RC522_SPI_BUS_GPIO_MISO    (19)
#define RC522_SPI_BUS_GPIO_MOSI    (23)
#define RC522_SPI_BUS_GPIO_SCLK    (18)
#define RC522_SPI_SCANNER_GPIO_SDA (5)
#define RC522_SCANNER_GPIO_RST     (-1) // soft-reset

typedef struct {
    char uid[RC522_PICC_UID_STR_BUFFER_SIZE_MAX];
    bool is_valid;
} rfid_data_t;

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
static void on_picc_state_changed(void *arg, esp_event_base_t base, int32_t event_id, void *data);

void start_rfid(void);

void capture_rfid(void (*function_pointer)(void *arg, esp_event_base_t base, int32_t event_id, void *data));

rfid_data_t* get_random_rfid(void);

#endif