#include <stdio.h>
#include "trinetra_types.h"

int alert_output_apply(const trinetra_alert_command_t *command)
{
    if (!command) return -1;
    printf("[ALERT_OUTPUT] %s severity=%u mask=0x%04x\n",
           command->activate ? "ACTIVE" : "CLEAR",
           command->severity, command->output_mask);
    return 0;
}
