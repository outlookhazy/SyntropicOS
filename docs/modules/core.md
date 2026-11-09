# Core & Utility Modules

`src/syntropic/common/` and `src/syntropic/util/` provide SyntropicOS's zero-heap foundation: status codes, compiler portability macros, lock-free ISR-to-task queues, static block/slab allocators, wire codecs (JSON, CBOR, Protobuf, Base64, LZ4, CRC), string formatting, finite state machines, pub/sub messaging, Q16.16 fixed-point linear algebra, motion profiles, and timing helpers.

---

## Technical Overview

| Domain | Headers | Allocation & Concurrency |
|---|---|---|
| **Foundation & Asserts** | `common/syn_defs.h`, `common/syn_compiler.h`, `common/syn_barrier.h`, `util/syn_bits.h`, `util/syn_assert.h` | Header-only, always enabled; `SYN_DISABLE_ASSERT` strips runtime checks |
| **Lock-Free Queues & Streams** | `util/syn_ringbuf.h`, `util/syn_spsc_queue.h`, `util/syn_stream.h`, `util/syn_pingpong.h` | Caller-owned buffers; lock-free SPSC with acquire/release barriers (`ISR <-> task`) |
| **Static Memory Allocators** | `util/syn_pool.h`, `util/syn_slab.h` | O(1) deterministic block (`SYN_Pool`) and multi-class (`SYN_SlabAllocator`) pools |
| **Packing, Checksums & Codecs** | `util/syn_pack.h`, `util/syn_crc.h`, `util/syn_base64.h`, `util/syn_lz4.h`, `util/syn_json_read.h`, `util/syn_json_write.h`, `util/syn_cbor_read.h`, `util/syn_cbor_write.h`, `util/syn_protobuf.h` | Alignment-safe endian packing; zero-malloc streaming parsers, encoders, and LZ4 |
| **Strings & Formatting** | `util/syn_str.h`, `util/syn_fmt.h` | `printf`-free integer/hex/Q16.16 formatting and bounds-checked string parsing |
| **State, Events & Pub/Sub** | `util/syn_fsm.h`, `util/syn_pubsub.h`, `util/syn_event.h`, `util/syn_spinlock.h` | Table-driven FSMs, static topic broker, 32-bit event flags, AMP spinlock guards |
| **Fixed-Point Math & Geometry** | `util/syn_qmath.h`, `util/syn_vector.h`, `util/syn_matrix.h`, `util/syn_quaternion.h`, `util/syn_transform.h`, `util/syn_geo.h`, `util/syn_lut.h`, `util/syn_nn.h` | Q16.16 / Q15 / Q7 arithmetic, LU/Cholesky/QR solvers, quaternions, WGS-84/ENU |
| **Motion & Conditioning** | `util/syn_ramp.h`, `util/syn_scurve.h`, `util/syn_hysteresis.h`, `util/syn_change_filter.h` | Linear/trapezoidal ramps, 7-phase jerk-limited 1D/3D S-curves, deadband filters |
| **Timing, Retry & Telemetry** | `util/syn_timeout.h`, `util/syn_backoff.h`, `util/syn_rate_limit.h`, `util/syn_random.h`, `util/syn_metrics.h` | Wrap-safe millisecond timeouts, jittered backoff, token-bucket rate limit, metrics |

---

## 1. Foundation, Compiler, Bits & Asserts

- **`common/syn_defs.h`** *(always enabled)*: Core return enum `SYN_Status` (`SYN_OK = 0`, `SYN_ERROR`, `SYN_BUSY`, `SYN_TIMEOUT`, `SYN_INVALID_PARAM`, `SYN_NOT_IMPLEMENTED`, `SYN_NOT_FOUND`), `SYN_GPIO_Pin`, `SYN_GPIO_Mode`, `SYN_UARTInstance`, and utility macros (`SYN_ARRAY_SIZE`, `SYN_UNUSED_VAR`, `SYN_MIN`, `SYN_MAX`, `SYN_ABS`, `SYN_SIGN`, `SYN_CLAMP`).
- **`common/syn_compiler.h`** *(always enabled)*: Portable toolchain attributes across GCC, Clang, ARM Compiler, and IAR (`SYN_WEAK`, `SYN_PACKED`, `SYN_INLINE`, `SYN_UNUSED`, `SYN_NORETURN`, `SYN_STATIC_ASSERT(expr, msg)`, `SYN_SECTION(name)`, `SYN_ALIGN(n)`, `SYN_COMPILER_BARRIER()`, `SYN_FALLTHROUGH`).
- **`common/syn_barrier.h`** *(always enabled; hardware fence when `SYN_USE_MULTICORE=1`)*: `SYN_LOAD_ACQUIRE(ptr)` / `SYN_STORE_RELEASE(ptr, val)` (`syn_load_acquire`, `syn_store_release`) memory-ordering primitives for ISR and dual-core lock-free structures.
- **`util/syn_bits.h`** *(header-only)*: Bit/field manipulation macros (`SYN_BIT`, `SYN_BIT_SET`, `SYN_BIT_CLEAR`, `SYN_BIT_TOGGLE`, `SYN_BIT_CHECK`, `SYN_BITMASK(width, offset)`, `SYN_BITS_GET`, `SYN_BITS_SET`, `SYN_BITS_TO_BYTES`).
- **`util/syn_assert.h`** *(header-only)*: `SYN_ASSERT(expr)` invokes weak `syn_assert_failed(const char *file, int line)` on violation; define `SYN_DISABLE_ASSERT` in `syn_config.h` to compile all asserts to `((void)0)`.

---

## 2. Lock-Free Queues, Streams & Double Buffers

### Cooperative Byte Stream (`util/syn_stream.h`)

`SYN_Stream` wraps a lock-free `SYN_RingBuf` with protothread wake conditions (`PT_STREAM_WAIT` polled or `PT_BLOCK_STREAM` scheduler-parked):
- **Default mode**: Readable when >= 1 byte is buffered.
- **Threshold mode** (`syn_stream_set_threshold(&s, n)`): Readable when >= `n` bytes arrive.
- **Delimiter mode** (`syn_stream_set_delimiter(&s, '\n')`): Readable when the delimiter byte arrives. If an oversized frame fills the buffer without a delimiter, `SYN_Stream` flushes the partial frame and discards trailing bytes until the next delimiter arrives so `syn_stream_read_line()` never returns a truncated frame.

```c
#include "syntropic/syntropic.h"

static uint8_t s_raw_buf[128];
static SYN_Stream s_rx_stream;

void stream_setup(void) {
    syn_stream_init(&s_rx_stream, s_raw_buf, sizeof(s_raw_buf));
    syn_stream_set_delimiter(&s_rx_stream, '\n');
}

/* ISR producer: lock-free single-byte push */
void uart_rx_isr_byte(uint8_t byte) {
    (void)syn_stream_put(&s_rx_stream, byte);
}

/* Protothread consumer: wakes on complete '\n'-terminated lines */
SYN_PT_Status line_consumer_task(SYN_PT *pt, SYN_Task *task) {
    static uint8_t line_buf[64];
    size_t len;
    (void)task;

    PT_BEGIN(pt);
    for (;;) {
        PT_STREAM_WAIT(pt, &s_rx_stream);
        len = syn_stream_read_line(&s_rx_stream, line_buf, sizeof(line_buf));
        (void)len;
    }
    PT_END(pt);
}
```

### Byte Ring Buffer, Typed SPSC Queue & Ping-Pong Buffer

- **`util/syn_ringbuf.h`** (`SYN_RingBuf`, always enabled): Lock-free SPSC byte ring buffer (`size - 1` usable capacity) with single-byte (`syn_ringbuf_put`, `syn_ringbuf_get`, `syn_ringbuf_peek`) and bulk (`syn_ringbuf_write`, `syn_ringbuf_read`, `syn_ringbuf_peek_n`) APIs.
- **`util/syn_spsc_queue.h`** (`SYN_SPSC_Queue`, always enabled): Lock-free SPSC queue for fixed-size structs (`syn_spsc_queue_init(&q, buf, sizeof(elem), cap)`, `syn_spsc_queue_push`, `syn_spsc_queue_pop`, `PT_QUEUE_WAIT_POP`, `PT_BLOCK_QUEUE_POP`).
- **`util/syn_pingpong.h`** (`SYN_PingPong`, header-only): Zero-copy double buffer for DMA/ISR producers (`syn_pingpong_init(&pp, buf_a, buf_b, sz)`, `syn_pingpong_active` for ISR writes, `syn_pingpong_swap` on completion, `syn_pingpong_ready` / `syn_pingpong_ready_buf` / `syn_pingpong_consume` in task context).

---

## 3. Static Memory Allocators (`syn_pool` & `syn_slab`)

SyntropicOS forbids `malloc()`, providing two deterministic O(1) static allocators backed by caller-owned byte arrays:

- **`util/syn_pool.h`** (`SYN_USE_POOL`, header-only): Fixed-size block pool with embedded freelist, double-free detection bitmap, and `syn_pool_high_watermark()` tracking. Size backing storage with `SYN_POOL_BUF_SIZE(block_size, count)`.
- **`util/syn_slab.h`** (always enabled): Multi-class slab allocator (`SYN_SlabAllocator`, up to `SYN_SLAB_MAX_CLASSES = 8` sorted block sizes) routing `syn_slab_alloc(&slab, size)` to the smallest fitting class in O(1) time without fragmentation.

```c
#include "syntropic/util/syn_pool.h"

#define PKT_SIZE  64U
#define PKT_COUNT 8U

static uint8_t s_pool_mem[SYN_POOL_BUF_SIZE(PKT_SIZE, PKT_COUNT)];
static SYN_Pool s_pkt_pool;

void pkt_pool_setup(void) {
    syn_pool_init(&s_pkt_pool, s_pool_mem, sizeof(s_pool_mem), PKT_SIZE);
    void *blk = syn_pool_alloc(&s_pkt_pool);
    if (blk != NULL) {
        syn_pool_free(&s_pkt_pool, blk);
    }
}
```

---

## 4. Binary Packing, Checksums, Compression & Wire Codecs

### Endian-Safe Packing & CRC (`syn_pack.h`, `syn_crc.h`)

- **`util/syn_pack.h`** *(header-only)*: Alignment-safe wire serialization preventing unaligned-access faults on Cortex-M0/M0+:
  - Advancing cursor (`size_t *pos`): `syn_pack_u8/u16/u32/u64`, `syn_pack_i8/i16/i32`, `syn_pack_bytes`, `syn_unpack_*`, plus little-endian `_le` variants (`syn_pack_u16_le`, `syn_unpack_u32_le`, etc.).
  - Non-advancing offset (`size_t pos`): `syn_peek_u8/u16/u32[_le]` and `syn_poke_u16/u32[_le]`.
- **`util/syn_crc.h`** *(always enabled; `SYN_CRC_USE_TABLE` = `1` table / `0` bitwise)*: Single-shot and streaming (`_update`) calculators for CRC-8/MAXIM (`syn_crc8`), CRC-16/CCITT (`syn_crc16_ccitt`), CRC-16/Modbus (`syn_crc16_modbus`), and CRC-32 (`syn_crc32`).

### Structured Codecs & Compression (`JSON`, `CBOR`, `Protobuf`, `Base64`, `LZ4`)

| Module | Guard | Key Types & Entry Points | Summary |
|---|---|---|---|
| `util/syn_json_read.h` | `SYN_USE_JSON` | `SYN_JsonReader`; `syn_json_parse`, `syn_json_get_str`, `syn_json_get_int`, `syn_json_get_bool` | In-place tokenizer (up to 32 tokens) with dot-path nested key lookup |
| `util/syn_json_write.h` | `SYN_USE_JSON` | `SYN_JsonWriter`; `syn_json_init`, `syn_json_obj_open/close`, `syn_json_key_str/int/uint/bool` | Streaming JSON serializer with escaping and overflow tracking (`syn_json_ok`) |
| `util/syn_cbor_read.h` | `SYN_USE_CBOR` | `SYN_CborReader`; `syn_cbor_reader_init`, `syn_cbor_peek_type`, `syn_cbor_read_map_begin`, `syn_cbor_skip` | Streaming RFC 8949 CBOR decoder for maps, arrays, ints, floats, strings, bytes |
| `util/syn_cbor_write.h` | `SYN_USE_CBOR` | `SYN_CborWriter`; `syn_cbor_writer_init`, `syn_cbor_write_map_begin`, `syn_cbor_write_uint/text_cstr/bytes` | Definite-length RFC 8949 CBOR binary encoder |
| `util/syn_protobuf.h` | `SYN_USE_PROTOBUF` | `SYN_PB_Encoder`, `SYN_PB_Decoder`; `syn_pb_encode_uint32/string/bytes`, `syn_pb_decode_tag/varint`, `syn_pb_skip_field` | Protocol Buffers wire-format varint, fixed32/64, and length-delimited codec |
| `util/syn_base64.h` | `SYN_USE_BASE64` | `syn_base64_encode/decode`, `syn_base64url_encode/decode` | RFC 4648 Base64 and Base64URL codec |
| `util/syn_lz4.h` | `SYN_USE_LZ4` | `SYN_Lz4`; `syn_lz4_init`, `syn_lz4_compress`, `syn_lz4_decompress` | Zero-heap LZ4 raw block compressor and stateless decompressor |

```c
#include "syntropic/util/syn_cbor_write.h"

size_t encode_telemetry(uint8_t *buf, size_t cap, uint32_t mv, int32_t temp_c) {
    SYN_CborWriter w;
    syn_cbor_writer_init(&w, buf, cap);
    syn_cbor_write_map_begin(&w, 2);
    syn_cbor_write_text_cstr(&w, "mv");
    syn_cbor_write_uint(&w, mv);
    syn_cbor_write_text_cstr(&w, "tc");
    syn_cbor_write_int(&w, temp_c);
    return syn_cbor_writer_ok(&w) ? syn_cbor_writer_len(&w) : 0U;
}
```

---

## 5. Strings & `printf`-Free Formatting (`syn_str.h`, `syn_fmt.h`)

- **`util/syn_str.h`** *(always enabled)*: In-place delimiter splitting (`syn_str_split`), whitespace trimming (`syn_str_trim`), and overflow-checked integer parsing (`syn_str_to_i32`, `syn_str_to_u32`).
- **`util/syn_fmt.h`** (`SYN_USE_FMT`): Formats numbers and buffers without pulling in C library `snprintf`: `syn_fmt_int`, `syn_fmt_uint`, `syn_fmt_hex`, `syn_fmt_hex_parse`, `syn_fmt_q16`, `syn_fmt_fixed`, `syn_fmt_hexdump`, and `syn_fmt_concat`.

---

## 6. State Machines, Events, Pub/Sub & Concurrency

- **`util/syn_fsm.h`** (`SYN_USE_FSM`): Table-driven `SYN_FSM` supporting `SYN_FSM_Transition` tables (`from`, `event`, `to`, `SYN_FSM_Guard`, `SYN_FSM_Action`), per-state entry/exit hooks (`SYN_FSM_StateDesc`), and `syn_fsm_dispatch()`. See [Cross-Module Integration](integration.md) for pairing `syn_fsm` with `syn_ao` Active Objects.
- **`util/syn_pubsub.h`** (`SYN_USE_PUBSUB`): Topic-addressed publish/subscribe broker (`SYN_PubSubBroker`) over a caller-owned `SYN_PubSubSub` array (`syn_pubsub_init`, `syn_pubsub_subscribe`, `syn_pubsub_publish`, wildcard `SYN_PUBSUB_TOPIC_ALL`).
- **`util/syn_event.h`** (`SYN_USE_EVENT`) & **`util/syn_spinlock.h`** (`SYN_USE_MULTICORE`): 32-bit `SYN_EventGroup` (`PT_WAIT_EVENT`, `PT_WAIT_EVENT_ANY`) and scoped `SYN_SPINLOCK_GUARD(id)`. Full synchronization details are covered in [Cooperative Multitasking](multitasking.md).

---

## 7. Fixed-Point Math, Linear Algebra, 3D Transforms & Geodesy

- **`util/syn_qmath.h`** (`SYN_USE_QMATH`): `q16_t` (Q16.16), `q15_t` (Q0.15), and `q7_t` (Q0.7) fixed-point types, saturating arithmetic (`q16_add_sat`, `q16_sub_sat`, `q16_mul_sat`), interpolation (`q16_lerp`), integer square roots (`syn_isqrt32`, `syn_isqrt64`), transcendentals (`q16_sin`, `q16_cos`, `q16_sincos_fast`, `q16_atan2`, `q16_sqrt`, `q16_rsqrt`, `q16_exp`, `q16_log`, `q16_pow`), and string conversion (`q16_to_str`, `q16_from_str`).
- **`util/syn_vector.h`** *(always enabled)*: Elementwise `q16_t` vector ops (`syn_vec_add`, `syn_vec_sub`, `syn_vec_scale`, `syn_vec_clamp`) and statistical reductions (`syn_vec_min`, `syn_vec_max`, `syn_vec_mean`, `syn_vec_variance`, `syn_vec_rms`).
- **`util/syn_matrix.h`** (`SYN_USE_MATRIX`): Caller-backed `SYN_Matrix` (`SYN_MAT_DECL`, `SYN_MAT_STATIC`), 2D/3D homogeneous transforms, dot/cross products, and numerical solvers (`syn_matrix_solve_lu`, `syn_matrix_solve_cholesky`, `syn_matrix_least_squares`, `syn_matrix_qr`, `syn_matrix_eigen_sym2/3`).
- **`util/syn_quaternion.h`** *(always enabled)*: `SYN_Quaternion` 3D orientation math (`syn_quat_mul`, `syn_quat_normalize`, `syn_quat_rotate_vec3`, `syn_quat_from_euler`, `syn_quat_to_euler`, `syn_quat_to_mat3x3`, `syn_quat_slerp`).
- **`util/syn_transform.h`** *(always enabled)*: Q16.16 Cartesian <-> Polar (`syn_cart2pol`, `syn_pol2cart`) and Cartesian <-> Spherical (`syn_cart2sph`, `syn_sph2cart`) coordinate conversions.
- **`util/syn_geo.h`** (`SYN_USE_GEO`): WGS-84 <-> ECEF <-> local ENU transformations (`syn_geo_wgs84_to_enu`), Haversine and fixed-point micro-degree distances (`syn_geo_haversine_m`, `syn_geo_distance_fixed_mm`), and NMEA GGA position extraction (`syn_geo_pos_from_gga`).
- **`util/syn_lut.h`** *(header-only)*: Piecewise-linear 1D lookup table (`SYN_LUT_Entry`) with clamped forward (`syn_lut_forward`) and inverse (`syn_lut_reverse`) interpolation.
- **`util/syn_nn.h`** *(always enabled)*: Quantized `q7_t` TinyML layers (Dense, Conv1D, Attention, Pooling, Softmax, `syn_nn_dense_pt` / `syn_nn_conv1d_pt`). Covered in [DSP & Filters](dsp.md).

---

## 8. Motion Profiles & Signal Conditioning

- **`util/syn_ramp.h`** (`SYN_USE_RAMP`): Linear slew-rate (`syn_ramp_set_target`) and trapezoidal velocity/acceleration (`syn_ramp_set_target_trapezoid`, `syn_ramp_set_target_trapezoid_fp`) setpoint generators updated per tick via `syn_ramp_update()`.
- **`util/syn_scurve.h`** (`SYN_USE_SCURVE`): 7-phase jerk-limited 1D (`SYN_SCurve`) and coordinated 3-axis (`SYN_SCurve3D`, `syn_scurve3d_plan` / `syn_scurve3d_update`) trajectory generator bounding `v_max`, `a_max`, and `j_max`.
- **`util/syn_hysteresis.h`** *(header-only)*: Schmitt-trigger integer comparator (`SYN_Hysteresis`, `syn_hyst_init`, `syn_hyst_update`) switching at `threshold +/- band` to suppress relay/alarm chatter.
- **`util/syn_change_filter.h`** *(header-only)*: Report-on-change gate (`SYN_ChangeFilter`, `syn_change_filter_update`) requiring both `|delta| >= dead_band` and elapsed time `>= min_interval_ms`.

---

## 9. Timing, Retry Backoff, Rate Limiting, PRNG & Metrics

- **`util/syn_timeout.h`** *(header-only)*: 32-bit wrap-safe deadline tracker (`SYN_Timeout`: `syn_timeout_start`, `syn_timeout_expired`, `syn_timeout_remaining`, drift-free `syn_timeout_periodic`, and `PT_WAIT_TIMEOUT(pt, &to)`).
- **`util/syn_backoff.h`** *(always enabled)*: Exponential retry backoff (`SYN_Backoff`: `syn_backoff_init`, `syn_backoff_next_ms`, `syn_backoff_exhausted`, `syn_backoff_reset`) with 1.0x–1.5x random jitter.
- **`util/syn_rate_limit.h`** *(header-only)*: Token-bucket rate limiter (`SYN_RateLimit`: `syn_rate_limit_init(&rl, max_tokens, interval_ms)`, `syn_rate_limit_allow`).
- **`util/syn_random.h`** *(always enabled)*: `syn_random_fill`, `syn_random_u32`, and `syn_random_range(min, max)` delegating to platform `syn_port_random_fill()` (overrideable weak hook with xorshift test fallback).
- **`util/syn_metrics.h`** (`SYN_USE_METRICS`, opt-in default `0`): Intrusive static counter/gauge registry (`SYN_METRIC_DECLARE`, `SYN_METRIC_REGISTER`, `SYN_METRIC_INC/ADD/SET`, `syn_metrics_foreach`); compiles to zero-cost no-ops when `SYN_USE_METRICS` is `0`.
