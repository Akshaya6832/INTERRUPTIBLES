#include <string.h>
#include "trinetra_ipc.h"
#include "trinetra_config.h"
#include "fusion_state.h"

static trinetra_validated_telemetry_t latest;
static uint8_t have_water;

void fusion_state_init(void)
{
    memset(&latest, 0, sizeof(latest));
    have_water = 0;
}

void fusion_state_update(const trinetra_validated_telemetry_t *s)
{
    if (s == NULL) return;
    latest = *s;
    if (s->sensor.sensor_type == TRINETRA_SENSOR_WATER_LEVEL) have_water = 1;
}

void fusion_state_evaluate(trinetra_fused_hazard_state_t *out)
{
    memset(out, 0, sizeof(*out));
    out->evaluation_timestamp_ms = trinetra_now_ms();
    if (!have_water) {
        out->data_fresh = 0;
        out->severity = TRINETRA_SEVERITY_NORMAL;
        return;
    }
    out->water_level_m = latest.sensor.value;
    out->water_rate_mpm = latest.sensor.rate_of_change;
    out->data_fresh = 1;
    float score = (out->water_level_m / TRINETRA_WATER_LEVEL_MAX_M) * 100.0f;
    if (out->water_rate_mpm > TRINETRA_DEMO_RATE_WARNING_MPM) score += 10.0f;
    if (score > 100.0f) score = 100.0f;
    out->hazard_score = score;
    out->confidence = latest.quality;
    if (out->water_level_m >= TRINETRA_DEMO_WATER_CRITICAL_M) out->severity = TRINETRA_SEVERITY_CRITICAL;
    else if (out->water_level_m >= TRINETRA_DEMO_WATER_WARNING_M) out->severity = TRINETRA_SEVERITY_WARNING;
    else if (out->water_level_m >= TRINETRA_DEMO_WATER_ADVISORY_M) out->severity = TRINETRA_SEVERITY_ADVISORY;
    else out->severity = TRINETRA_SEVERITY_NORMAL;
}
