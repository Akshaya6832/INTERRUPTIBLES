#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <time.h>

#include "trinetra_config.h"
#include "trinetra_ipc.h"
#include "sensor_hal.h"

#define PULSE_TIMER 1
#define RX_BUFFER 512

static int make_periodic_timer(int chid, uint64_t period_ms, timer_t *timer)
{
    struct sigevent event;
    SIGEV_PULSE_INIT(&event, ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0),
                     SIGEV_PULSE_PRIO_INHERIT, PULSE_TIMER, 0);
    if (timer_create(CLOCK_MONOTONIC, &event, timer) == -1) return -1;
    struct itimerspec spec;
    memset(&spec, 0, sizeof(spec));
    spec.it_value.tv_sec = (time_t)(period_ms / 1000ULL);
    spec.it_value.tv_nsec = (long)((period_ms % 1000ULL) * 1000000ULL);
    spec.it_interval = spec.it_value;
    return timer_settime(*timer, 0, &spec, NULL);
}

int main(void)
{
    trinetra_process_state_t state = TRINETRA_STATE_STARTING;
    name_attach_t *attach = name_attach(NULL, TRINETRA_SERVICE_SENSOR_INGEST, 0);
    if (attach == NULL) { perror("[SENSOR_INGEST] name_attach"); return EXIT_FAILURE; }

    if (sensor_hal_init() != 0) {
        state = TRINETRA_STATE_DEGRADED;
        printf("[SENSOR_INGEST] HAL unavailable; IPC test input remains enabled.\n");
    } else {
        state = TRINETRA_STATE_RUNNING;
    }
    printf("[SENSOR_INGEST] state=%s service=%s chid=%d\n",
           trinetra_state_name(state), TRINETRA_SERVICE_SENSOR_INGEST, attach->chid);

    timer_t timer;
    if (make_periodic_timer(attach->chid, TRINETRA_VALIDATOR_MAINTENANCE_MS, &timer) == -1)
        perror("[SENSOR_INGEST] timer_create/settime");

    int validator_coid = -1;
    uint64_t last_maintenance = trinetra_now_ms();
    unsigned received = 0, forwarded = 0;

    for (;;) {
        trinetra_message_t msg;
        memset(&msg, 0, sizeof(msg));
        int rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) { if (errno == EINTR) continue; perror("[SENSOR_INGEST] MsgReceive"); break; }
        if (rcvid == 0) {
            uint64_t now = trinetra_now_ms();
            if (now - last_maintenance >= TRINETRA_VALIDATOR_MAINTENANCE_MS) {
                if (validator_coid == -1) validator_coid = trinetra_open_service(TRINETRA_SERVICE_VALIDATOR);
                if (validator_coid == -1) {
                    state = TRINETRA_STATE_DEGRADED;
                    printf("[SENSOR_INGEST] state=DEGRADED validator unavailable\n");
                }
                last_maintenance = now;
            }
            continue;
        }
        if (rcvid < 0) continue;

        trinetra_reply_t reply = { EINVAL, 0 };
        if (msg.header.magic != TRINETRA_MAGIC || msg.header.version != TRINETRA_PROTOCOL_VERSION ||
            msg.header.message_type != TRINETRA_MSG_SENSOR_TELEMETRY ||
            msg.header.payload_length != sizeof(msg.payload.sensor)) {
            MsgReply(rcvid, EINVAL, &reply, sizeof(reply));
            continue;
        }

        received++;
        if (validator_coid == -1) validator_coid = trinetra_open_service(TRINETRA_SERVICE_VALIDATOR);
        if (validator_coid == -1) {
            state = TRINETRA_STATE_DEGRADED;
            reply.status = ENOENT;
        } else {
            msg.header.source_process = TRINETRA_PROC_SENSOR_INGEST;
            msg.header.destination_process = TRINETRA_PROC_VALIDATOR;
            int send_rc = trinetra_send(validator_coid, &msg, &reply);
            if (send_rc == 0) {
                forwarded++;
                state = TRINETRA_STATE_RUNNING;
            } else {
                state = TRINETRA_STATE_DEGRADED;
                reply.status = send_rc > 0 ? send_rc : EIO;
            }
        }
        MsgReply(rcvid, EOK, &reply, sizeof(reply));
        printf("[SENSOR_INGEST] rx=%u forwarded=%u state=%s\n",
               received, forwarded, trinetra_state_name(state));
    }

    state = TRINETRA_STATE_STOPPING;
    printf("[SENSOR_INGEST] state=%s\n", trinetra_state_name(state));
    sensor_hal_shutdown();
    name_detach(attach, 0);
    return EXIT_SUCCESS;
}
