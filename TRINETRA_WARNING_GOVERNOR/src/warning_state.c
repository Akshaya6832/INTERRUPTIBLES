#include <stdio.h>
#include <string.h>
#include "trinetra_ipc.h"
#include "warning_state.h"

static trinetra_severity_t current_severity;

void governor_state_init(void) { current_severity = TRINETRA_SEVERITY_NORMAL; }

void governor_evaluate(const trinetra_fused_hazard_state_t *hazard, trinetra_warning_state_t *warning)
{
    memset(warning, 0, sizeof(*warning));
    warning->timestamp_ms = trinetra_now_ms();
    warning->confidence = hazard ? hazard->confidence : 0;
    trinetra_severity_t proposed = hazard ? (trinetra_severity_t)hazard->severity : TRINETRA_SEVERITY_NORMAL;
    if (!hazard || !hazard->data_fresh || hazard->confidence < 50) proposed = TRINETRA_SEVERITY_NORMAL;
    if (proposed > current_severity || proposed == current_severity) current_severity = proposed;
    else if (proposed + 1 >= current_severity) current_severity = proposed;
    warning->severity = (uint8_t)current_severity;
    warning->state_changed = (warning->severity != (uint8_t)proposed) ? 0U : 1U;
    warning->reason_code = hazard ? (uint32_t)hazard->severity : 0U;
    snprintf(warning->reason, sizeof(warning->reason), "hazard_score=%.1f water=%.2fm rate=%.2fm/min",
             hazard ? hazard->hazard_score : 0.0f,
             hazard ? hazard->water_level_m : 0.0f,
             hazard ? hazard->water_rate_mpm : 0.0f);
}
