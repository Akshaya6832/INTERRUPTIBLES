#include <stdio.h>
#include <string.h>
#include "trinetra_ipc.h"
#include "alert_state.h"

static uint8_t active;
static uint8_t last_severity;

void alert_state_init(void) { active = 0; last_severity = TRINETRA_SEVERITY_NORMAL; }

void alert_handle_warning(const trinetra_warning_state_t *w, trinetra_alert_command_t *c, uint32_t sequence)
{
    memset(c, 0, sizeof(*c));
    c->timestamp_ms = trinetra_now_ms();
    c->severity = w ? w->severity : TRINETRA_SEVERITY_NORMAL;
    c->activate = (c->severity >= TRINETRA_SEVERITY_WARNING) ? 1U : 0U;
    c->output_mask = c->activate ? 0x0003U : 0U;
    c->sequence_number = sequence;
    snprintf(c->text, sizeof(c->text), "%s", w ? w->reason : "no warning data");
    active = c->activate;
    last_severity = c->severity;
}
