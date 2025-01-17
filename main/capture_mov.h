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
#include "esp_random.h"

void movement_sensor_init(void);
int get_movement(void);
int simulate_movement(void);

#endif