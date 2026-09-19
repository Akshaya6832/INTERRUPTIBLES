#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <time.h>

#include "trinetra_config.h"
#include "trinetra_ipc.h"
#include "validator_rules.h"
#include "validator_state.h"

#define MAINTENANCE_PULSE 20

static int create_timer(int chid, timer_t *timer)
{
    struct sigevent ev;
    int coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) return -1;
    SIGEV_PULSE_INIT(&ev, coid, SIGEV_PULSE_PRIO_INHERIT, MAINTENANCE_PULSE, 0);
    if (timer_create(CLOCK_MONOTONIC, &ev, timer) == -1) return -1;
    struct itimerspec spec;
    memset(&spec, 0, sizeof(spec));
    spec.it_value.tv_sec = 1;
    spec.it_interval.tv_sec = 1;
    return timer_settime(*timer, 0, &spec, NULL);
}

static void send_validated(int coid, const trinetra_message_t *in, uint32_t seq)
{
    if (coid == -1) return;
    trinetra_message_t out;
    trinetra_fill_header(&out, TRINETRA_MSG_VALIDATED_TELEMETRY,
                         TRINETRA_PROC_VALIDATOR, TRINETRA_PROC_HAZARD_FUSION,
                         seq, sizeof(out.payload.validated));
    out.payload.validated.sensor = in->payload.sensor;
    out.payload.validated.quality = 100;
    trinetra_reply_t reply;
    int rc = trinetra_send(coid, &out, &reply);
    if (rc != EOK) fprintf(stderr, "[VALIDATOR] HAZARD_FUSION send failed rc=%d\n", rc);
}

int main(void)
{
    printf("[VALIDATOR] Starting TRINETRA validator.\n");
    printf("[VALIDATOR] age=%llu ms capacity=%u water=[%.1f, %.1f]m max_rate=%.1f m/min\n",
           (unsigned long long)TRINETRA_MAX_SENSOR_AGE_MS, TRINETRA_MAX_SENSOR_STATES,
           TRINETRA_WATER_LEVEL_MIN_M, TRINETRA_WATER_LEVEL_MAX_M, TRINETRA_WATER_LEVEL_MAX_RATE_MPM);

    validator_state_init();
    name_attach_t *attach = name_attach(NULL, TRINETRA_SERVICE_VALIDATOR, 0);
    if (attach == NULL) { perror("[VALIDATOR] name_attach"); return EXIT_FAILURE; }

    timer_t timer;
    if (create_timer(attach->chid, &timer) == -1) perror("[VALIDATOR] timer");

    int fusion_coid = -1;
    uint32_t out_seq = 1;
    unsigned total = 0, accepted = 0, rejected = 0;
    trinetra_process_state_t state = TRINETRA_STATE_RUNNING;

    printf("[VALIDATOR] state=%s service=%s chid=%d\n", trinetra_state_name(state), TRINETRA_SERVICE_VALIDATOR, attach->chid);

    for (;;) {
        trinetra_message_t msg;
        memset(&msg, 0, sizeof(msg));
        int rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) { if (errno == EINTR) continue; perror("[VALIDATOR] MsgReceive"); break; }
        if (rcvid == 0) {
            if (fusion_coid == -1) fusion_coid = trinetra_open_service(TRINETRA_SERVICE_HAZARD_FUSION);
            if (fusion_coid == -1) state = TRINETRA_STATE_DEGRADED;
            continue;
        }
        if (rcvid < 0) continue;

        total++;
        char reason[TRINETRA_MAX_REASON];
        int result = validator_validate(&msg, reason, sizeof(reason));
        trinetra_reply_t reply;
        reply.status = result;
        reply.reserved = 0;

        if (result == EOK) {
            accepted++;
            validator_state_commit(&msg.payload.sensor);
            if (fusion_coid == -1) fusion_coid = trinetra_open_service(TRINETRA_SERVICE_HAZARD_FUSION);
            if (fusion_coid == -1) {
                state = TRINETRA_STATE_DEGRADED;
                printf("[VALIDATOR] accepted seq=%u but FUSION unavailable\n", msg.payload.sensor.sequence_number);
            } else {
                send_validated(fusion_coid, &msg, out_seq++);
                state = TRINETRA_STATE_RUNNING;
            }
        } else {
            rejected++;
        }

        printf("[VALIDATOR] seq=%u result=%s reason=%s totals=%u/%u/%u state=%s\n",
               msg.payload.sensor.sequence_number,
               result == EOK ? "ACCEPT" : "REJECT", reason,
               total, accepted, rejected, trinetra_state_name(state));
        MsgReply(rcvid, EOK, &reply, sizeof(reply));
    }

    state = TRINETRA_STATE_STOPPING;
    printf("[VALIDATOR] state=%s\n", trinetra_state_name(state));
    name_detach(attach, 0);
    return EXIT_SUCCESS;
}
