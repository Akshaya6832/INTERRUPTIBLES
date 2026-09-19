#include "trinetra_types.h"
int fusion_quality_score(const trinetra_validated_telemetry_t *s)
{ return (s != 0) ? (int)s->quality : 0; }
