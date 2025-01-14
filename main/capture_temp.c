#include "capture_temp.h"
#include <stdio.h>
#include "idir_wifi.h"
#define NUM_SAMPLES 20  // Nombre de lectures pour la moyenne
// Définir la broche ADC et les paramètres
#define ADC_CHANNEL ADC1_CHANNEL_5 // Canal ADC utilisé (GPIO 33)
#define ADC_WIDTH ADC_WIDTH_BIT_12    // Résolution ADC : 12 bits (valeurs de 0 à 4095)
#define ADC_ATTEN ADC_ATTEN_DB_11     // Atténuation : plage jusqu'à 3,3 V

// Fonction pour initialiser le capteur
void temperature_sensor_init(void) {
    // Configurer la largeur de l'ADC (résolution)
    adc1_config_width(ADC_WIDTH);
    // Configurer le canal ADC avec l'atténuation
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);
}
// Fonction pour obtenir la température
float get_temperature(void) {
    float voltage;
    float  somme=0;//pour faire la maoiyenne 
    // Lire la valeur brute de l'ADC
    for(int i=0;i<5;i++){
    int raw_adc = adc1_get_raw(ADC_CHANNEL);
    // Converter la valeur brute en tension (millivolts)
    voltage =((float)raw_adc/ 4095.0) *3300.0;
    somme+=voltage;
    for(int i=0;i<100;i++);//delais entre les lecteur de température 
    }
    float moy=somme/4;
    // Calculer la température (exemple : Température = Voltage / 10)
    float temperature = moy/10.0  ;//10mv pac degré
    return temperature;
}

/*
float get_temperature(void) {
    float voltage = 0;
    float somme = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        int raw_adc = adc1_get_raw(ADC_CHANNEL);
        voltage = ((float)raw_adc / 4095.0) * 3300.0; // Convertir en mV
        somme += voltage;
        vTaskDelay(pdMS_TO_TICKS(10)); // Délai de 10ms entre les lectures
    }

    float moy = somme / NUM_SAMPLES;
    float temperature = moy / 10.0; // 10 mV par degré
    return temperature;
}
*/