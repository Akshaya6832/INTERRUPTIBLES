#include <errno.h>
#include "sensor_hal.h"

int sensor_hal_init(void) { return 0; }

/* Production adapter hook. Real UART/SPI/GPIO driver integration belongs here. */
int sensor_hal_read(trinetra_sensor_telemetry_t *sample)
{
    (void)sample;
    errno = ENOSYS;
    return -1;
}

void sensor_hal_shutdown(void) { }
