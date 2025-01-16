#include "capture_mouv.h"
#include "esp_random.h"

extern void send_http_post(char *route, char *sensor_data);

void ini_mouvment(void){
      // Configurer les GPIO
    gpio_set_direction(GPIO_NUM_26, GPIO_MODE_INPUT);  // GPIO 21 pour le capteur PIR en entrée
    gpio_set_pull_mode(GPIO_NUM_26, GPIO_PULLDOWN_ONLY);  // Résistance de tirage vers le bas

}

int Detecteur_mouvement(void){
        // Vérifier l'état du capteur PIR
        int mouvement = gpio_get_level(GPIO_NUM_26);
        printf("PIR State: %d\n", mouvement);  // Afficher l'état du capteur
        if (mouvement == 1) {
            // Mouvement détecté - allumer la LED et activer le buzzer
            printf("Mouvement détecté !\n");
        } else {
            // Pas de mouvement - éteindre la LED et désactiver le buzzer
          
            printf("Pas de mouvement \n");
        }
        return mouvement;
}

void capture_mouvement(void *pvParameter) {
    char sensor_data[16];
    while (1) {
        snprintf(sensor_data, sizeof(sensor_data), "%.2f", (float) Detecteur_mouvement());
        send_http_post("/api/post/movement", sensor_data);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1000 milliseconds (1 second)
    }
}

void simulate_mouvement(void *pvParameter) {
    char sensor_data[16];
    while (1) {
        snprintf(sensor_data, sizeof(sensor_data), "%.2f", (float) ((esp_random() % 2)));
        send_http_post("/api/post/movement", sensor_data);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1000 milliseconds (1 second)
    }
}

