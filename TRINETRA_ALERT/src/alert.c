#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <time.h>
#include "trinetra_config.h"
#include "trinetra_ipc.h"
#include "alert_state.h"

#define ALERT_PULSE 50
int alert_output_apply(const trinetra_alert_command_t *command);

static int make_timer(int chid, timer_t *timer)
{
    struct sigevent ev;
    int coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) return -1;
    SIGEV_PULSE_INIT(&ev, coid, SIGEV_PULSE_PRIO_INHERIT, ALERT_PULSE, 0);
    if (timer_create(CLOCK_MONOTONIC, &ev, timer) == -1) return -1;
    struct itimerspec spec;
    memset(&spec, 0, sizeof(spec));
    spec.it_value.tv_sec = 0; spec.it_value.tv_nsec = 100000000L;
    spec.it_interval.tv_sec = 0; spec.it_interval.tv_nsec = 500000000L;
    return timer_settime(*timer, 0, &spec, NULL);
}

int main(void)
{
    alert_state_init();
    name_attach_t *attach = name_attach(NULL, TRINETRA_SERVICE_ALERT, 0);
    if (!attach) { perror("[ALERT] name_attach"); return EXIT_FAILURE; }
    timer_t timer;
    if (make_timer(attach->chid, &timer) == -1) perror("[ALERT] timer");
    trinetra_process_state_t state = TRINETRA_STATE_RUNNING;
    uint64_t last_command_ms = 0;
    uint32_t out_seq = 1;
    printf("[ALERT] state=%s service=%s chid=%d supervision=%llu ms\n",
           trinetra_state_name(state), TRINETRA_SERVICE_ALERT, attach->chid,
           (unsigned long long)TRINETRA_ALERT_SUPERVISION_MS);

    for (;;) {
        trinetra_message_t msg;
        memset(&msg, 0, sizeof(msg));
        int rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) { if (errno == EINTR) continue; perror("[ALERT] MsgReceive"); break; }
        if (rcvid == 0) {
            if (last_command_ms != 0 && trinetra_now_ms() - last_command_ms > TRINETRA_ALERT_SUPERVISION_MS * 3ULL)
                state = TRINETRA_STATE_DEGRADED;
            printf("[ALERT] supervision state=%s\n", trinetra_state_name(state));
            continue;
        }
        if (rcvid < 0) continue;

        trinetra_reply_t reply = { EINVAL, 0 };
        if (msg.header.magic == TRINETRA_MAGIC && msg.header.version == TRINETRA_PROTOCOL_VERSION &&
            msg.header.message_type == TRINETRA_MSG_WARNING_STATE &&
            msg.header.payload_length == sizeof(msg.payload.warning) &&
            msg.header.destination_process == TRINETRA_PROC_ALERT) {
            trinetra_alert_command_t command;
            alert_handle_warning(&msg.payload.warning, &command, out_seq++);
            if (alert_output_apply(&command) == 0) {
                last_command_ms = trinetra_now_ms();
                state = TRINETRA_STATE_RUNNING;
                reply.status = EOK;
                printf("[ALERT] command seq=%u activate=%u severity=%u state=%s\n",
                       command.sequence_number, command.activate, command.severity, trinetra_state_name(state));
            } else {
                state = TRINETRA_STATE_ERROR;
                reply.status = EIO;
            }
        }
        MsgReply(rcvid, EOK, &reply, sizeof(reply));
    }
    name_detach(attach, 0);
    return EXIT_SUCCESS;
}
