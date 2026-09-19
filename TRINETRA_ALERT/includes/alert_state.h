#ifndef ALERT_STATE_H
#define ALERT_STATE_H
#include "trinetra_types.h"
void alert_state_init(void);
void alert_handle_warning(const trinetra_warning_state_t *warning, trinetra_alert_command_t *command, uint32_t sequence);
#endif
