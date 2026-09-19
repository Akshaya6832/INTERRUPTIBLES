#include <string.h>
#include "validator_state.h"

static validator_sensor_state_t states[TRINETRA_MAX_SENSOR_STATES];

void validator_state_init(void) { memset(states, 0, sizeof(states)); }

validator_sensor_state_t *validator_state_find(uint16_t node_id, uint16_t sensor_id)
{
    unsigned i;
    for (i = 0; i < TRINETRA_MAX_SENSOR_STATES; ++i)
        if (states[i].in_use && states[i].node_id == node_id && states[i].sensor_id == sensor_id)
            return &states[i];
    return NULL;
}

validator_sensor_state_t *validator_state_get_or_create(uint16_t node_id, uint16_t sensor_id, uint32_t sensor_type)
{
    validator_sensor_state_t *s = validator_state_find(node_id, sensor_id);
    unsigned i;
    if (s != NULL) return s;
    for (i = 0; i < TRINETRA_MAX_SENSOR_STATES; ++i) {
        if (!states[i].in_use) {
            memset(&states[i], 0, sizeof(states[i]));
            states[i].in_use = 1;
            states[i].node_id = node_id;
            states[i].sensor_id = sensor_id;
            states[i].sensor_type = sensor_type;
            return &states[i];
        }
    }
    return NULL;
}
