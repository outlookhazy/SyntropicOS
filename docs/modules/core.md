# Core & Utilities

These modules are **always included** by the umbrella header `syntropic/syntropic.h`. They are header-only or have negligible code size — zero cost if unused.

## Foundation

| Module | Header | Description |
|---|---|---|
| Defs | `common/syn_defs.h` | `SYN_Status` return codes (`SYN_OK`, `SYN_ERROR`, `SYN_BUSY`, `SYN_TIMEOUT`, `SYN_INVALID_PARAM`, `SYN_NOT_IMPLEMENTED`), GPIO types, utility macros (`SYN_MIN`, `SYN_MAX`, `SYN_CLAMP`, `SYN_ARRAY_SIZE`) |
| Compiler | `common/syn_compiler.h` | Portable compiler attributes: `SYN_WEAK`, `SYN_NORETURN`, `SYN_PACKED`, `SYN_FALLTHROUGH`, etc. |
| Assert | `util/syn_assert.h` | `SYN_ASSERT(expr)` — compile-time strippable via `SYN_DISABLE_ASSERT` |

## Data Structures

| Module | Header | Description |
|---|---|---|
| Ring Buffer | `util/syn_ringbuf.h` | ISR-safe single-producer/single-consumer (SPSC) ring buffer. Used internally by UART TX/RX and other modules. |
| Ping-Pong | `util/syn_pingpong.h` | Double buffer for DMA or ISR ↔ main-thread handoff |
| Data Pack | `util/syn_pack.h` | Binary structure serialization helper |

## Math & Numeric

| Module | Header | Description |
|---|---|---|
| Bits | `util/syn_bits.h` | Bit set/clear/toggle/test macros |
| CRC | `util/syn_crc.h` | CRC-8, CRC-16 (CCITT and Modbus variants), CRC-32 |
| Q-Math | `util/syn_qmath.h` | Q16.16 fixed-point multiply, divide, square root, and trigonometry — no floating point |
| LUT | `util/syn_lut.h` | Interpolated lookup table with forward and reverse lookup |

## Timing & Thresholds

| Module | Header | Description |
|---|---|---|
| Timeout | `util/syn_timeout.h` | Wrap-safe timeout helper using `syn_port_get_tick_ms()`. Also provides `PT_WAIT_TIMEOUT` for protothreads. |
| Hysteresis | `util/syn_hysteresis.h` | Deadband threshold comparator — prevents output oscillation near setpoints |
| Rate Limit | `util/syn_rate_limit.h` | Slew-rate / rate-of-change limiter |
| Change Filter | `util/syn_change_filter.h` | Value change detection filter — suppresses updates until value changes by a threshold |

## Motion

| Module | Header | Config |
|---|---|---|
| Ramp Gen | `util/syn_ramp.h` | `SYN_USE_RAMP` — Trapezoidal motion ramp generator |
| S-Curve Gen | `util/syn_scurve.h` | `SYN_USE_SCURVE` — Jerk-limited s-curve trajectory generator |

## Serialization

| Module | Header | Config |
|---|---|---|
| Formatting | `util/syn_fmt.h` | `SYN_USE_FMT` — Lightweight printf alternative. No `stdio.h` dependency. Used by logging, CLI, profiler, and trace modules. |
| JSON Reader | `util/syn_json_read.h` | `SYN_USE_JSON` — Zero-allocation streaming JSON reader |
| JSON Writer | `util/syn_json_write.h` | `SYN_USE_JSON` — Zero-allocation streaming JSON writer |
| CBOR Reader | `util/syn_cbor_read.h` | `SYN_USE_CBOR` — Zero-allocation streaming CBOR binary reader |
| CBOR Writer | `util/syn_cbor_write.h` | `SYN_USE_CBOR` — Zero-allocation streaming CBOR binary writer |

## Messaging

| Module | Header | Config |
|---|---|---|
| Pub/Sub | `util/syn_pubsub.h` | `SYN_USE_PUBSUB` — Non-blocking publish-subscribe message bus |
