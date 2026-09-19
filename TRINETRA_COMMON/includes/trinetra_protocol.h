#ifndef TRINETRA_PROTOCOL_H
#define TRINETRA_PROTOCOL_H

#include "trinetra_types.h"

/* Fixed IPC envelope. Payload is one of the protocol structures below. */
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t header_size;
    uint32_t message_type;
    uint32_t source_process;
    uint32_t destination_process;
    uint32_t sequence_number;
    uint64_t timestamp_ms;
    uint32_t payload_length;
    uint32_t reserved;
} trinetra_message_header_t;

typedef struct {
    trinetra_message_header_t header;
    union {
        trinetra_sensor_telemetry_t sensor;
        trinetra_validated_telemetry_t validated;
        trinetra_validation_result_t validation_result;
        trinetra_fused_hazard_state_t hazard;
        trinetra_warning_state_t warning;
        trinetra_alert_command_t alert;
    } payload;
} trinetra_message_t;

typedef struct {
    int32_t status;
    uint32_t reserved;
} trinetra_reply_t;

#endif
