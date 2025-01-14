#include "capture_gaz.h"
#define ADC_CHANNEL ADC1_CHANNEL_6 // GPIO 34

void init_cap_gaz(void){
  // Configuration de l'ADC : résolution 12 bits, plage 0 à 5V
    adc1_config_width(ADC_WIDTH_BIT_12);                     // 12 bits (0-4095)
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11); // 0 ~3.3V
}
// Fonction pour détecter le taux de gaz
float Detecteur_gaz(void)
{
    // Lecture de l'ADC
    int adc_reading = adc1_get_raw(ADC_CHANNEL);

    // Conversion de la lecture ADC en tension (en volts)
    float volt = (adc_reading / 4095.0) * 3.3; // Conversion en volts

    // Calcul du taux de gaz basé sur la tension lue
    float taux = (volt / 3.3) * 1000; // Simple calcul de taux (en pourcentage)

    // Affichage des résultats
    //printf("Lecture ADC: %d, Tension: %.2f V, Taux de gaz: %.2f%%\n", adc_reading, volt, taux);

    // Retourner le taux de gaz
    return taux;
}
