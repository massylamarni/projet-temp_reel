#include "capture_mov.h"

void movement_sensor_init(void) {
      // Configurer les GPIO
    gpio_set_direction(GPIO_NUM_26, GPIO_MODE_INPUT);  // GPIO 21 pour le capteur PIR en entrée
    gpio_set_pull_mode(GPIO_NUM_26, GPIO_PULLDOWN_ONLY);  // Résistance de tirage vers le bas

}

int get_movement(void){
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

int simulate_movement(void) {
    return esp_random() % 2;
}

