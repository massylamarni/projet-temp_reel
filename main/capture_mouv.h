#ifndef MOUVMENT_H
#define MOUVMENT_H
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "driver/gpio.h"
extern  int mouvement ;
void ini_mouvment(void);
int Detecteur_mouvement(void);
void capture_mouvement(void *pvParameter);
void simulate_mouvement(void *pvParameter);
#endif