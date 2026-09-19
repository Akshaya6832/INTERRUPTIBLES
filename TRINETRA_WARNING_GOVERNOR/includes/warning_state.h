#ifndef WARNING_STATE_H
#define WARNING_STATE_H
#include "trinetra_types.h"
void governor_state_init(void);
void governor_evaluate(const trinetra_fused_hazard_state_t *hazard, trinetra_warning_state_t *warning);
#endif
