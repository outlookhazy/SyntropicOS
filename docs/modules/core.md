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

## Cryptography

| Module | Header | Config |
|---|---|---|
| SHA-256 | `util/syn_sha256.h` | `SYN_USE_SHA256` — FIPS 180-4 SHA-256 hash (streaming: init → update → final) |
| HMAC-SHA256 | `util/syn_hmac.h` | `SYN_USE_SHA256` — HMAC-SHA256 keyed message authentication code |

Both modules are pure C99 with no external dependencies. The SHA-256 context is ~112 bytes (caller-owned). HMAC is header-only and wraps two SHA-256 contexts.

```c
// One-shot hash
uint8_t hash[32];
syn_sha256("abc", 3, hash);

// Streaming hash
SYN_SHA256 ctx;
syn_sha256_init(&ctx);
syn_sha256_update(&ctx, chunk1, len1);
syn_sha256_update(&ctx, chunk2, len2);
syn_sha256_final(&ctx, hash);

// HMAC-SHA256 (one-shot)
uint8_t mac[32];
syn_hmac_sha256(key, key_len, message, msg_len, mac);
```

## Memory

| Module | Header | Config |
|---|---|---|
| Block Pool | `util/syn_pool.h` | `SYN_USE_POOL` — Fixed-size block memory pool with O(1) alloc/free |

Header-only allocator backed by a caller-provided byte array. All blocks are the same size. Blocks are automatically 4-byte aligned. Tracks high watermark for diagnostics.

```c
static uint8_t backing[SYN_POOL_BUF_SIZE(64, 8)];  // 8 blocks × 64 bytes
static SYN_Pool pool;

syn_pool_init(&pool, backing, sizeof(backing), 64);

void *blk = syn_pool_alloc(&pool);   // O(1) pop from freelist
// ... use blk (64 bytes) ...
syn_pool_free(&pool, blk);           // O(1) push to freelist

printf("peak usage: %zu blocks\n", syn_pool_high_watermark(&pool));
```

## Messaging

| Module | Header | Config |
|---|---|---|
| Pub/Sub | `util/syn_pubsub.h` | `SYN_USE_PUBSUB` — Non-blocking publish-subscribe message bus |

