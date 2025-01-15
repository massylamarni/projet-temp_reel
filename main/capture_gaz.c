#include "capture_gaz.h"
#include "esp_random.h"
#define ADC_CHANNEL ADC1_CHANNEL_6 // GPIO 34

extern void send_http_post(char *route, char *sensor_data);


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

void capture_gaz(void *pvParameter) {   
    char sensor_data[16];
    while (1) {
        snprintf(sensor_data, sizeof(sensor_data), "%.2f", (float) Detecteur_gaz());
        send_http_post("/api/post/gas", sensor_data);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1000 milliseconds (1 second)
    }
}

void simulate_gaz(void *pvParameter) {
    char sensor_data[16];
    while (1) {
        snprintf(sensor_data, sizeof(sensor_data), "%.2f", (float) ((esp_random() % 5) + 1));
        send_http_post("/api/post/gas", sensor_data);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1000 milliseconds (1 second)
    }
}