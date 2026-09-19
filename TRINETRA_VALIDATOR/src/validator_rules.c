#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "trinetra_config.h"
#include "validator_rules.h"
#include "validator_state.h"
#include "trinetra_ipc.h"

static int reject(char *reason, size_t n, const char *text)
{
    if (reason != NULL && n > 0) snprintf(reason, n, "%s", text);
    return EINVAL;
}

int validator_validate(const trinetra_message_t *m, char *reason, size_t reason_size)
{
    if (m == NULL) return reject(reason, reason_size, "null message");
    if (m->header.magic != TRINETRA_MAGIC) return reject(reason, reason_size, "bad magic");
    if (m->header.version != TRINETRA_PROTOCOL_VERSION) return reject(reason, reason_size, "bad protocol version");
    if (m->header.header_size != sizeof(trinetra_message_header_t)) return reject(reason, reason_size, "bad header size");
    if (m->header.message_type != TRINETRA_MSG_SENSOR_TELEMETRY) return reject(reason, reason_size, "unexpected message type");
    if (m->header.destination_process != TRINETRA_PROC_VALIDATOR) return reject(reason, reason_size, "wrong destination");
    if (m->header.payload_length != sizeof(m->payload.sensor)) return reject(reason, reason_size, "bad payload length");
    if (m->payload.sensor.node_id == 0) return reject(reason, reason_size, "invalid node id");
    if (m->payload.sensor.sensor_id == 0) return reject(reason, reason_size, "invalid sensor id");
    if (m->payload.sensor.sensor_type < TRINETRA_SENSOR_RAINFALL || m->payload.sensor.sensor_type > TRINETRA_SENSOR_VIBRATION)
        return reject(reason, reason_size, "invalid sensor type");
    if (m->payload.sensor.sequence_number == 0) return reject(reason, reason_size, "invalid sequence");
    if (!isfinite(m->payload.sensor.value) || !isfinite(m->payload.sensor.rate_of_change))
        return reject(reason, reason_size, "non-finite numeric value");

    uint64_t now = trinetra_now_ms();
    uint64_t ts = m->payload.sensor.timestamp_ms;
    if (ts > now + TRINETRA_MAX_FUTURE_TIMESTAMP_MS) return reject(reason, reason_size, "future timestamp");
    if (now >= ts && (now - ts) > TRINETRA_MAX_SENSOR_AGE_MS) return reject(reason, reason_size, "stale timestamp");

    if (m->payload.sensor.sensor_type == TRINETRA_SENSOR_WATER_LEVEL) {
        if (m->payload.sensor.value < TRINETRA_WATER_LEVEL_MIN_M || m->payload.sensor.value > TRINETRA_WATER_LEVEL_MAX_M)
            return reject(reason, reason_size, "water level out of range");
        if (fabsf(m->payload.sensor.rate_of_change) > TRINETRA_WATER_LEVEL_MAX_RATE_MPM)
            return reject(reason, reason_size, "water rate exceeds limit");
    }

    validator_sensor_state_t *prev = validator_state_find(m->payload.sensor.node_id, m->payload.sensor.sensor_id);
    if (prev != NULL) {
        if (m->payload.sensor.sequence_number <= prev->last_sequence)
            return reject(reason, reason_size, "duplicate or out-of-order sequence");
        if (m->payload.sensor.timestamp_ms < prev->last_timestamp_ms)
            return reject(reason, reason_size, "timestamp moved backwards");
        if (m->payload.sensor.sensor_type == TRINETRA_SENSOR_WATER_LEVEL) {
            uint64_t dt_ms = m->payload.sensor.timestamp_ms - prev->last_timestamp_ms;
            if (dt_ms > 0) {
                float observed_mpm = fabsf(m->payload.sensor.value - prev->last_value) * 60000.0f / (float)dt_ms;
                if (fabsf(observed_mpm - fabsf(m->payload.sensor.rate_of_change)) > TRINETRA_MAX_REPORTED_RATE_ERROR)
                    return reject(reason, reason_size, "reported/observed rate mismatch");
            }
        }
    }
    if (reason != NULL && reason_size > 0) snprintf(reason, reason_size, "accepted");
    return EOK;
}

void validator_state_commit(const trinetra_sensor_telemetry_t *sensor)
{
    validator_sensor_state_t *s = validator_state_get_or_create(sensor->node_id, sensor->sensor_id, sensor->sensor_type);
    if (s == NULL) return;
    s->last_sequence = sensor->sequence_number;
    s->last_timestamp_ms = sensor->timestamp_ms;
    s->last_value = sensor->value;
    s->sensor_type = sensor->sensor_type;
}
