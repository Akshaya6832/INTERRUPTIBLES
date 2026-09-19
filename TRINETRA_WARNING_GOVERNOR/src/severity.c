#include "trinetra_types.h"
trinetra_severity_t governor_severity_from_hazard(const trinetra_fused_hazard_state_t *h)
{ return h ? (trinetra_severity_t)h->severity : TRINETRA_SEVERITY_NORMAL; }
