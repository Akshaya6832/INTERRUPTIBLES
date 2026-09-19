#ifndef VALIDATOR_STATE_H
#define VALIDATOR_STATE_H
#include <stdint.h>
#include "trinetra_types.h"

typedef struct {
    uint8_t in_use;
    uint16_t node_id;
    uint16_t sensor_id;
    uint32_t sensor_type;
    uint32_t last_sequence;
    uint64_t last_timestamp_ms;
    float last_value;
} validator_sensor_state_t;

void validator_state_init(void);
validator_sensor_state_t *validator_state_find(uint16_t node_id, uint16_t sensor_id);
validator_sensor_state_t *validator_state_get_or_create(uint16_t node_id, uint16_t sensor_id, uint32_t sensor_type);

#endif
