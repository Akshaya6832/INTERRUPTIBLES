#ifndef TRINETRA_TYPES_H
#define TRINETRA_TYPES_H

#include <stdint.h>
#include <stddef.h>

#define TRINETRA_PROTOCOL_VERSION 1U
#define TRINETRA_MAGIC 0x54524E54U /* 'TRNT' */

#define TRINETRA_MAX_REASON 96U
#define TRINETRA_MAX_MESSAGE_SIZE 512U
#define TRINETRA_MAX_SENSOR_STATES 64U

/* Runtime process states. */
typedef enum {
    TRINETRA_STATE_STARTING = 0,
    TRINETRA_STATE_RUNNING,
    TRINETRA_STATE_DEGRADED,
    TRINETRA_STATE_ERROR,
    TRINETRA_STATE_STOPPING
} trinetra_process_state_t;

typedef enum {
    TRINETRA_PROC_UNKNOWN = 0,
    TRINETRA_PROC_SENSOR_SIM,
    TRINETRA_PROC_SENSOR_INGEST,
    TRINETRA_PROC_VALIDATOR,
    TRINETRA_PROC_HAZARD_FUSION,
    TRINETRA_PROC_WARNING_GOVERNOR,
    TRINETRA_PROC_ALERT
} trinetra_process_id_t;

typedef enum {
    TRINETRA_MSG_INVALID = 0,
    TRINETRA_MSG_SENSOR_TELEMETRY = 1,
    TRINETRA_MSG_VALIDATED_TELEMETRY = 2,
    TRINETRA_MSG_VALIDATION_RESULT = 3,
    TRINETRA_MSG_FUSED_HAZARD_STATE = 4,
    TRINETRA_MSG_WARNING_STATE = 5,
    TRINETRA_MSG_ALERT_COMMAND = 6,
    TRINETRA_MSG_HEALTH_STATUS = 7,
    TRINETRA_MSG_CONTROL_COMMAND = 8
} trinetra_message_type_t;

typedef enum {
    TRINETRA_SENSOR_UNKNOWN = 0,
    TRINETRA_SENSOR_RAINFALL = 1,
    TRINETRA_SENSOR_WATER_LEVEL = 2,
    TRINETRA_SENSOR_FLOW = 3,
    TRINETRA_SENSOR_TIDE_LEVEL = 4,
    TRINETRA_SENSOR_ACCELEROMETER = 5,
    TRINETRA_SENSOR_VIBRATION = 6
} trinetra_sensor_type_t;

typedef enum {
    TRINETRA_SEVERITY_NORMAL = 0,
    TRINETRA_SEVERITY_ADVISORY = 1,
    TRINETRA_SEVERITY_WARNING = 2,
    TRINETRA_SEVERITY_CRITICAL = 3
} trinetra_severity_t;

typedef enum {
    TRINETRA_VALIDATION_ACCEPT = 0,
    TRINETRA_VALIDATION_REJECT = 1
} trinetra_validation_decision_t;

typedef struct {
    uint16_t node_id;
    uint16_t sensor_id;
    uint32_t sensor_type;
    uint32_t sequence_number;
    uint64_t timestamp_ms;
    float value;
    float rate_of_change;
} trinetra_sensor_telemetry_t;

typedef struct {
    trinetra_sensor_telemetry_t sensor;
    uint8_t quality;
    uint8_t reserved[3];
} trinetra_validated_telemetry_t;

typedef struct {
    uint8_t decision;
    uint8_t reason_code;
    uint16_t reserved;
    char reason[TRINETRA_MAX_REASON];
} trinetra_validation_result_t;

typedef struct {
    uint64_t evaluation_timestamp_ms;
    float water_level_m;
    float water_rate_mpm;
    float rainfall;
    float flow;
    float tide_level_m;
    float hazard_score;
    uint8_t confidence;
    uint8_t severity;
    uint8_t data_fresh;
    uint8_t reserved;
} trinetra_fused_hazard_state_t;

typedef struct {
    uint64_t timestamp_ms;
    uint8_t severity;
    uint8_t confidence;
    uint8_t state_changed;
    uint8_t reserved;
    uint32_t reason_code;
    char reason[TRINETRA_MAX_REASON];
} trinetra_warning_state_t;

typedef struct {
    uint64_t timestamp_ms;
    uint8_t severity;
    uint8_t activate;
    uint16_t output_mask;
    uint32_t sequence_number;
    char text[TRINETRA_MAX_REASON];
} trinetra_alert_command_t;

#endif
