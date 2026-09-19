#include "trinetra_types.h"
int governor_confidence_usable(const trinetra_fused_hazard_state_t *h)
{ return h && h->data_fresh && h->confidence >= 50; }
