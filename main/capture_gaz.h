#ifndef CAPTURE_GAZ_H
#define CAPTURE_GAZ_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_random.h"

#define ADC_CHANNEL ADC1_CHANNEL_6 // GPIO 34

void gas_sensor_init(void);
float get_gas(void);
float simulate_gaz(void);

#endif