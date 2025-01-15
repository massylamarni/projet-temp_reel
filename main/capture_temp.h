#ifndef CAPTURE_TEMP_H
#define CAPTURE_TEMP_H

#include "driver/adc.h"

// prototype des fonction
void temperature_sensor_init(void);
float get_temperature(void);
void capture_temp(void *pvParameter);
void simulate_temp(void *pvParameter);

#endif // TEMPERATURE_SENSOR_H
//////////////////////