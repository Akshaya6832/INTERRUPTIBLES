<!-- Engineering student note: README.md documents deployment, startup, configuration, and target integration. -->
# ESP32 -> Raspberry Pi integration boundary

Current TRINETRA bring-up uses `TRINETRA_SENSOR_SIM` because the LoRa path is not
currently reliable. Do not bypass VALIDATOR when the ESP32 path is added.

Required logical fields from ESP32 telemetry:

- node_id
- sensor_id
- sensor_type
- sequence_number
- timestamp_ms
- value
- rate_of_change

The Raspberry Pi/QNX side must convert the external packet into the existing
`trinetra_sensor_telemetry_t` and then feed it through:

ESP32 transport -> SENSOR_INGEST -> VALIDATOR -> HAZARD_FUSION -> WARNING_GOVERNOR -> ALERT

Do not invent a UART/SPI register map or a LoRa packet format until the actual
ESP32 firmware and physical link are selected. The current `sensor_hal.c`,
`uart_interface.c`, and `spi_interface.c` files are deliberately adapter hooks.

For the first hardware test, keep `TRINETRA_SENSOR_SIM` available as a fallback
source. That lets the QNX chain be verified independently of the LoRa link.
