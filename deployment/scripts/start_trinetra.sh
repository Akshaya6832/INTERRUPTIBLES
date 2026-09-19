#!/bin/sh
# Run from the directory containing the six built binaries.
# Start downstream services first, then upstream input.
./TRINETRA_ALERT &
./TRINETRA_WARNING_GOVERNOR &
./TRINETRA_HAZARD_FUSION &
./TRINETRA_VALIDATOR &
./TRINETRA_SENSOR_INGEST &
./TRINETRA_SENSOR_SIM &
echo "TRINETRA processes started."
