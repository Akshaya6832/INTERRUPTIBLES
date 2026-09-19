#ifndef VALIDATOR_RULES_H
#define VALIDATOR_RULES_H
#include <stddef.h>
#include "trinetra_protocol.h"

int validator_validate(const trinetra_message_t *message, char *reason, size_t reason_size);
void validator_state_commit(const trinetra_sensor_telemetry_t *sensor);

#endif
