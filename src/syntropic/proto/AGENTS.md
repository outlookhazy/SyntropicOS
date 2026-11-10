# Automotive, Industrial & IoT Protocol Invariants (`src/syntropic/proto/`)

Applies to all wire, diagnostic, fieldbus, RC/telemetry, and IoT protocol engines under `src/syntropic/proto/`.

## Cross-Directory Touchpoints

- **Drivers & Transports**: `src/syntropic/drivers/syn_can.h` (Classic CAN `8B` and `SYN_USE_CAN_FD` `64B`), `syn_uart.h`, `src/syntropic/net/syn_transport.h`, and `src/syntropic/crypto/` (`syn_cose`, `syn_ocpp`).
- **Large Protocol Engines**: `11` files exceed `800` lines (`syn_uds.c`, `syn_lwm2m.c`, `syn_ethercat.c`, `syn_opcua.c`). Run `grep -n` for target service/frame handlers and read `StartLine..EndLine` slices when investigating specific services.

## Wire-Engine & State-Machine Invariants

- **Byte-Streaming Parser Resynchronization**: UART/RS-485/meter/RC byte-stream parsers (`syn_modbus`, `syn_dlt645`, `syn_cjt188`, `syn_mbus`, `syn_sbus`, `syn_ibus`, `syn_crsf`, `syn_msp`, `syn_mavlink`, `syn_nmea`, `syn_at_parser`) must resynchronize cleanly after false start bytes, truncated payloads, or CRC failures — re-scanning buffered bytes for valid sync headers rather than silently dropping embedded frames.
- **Classic CAN vs. CAN FD & Transport Framing Bounds**: Validate DLC/frame length against active bus mode (`8B` Classic CAN vs. up to `64B` CAN FD in `syn_isotp`, `syn_j1939`, `syn_n2k`, `syn_canopen`, `syn_cannm`, `syn_gbt27930`) and check caller `tx_buf`/`resp_cap` capacity before writing headers or padding.
- **Multi-Frame Session Isolation**: Concurrent or interleaved multi-packet sessions (`syn_isotp`, `syn_j1939` TP/BAM, `syn_n2k` Fast-Packet, `syn_lintp`) must isolate sequence counters, byte offsets, and PGN/ID state per session and abort/reset cleanly on RTS collision, sequence gap, or timeout.
- **Fail-Closed Diagnostic & Fieldbus Access Control**: Enforce session, routing-activation, SecurityAccess, and Object Dictionary access masks *before* mutating state or memory (`syn_uds`, `syn_doip`, `syn_kwp2000`, `syn_ccp`, `syn_xcp`, `syn_canopen`), returning standard negative response/abort codes on unauthorized or out-of-range requests.
- **Cross-Protocol Feature-Gate Independence**: Optional integrations between protocol engines (e.g., CoE Object Dictionary hooks in `syn_ethercat` when `SYN_USE_CANOPEN == 0`) must guard cross-module dependencies so every protocol compiles standalone.
