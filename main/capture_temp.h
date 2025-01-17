#ifndef CAPTURE_TEMP_H
#define CAPTURE_TEMP_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_random.h"
#include <stdio.h>

#define NUM_SAMPLES 20  // Nombre de lectures pour la moyenne
// Définir la broche ADC et les paramètres
#define ADC_CHANNEL ADC1_CHANNEL_5 // Canal ADC utilisé (GPIO 33)
#define ADC_WIDTH ADC_WIDTH_BIT_12    // Résolution ADC : 12 bits (valeurs de 0 à 4095)
#define ADC_ATTEN ADC_ATTEN_DB_11     // Atténuation : plage jusqu'à 3,3 V

// prototype des fonction
void temperature_sensor_init(void);
float get_temp(void);
float simulate_temp(void);

#endif // TEMPERATURE_SENSOR_H
//////////////////////