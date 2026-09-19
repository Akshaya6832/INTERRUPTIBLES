#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#include "trinetra_ipc.h"

uint64_t trinetra_now_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        return 0ULL;
    }
    return ((uint64_t)ts.tv_sec * 1000ULL) + ((uint64_t)ts.tv_nsec / 1000000ULL);
}

void trinetra_fill_header(trinetra_message_t *message,
                          uint32_t type,
                          uint32_t source,
                          uint32_t destination,
                          uint32_t sequence,
                          uint32_t payload_length)
{
    memset(message, 0, sizeof(*message));
    message->header.magic = TRINETRA_MAGIC;
    message->header.version = TRINETRA_PROTOCOL_VERSION;
    message->header.header_size = (uint16_t)sizeof(trinetra_message_header_t);
    message->header.message_type = type;
    message->header.source_process = source;
    message->header.destination_process = destination;
    message->header.sequence_number = sequence;
    message->header.timestamp_ms = trinetra_now_ms();
    message->header.payload_length = payload_length;
}

int trinetra_open_service(const char *name)
{
    int coid = name_open(name, 0);
    if (coid == -1) {
        return -1;
    }
    return coid;
}

int trinetra_send(int coid, const trinetra_message_t *message, trinetra_reply_t *reply)
{
    if (coid == -1 || message == NULL) {
        errno = EINVAL;
        return -1;
    }
    trinetra_reply_t local_reply;
    if (reply == NULL) {
        reply = &local_reply;
    }
    int rc = MsgSend(coid, message,
                     (int)(sizeof(message->header) + message->header.payload_length),
                     reply, sizeof(*reply));
    if (rc == -1) {
        return -1;
    }
    return reply->status;
}

const char *trinetra_process_name(uint32_t id)
{
    switch (id) {
        case TRINETRA_PROC_SENSOR_SIM: return "SENSOR_SIM";
        case TRINETRA_PROC_SENSOR_INGEST: return "SENSOR_INGEST";
        case TRINETRA_PROC_VALIDATOR: return "VALIDATOR";
        case TRINETRA_PROC_HAZARD_FUSION: return "HAZARD_FUSION";
        case TRINETRA_PROC_WARNING_GOVERNOR: return "WARNING_GOVERNOR";
        case TRINETRA_PROC_ALERT: return "ALERT";
        default: return "UNKNOWN";
    }
}

const char *trinetra_state_name(trinetra_process_state_t state)
{
    switch (state) {
        case TRINETRA_STATE_STARTING: return "STARTING";
        case TRINETRA_STATE_RUNNING: return "RUNNING";
        case TRINETRA_STATE_DEGRADED: return "DEGRADED";
        case TRINETRA_STATE_ERROR: return "ERROR";
        case TRINETRA_STATE_STOPPING: return "STOPPING";
        default: return "UNKNOWN";
    }
}

const char *trinetra_message_name(uint32_t type)
{
    switch (type) {
        case TRINETRA_MSG_SENSOR_TELEMETRY: return "SENSOR_TELEMETRY";
        case TRINETRA_MSG_VALIDATED_TELEMETRY: return "VALIDATED_TELEMETRY";
        case TRINETRA_MSG_VALIDATION_RESULT: return "VALIDATION_RESULT";
        case TRINETRA_MSG_FUSED_HAZARD_STATE: return "FUSED_HAZARD_STATE";
        case TRINETRA_MSG_WARNING_STATE: return "WARNING_STATE";
        case TRINETRA_MSG_ALERT_COMMAND: return "ALERT_COMMAND";
        case TRINETRA_MSG_HEALTH_STATUS: return "HEALTH_STATUS";
        case TRINETRA_MSG_CONTROL_COMMAND: return "CONTROL_COMMAND";
        default: return "INVALID";
    }
}
