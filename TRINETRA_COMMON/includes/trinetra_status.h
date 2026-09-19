#ifndef TRINETRA_STATUS_H
#define TRINETRA_STATUS_H

#include <stdint.h>
#include "trinetra_types.h"

typedef struct {
    trinetra_process_id_t process_id;
    trinetra_process_state_t state;
    uint64_t last_activity_ms;
    uint32_t messages_received;
    uint32_t messages_sent;
    uint32_t errors;
} trinetra_runtime_status_t;

#endif
