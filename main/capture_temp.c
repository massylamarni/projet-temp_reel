#include "capture_temp.h"

// Fonction pour initialiser le capteur
void temperature_sensor_init(void) {
    // Configurer la largeur de l'ADC (résolution)
    adc1_config_width(ADC_WIDTH);
    // Configurer le canal ADC avec l'atténuation
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);
}

// Fonction pour obtenir la température
float get_temp(void) {
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

float simulate_temp(void) {
    return (float)(esp_random() % 30) + 10;
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