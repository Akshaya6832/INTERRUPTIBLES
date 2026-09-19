#ifndef SENSOR_HAL_H
#define SENSOR_HAL_H

#include "trinetra_types.h"

int sensor_hal_init(void);
int sensor_hal_read(trinetra_sensor_telemetry_t *sample);
void sensor_hal_shutdown(void);

#endif
