#include "trinetra_types.h"
trinetra_severity_t governor_apply_hysteresis(trinetra_severity_t previous, trinetra_severity_t proposed)
{
    /* One-step downward changes are allowed only after the hazard falls below the proposed level. */
    if (proposed < previous && (previous - proposed) > 1) return (trinetra_severity_t)(previous - 1);
    return proposed;
}
