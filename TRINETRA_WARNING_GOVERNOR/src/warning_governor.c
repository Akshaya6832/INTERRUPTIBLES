#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <time.h>
#include "trinetra_config.h"
#include "trinetra_ipc.h"
#include "warning_state.h"

#define GOV_PULSE 40

static int make_timer(int chid, timer_t *timer)
{
    struct sigevent ev;
    int coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) return -1;
    SIGEV_PULSE_INIT(&ev, coid, SIGEV_PULSE_PRIO_INHERIT, GOV_PULSE, 0);
    if (timer_create(CLOCK_MONOTONIC, &ev, timer) == -1) return -1;
    struct itimerspec spec;
    memset(&spec, 0, sizeof(spec));
    spec.it_value.tv_sec = 0; spec.it_value.tv_nsec = 100000000L;
    spec.it_interval.tv_sec = 0; spec.it_interval.tv_nsec = 250000000L;
    return timer_settime(*timer, 0, &spec, NULL);
}

int main(void)
{
    governor_state_init();
    name_attach_t *attach = name_attach(NULL, TRINETRA_SERVICE_WARNING_GOVERNOR, 0);
    if (!attach) { perror("[WARNING_GOVERNOR] name_attach"); return EXIT_FAILURE; }
    timer_t timer;
    if (make_timer(attach->chid, &timer) == -1) perror("[WARNING_GOVERNOR] timer");
    int alert_coid = -1;
    uint32_t out_seq = 1;
    trinetra_fused_hazard_state_t latest;
    memset(&latest, 0, sizeof(latest));
    int have_latest = 0;
    printf("[WARNING_GOVERNOR] state=RUNNING service=%s chid=%d period=%llu ms\n",
           TRINETRA_SERVICE_WARNING_GOVERNOR, attach->chid,
           (unsigned long long)TRINETRA_GOVERNOR_PERIOD_MS);

    for (;;) {
        trinetra_message_t msg;
        memset(&msg, 0, sizeof(msg));
        int rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) { if (errno == EINTR) continue; perror("[WARNING_GOVERNOR] MsgReceive"); break; }
        if (rcvid == 0) {
            trinetra_warning_state_t warning;
            governor_evaluate(have_latest ? &latest : NULL, &warning);
            if (alert_coid == -1) alert_coid = trinetra_open_service(TRINETRA_SERVICE_ALERT);
            if (alert_coid == -1) {
                printf("[WARNING_GOVERNOR] ALERT unavailable; state=DEGRADED\n");
            } else {
                trinetra_message_t out;
                trinetra_fill_header(&out, TRINETRA_MSG_WARNING_STATE,
                                     TRINETRA_PROC_WARNING_GOVERNOR, TRINETRA_PROC_ALERT,
                                     out_seq++, sizeof(out.payload.warning));
                out.payload.warning = warning;
                trinetra_reply_t reply;
                int rc = trinetra_send(alert_coid, &out, &reply);
                if (rc != EOK) { ConnectDetach(alert_coid); alert_coid = -1; }
                printf("[WARNING_GOVERNOR] severity=%u confidence=%u state=%s\n",
                       warning.severity, warning.confidence,
                       rc == EOK ? "RUNNING" : "DEGRADED");
            }
            continue;
        }
        if (rcvid < 0) continue;
        trinetra_reply_t reply = { EINVAL, 0 };
        if (msg.header.magic == TRINETRA_MAGIC && msg.header.version == TRINETRA_PROTOCOL_VERSION &&
            msg.header.message_type == TRINETRA_MSG_FUSED_HAZARD_STATE &&
            msg.header.payload_length == sizeof(msg.payload.hazard) &&
            msg.header.destination_process == TRINETRA_PROC_WARNING_GOVERNOR) {
            latest = msg.payload.hazard;
            have_latest = 1;
            reply.status = EOK;
        }
        MsgReply(rcvid, EOK, &reply, sizeof(reply));
    }
    name_detach(attach, 0);
    return EXIT_SUCCESS;
}
