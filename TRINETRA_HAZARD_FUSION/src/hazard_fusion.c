#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <time.h>
#include "trinetra_ipc.h"
#include "trinetra_config.h"
#include "fusion_state.h"

#define FUSION_PULSE 30

static int make_timer(int chid, timer_t *timer)
{
    struct sigevent ev;
    int coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) return -1;
    SIGEV_PULSE_INIT(&ev, coid, SIGEV_PULSE_PRIO_INHERIT, FUSION_PULSE, 0);
    if (timer_create(CLOCK_MONOTONIC, &ev, timer) == -1) return -1;
    struct itimerspec spec;
    memset(&spec, 0, sizeof(spec));
    spec.it_value.tv_sec = 0; spec.it_value.tv_nsec = 100000000L;
    spec.it_interval.tv_sec = 0; spec.it_interval.tv_nsec = 250000000L;
    return timer_settime(*timer, 0, &spec, NULL);
}

int main(void)
{
    fusion_state_init();
    name_attach_t *attach = name_attach(NULL, TRINETRA_SERVICE_HAZARD_FUSION, 0);
    if (!attach) { perror("[HAZARD_FUSION] name_attach"); return EXIT_FAILURE; }
    timer_t timer;
    if (make_timer(attach->chid, &timer) == -1) perror("[HAZARD_FUSION] timer");
    int governor_coid = -1;
    uint32_t out_seq = 1;
    trinetra_process_state_t state = TRINETRA_STATE_RUNNING;
    printf("[HAZARD_FUSION] state=%s service=%s chid=%d period=%llu ms\n",
           trinetra_state_name(state), TRINETRA_SERVICE_HAZARD_FUSION, attach->chid,
           (unsigned long long)TRINETRA_FUSION_PERIOD_MS);

    for (;;) {
        trinetra_message_t msg;
        memset(&msg, 0, sizeof(msg));
        int rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) { if (errno == EINTR) continue; perror("[HAZARD_FUSION] MsgReceive"); break; }
        if (rcvid == 0) {
            trinetra_fused_hazard_state_t hazard;
            fusion_state_evaluate(&hazard);
            if (governor_coid == -1) governor_coid = trinetra_open_service(TRINETRA_SERVICE_WARNING_GOVERNOR);
            if (governor_coid == -1) {
                state = TRINETRA_STATE_DEGRADED;
            } else {
                trinetra_message_t out;
                trinetra_fill_header(&out, TRINETRA_MSG_FUSED_HAZARD_STATE,
                                     TRINETRA_PROC_HAZARD_FUSION, TRINETRA_PROC_WARNING_GOVERNOR,
                                     out_seq++, sizeof(out.payload.hazard));
                out.payload.hazard = hazard;
                trinetra_reply_t reply;
                int rc = trinetra_send(governor_coid, &out, &reply);
                state = (rc == EOK) ? TRINETRA_STATE_RUNNING : TRINETRA_STATE_DEGRADED;
                printf("[HAZARD_FUSION] score=%.1f severity=%u confidence=%u state=%s\n",
                       hazard.hazard_score, hazard.severity, hazard.confidence, trinetra_state_name(state));
            }
            continue;
        }
        if (rcvid < 0) continue;
        trinetra_reply_t reply = { EINVAL, 0 };
        if (msg.header.magic == TRINETRA_MAGIC && msg.header.version == TRINETRA_PROTOCOL_VERSION &&
            msg.header.message_type == TRINETRA_MSG_VALIDATED_TELEMETRY &&
            msg.header.payload_length == sizeof(msg.payload.validated) &&
            msg.header.destination_process == TRINETRA_PROC_HAZARD_FUSION) {
            fusion_state_update(&msg.payload.validated);
            reply.status = EOK;
        }
        MsgReply(rcvid, EOK, &reply, sizeof(reply));
    }
    name_detach(attach, 0);
    return EXIT_SUCCESS;
}
