#ifndef FUSION_STATE_H
#define FUSION_STATE_H
#include "trinetra_types.h"
void fusion_state_init(void);
void fusion_state_update(const trinetra_validated_telemetry_t *sample);
void fusion_state_evaluate(trinetra_fused_hazard_state_t *out);
#endif
