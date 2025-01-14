#ifndef CAPTURE_GAZ_H
#define CAPTURE_GAZ_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_log.h"
void init_cap_gaz(void);
float Detecteur_gaz(void);

#endif