#!/bin/sh
# Development stop helper; use pidin to inspect before killing processes in production.
slay TRINETRA_SENSOR_SIM 2>/dev/null || true
slay TRINETRA_SENSOR_INGEST 2>/dev/null || true
slay TRINETRA_VALIDATOR 2>/dev/null || true
slay TRINETRA_HAZARD_FUSION 2>/dev/null || true
slay TRINETRA_WARNING_GOVERNOR 2>/dev/null || true
slay TRINETRA_ALERT 2>/dev/null || true
