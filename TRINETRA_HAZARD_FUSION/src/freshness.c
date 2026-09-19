#include "trinetra_ipc.h"
int trinetra_freshness_ok(uint64_t timestamp_ms, uint64_t max_age_ms)
{ uint64_t now = trinetra_now_ms(); return (now >= timestamp_ms) && ((now - timestamp_ms) <= max_age_ms); }
