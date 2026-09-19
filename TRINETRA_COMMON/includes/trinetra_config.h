#ifndef TRINETRA_CONFIG_H
#define TRINETRA_CONFIG_H

/* These values are the validation requirements already used by the working validator. */
#define TRINETRA_MAX_SENSOR_AGE_MS          5000ULL
#define TRINETRA_MAX_FUTURE_TIMESTAMP_MS    1000ULL
#define TRINETRA_WATER_LEVEL_MIN_M          0.0f
#define TRINETRA_WATER_LEVEL_MAX_M          20.0f
#define TRINETRA_WATER_LEVEL_MAX_RATE_MPM   2.0f
#define TRINETRA_MAX_REPORTED_RATE_ERROR    0.50f

#define TRINETRA_VALIDATOR_MAINTENANCE_MS   1000ULL
#define TRINETRA_FUSION_PERIOD_MS           250ULL
#define TRINETRA_GOVERNOR_PERIOD_MS         250ULL
#define TRINETRA_ALERT_SUPERVISION_MS       500ULL
#define TRINETRA_SIM_PERIOD_MS              1000ULL

/* Development/demo hazard thresholds. Replace with approved system requirements. */
#define TRINETRA_DEMO_WATER_ADVISORY_M     5.0f
#define TRINETRA_DEMO_WATER_WARNING_M      10.0f
#define TRINETRA_DEMO_WATER_CRITICAL_M     15.0f
#define TRINETRA_DEMO_RATE_WARNING_MPM      1.0f

#endif
