#ifndef TRINETRA_IPC_H
#define TRINETRA_IPC_H

#include <stdint.h>
#include <sys/dispatch.h>
#include "trinetra_protocol.h"

#define TRINETRA_SERVICE_SENSOR_INGEST "trinetra/sensor_ingest"
#define TRINETRA_SERVICE_VALIDATOR "trinetra/validator"
#define TRINETRA_SERVICE_HAZARD_FUSION "trinetra/hazard_fusion"
#define TRINETRA_SERVICE_WARNING_GOVERNOR "trinetra/warning_governor"
#define TRINETRA_SERVICE_ALERT "trinetra/alert"

int trinetra_open_service(const char *name);
int trinetra_send(int coid, const trinetra_message_t *message, trinetra_reply_t *reply);
const char *trinetra_process_name(uint32_t process_id);
const char *trinetra_state_name(trinetra_process_state_t state);
const char *trinetra_message_name(uint32_t message_type);
uint64_t trinetra_now_ms(void);
void trinetra_fill_header(trinetra_message_t *message,
                          uint32_t type,
                          uint32_t source,
                          uint32_t destination,
                          uint32_t sequence,
                          uint32_t payload_length);

#endif
