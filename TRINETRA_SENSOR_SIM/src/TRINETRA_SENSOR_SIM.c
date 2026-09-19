#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <time.h>

#include "trinetra_config.h"
#include "trinetra_ipc.h"

#define SIM_PULSE 10
#define SIM_MESSAGES 20U

static int create_periodic_timer(int chid, timer_t *timer)
{
    struct sigevent ev;
    int coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) return -1;
    SIGEV_PULSE_INIT(&ev, coid, SIGEV_PULSE_PRIO_INHERIT, SIM_PULSE, 0);
    if (timer_create(CLOCK_MONOTONIC, &ev, timer) == -1) return -1;
    struct itimerspec spec;
    memset(&spec, 0, sizeof(spec));
    spec.it_value.tv_sec = 0;
    spec.it_value.tv_nsec = 100000000L;
    spec.it_interval.tv_sec = (time_t)(TRINETRA_SIM_PERIOD_MS / 1000ULL);
    spec.it_interval.tv_nsec = (long)((TRINETRA_SIM_PERIOD_MS % 1000ULL) * 1000000ULL);
    return timer_settime(*timer, 0, &spec, NULL);
}

static void make_sample(trinetra_sensor_telemetry_t *s, uint32_t seq)
{
    memset(s, 0, sizeof(*s));
    s->node_id = 1;
    s->sensor_id = 1;
    s->sensor_type = TRINETRA_SENSOR_WATER_LEVEL;
    s->sequence_number = seq;
    s->timestamp_ms = trinetra_now_ms();
    /* Deliberately move through advisory/warning/critical demo ranges. */
    s->value = 4.0f + (float)((seq % 16U) * 0.8f);
    if (s->value > TRINETRA_WATER_LEVEL_MAX_M) s->value = 4.0f;
    s->rate_of_change = 0.8f;
}

int main(void)
{
    printf("[SENSOR_SIM] Starting TRINETRA sensor simulator.\n");
    printf("[SENSOR_SIM] Test path: SENSOR_SIM -> SENSOR_INGEST -> VALIDATOR -> FUSION -> GOVERNOR -> ALERT\n");

    int chid = ChannelCreate(0);
    if (chid == -1) { perror("[SENSOR_SIM] ChannelCreate"); return EXIT_FAILURE; }

    timer_t timer;
    if (create_periodic_timer(chid, &timer) == -1) {
        perror("[SENSOR_SIM] timer");
        ChannelDestroy(chid);
        return EXIT_FAILURE;
    }

    int ingest_coid = -1;
    uint32_t seq = 1;
    unsigned sent = 0;
    unsigned connection_failures = 0;

    while (sent < SIM_MESSAGES) {
        struct {
            struct _pulse pulse;
        } pulse_msg;
        int rcvid = MsgReceive(chid, &pulse_msg, sizeof(pulse_msg), NULL);
        if (rcvid == -1) { if (errno == EINTR) continue; perror("[SENSOR_SIM] MsgReceive"); break; }
        if (rcvid != 0) continue;

        if (ingest_coid == -1) ingest_coid = trinetra_open_service(TRINETRA_SERVICE_SENSOR_INGEST);
        if (ingest_coid == -1) {
            connection_failures++;
            if ((connection_failures % 5U) == 0U)
                printf("[SENSOR_SIM] Waiting for SENSOR_INGEST... attempts=%u\n", connection_failures);
            continue;
        }

        trinetra_message_t msg;
        trinetra_fill_header(&msg, TRINETRA_MSG_SENSOR_TELEMETRY,
                             TRINETRA_PROC_SENSOR_SIM, TRINETRA_PROC_SENSOR_INGEST,
                             seq, sizeof(msg.payload.sensor));
        make_sample(&msg.payload.sensor, seq);

        trinetra_reply_t reply;
        int rc = trinetra_send(ingest_coid, &msg, &reply);
        if (rc != EOK) {
            printf("[SENSOR_SIM] send seq=%u failed rc=%d; reconnecting\n", seq, rc);
            ConnectDetach(ingest_coid);
            ingest_coid = -1;
            continue;
        }

        printf("[SENSOR_SIM] sent seq=%u water=%.2f m rate=%.2f m/min\n",
               seq, msg.payload.sensor.value, msg.payload.sensor.rate_of_change);
        sent++;
        seq++;
    }

    printf("[SENSOR_SIM] Scenario complete. messages=%u\n", sent);
    timer_delete(timer);
    if (ingest_coid != -1) ConnectDetach(ingest_coid);
    ChannelDestroy(chid);
    return (sent == SIM_MESSAGES) ? EXIT_SUCCESS : EXIT_FAILURE;
}
