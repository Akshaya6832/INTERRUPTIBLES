# TRINETRA — QNX 8.0 / Raspberry Pi 4

## Runtime architecture

Hardware -> SENSOR_INGEST -> VALIDATOR -> HAZARD_FUSION -> WARNING_GOVERNOR -> ALERT

All five runtime components are separate QNX processes. `TRINETRA_COMMON` is shared code, not a process. `TRINETRA_SENSOR_SIM` is a development/test process.

## Project structure

```text
TRINETRA/
├── TRINETRA_COMMON/
│   ├── includes/
│   │   ├── trinetra_types.h
│   │   ├── trinetra_protocol.h
│   │   ├── trinetra_ipc.h
│   │   ├── trinetra_config.h
│   │   └── trinetra_status.h
│   └── src/trinetra_ipc.c
├── TRINETRA_SENSOR_INGEST/
├── TRINETRA_SENSOR_SIM/
├── TRINETRA_VALIDATOR/
├── TRINETRA_HAZARD_FUSION/
├── TRINETRA_WARNING_GOVERNOR/
├── TRINETRA_ALERT/
├── deployment/
└── Makefile
```

Each executable project has its own `src/`, `includes/`, `build/`, `Makefile`, `.project`, and `.cproject`. The build directory is generated output; it is included only as a placeholder.

## Build

On the QNX host command prompt:

```bat
cd /d C:\Users\acer\qnx800_2
qnxsdp-env.bat
cd /d <path>\TRINETRA
make PLATFORM=aarch64le BUILD_PROFILE=debug
```

The six binaries are produced under each project's `build/aarch64le-debug/`.

## First bring-up

Start these in this order on the QNX target:

1. `TRINETRA_ALERT`
2. `TRINETRA_WARNING_GOVERNOR`
3. `TRINETRA_HAZARD_FUSION`
4. `TRINETRA_VALIDATOR`
5. `TRINETRA_SENSOR_INGEST`
6. `TRINETRA_SENSOR_SIM`

The simulator sends telemetry through the same QNX IPC chain used for the development path:

`SENSOR_SIM -> SENSOR_INGEST -> VALIDATOR -> HAZARD_FUSION -> WARNING_GOVERNOR -> ALERT`

## IPC

The data path uses QNX native synchronous message passing (`MsgSend`, `MsgReceive`, `MsgReply`) with named services for discovery. Timers use `timer_create`, `timer_settime`, `CLOCK_MONOTONIC`, and QNX pulses.

## Hardware boundary

`TRINETRA_SENSOR_INGEST/src/sensor_hal.c`, `uart_interface.c`, and `spi_interface.c` are the hardware boundary. No Raspberry Pi register map or undocumented sensor protocol is invented here. The current development input is the simulator.

## Important

The hazard thresholds in `trinetra_config.h` marked DEMO are implementation placeholders, not newly asserted safety requirements. They must be replaced by approved system requirements before any production/safety claim.
