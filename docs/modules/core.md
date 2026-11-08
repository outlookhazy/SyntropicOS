# Core & Utility Modules

SyntropicOS provides header-only foundation macros, ISR-safe data structures, binary structure packing, and fixed-point math utilities.

---

## Technical Specifications

| Feature | Specification |
|---|---|
| **RAM Cost** | Zero heap allocation. All structures (`SYN_RingBuf`, `SYN_SPSC_Queue`, `SYN_Stream`, `SYN_PingPong`) are caller-owned. |
| **ISR Safety** | SPSC (Single-Producer / Single-Consumer) ring buffers and queues are **lock-free and ISR-safe**. |
| **Asserts** | Runtime checks strippable in production via `#define SYN_DISABLE_ASSERT`. |


---

## 1. Cooperative Byte Stream (`util/syn_stream.h`)

`SYN_Stream` wraps an SPSC ring buffer with protothread-aware readability conditions. It supports 3 operational modes:
- **Default Mode**: De-blocks when any byte is available.
- **Threshold Mode**: De-blocks when ≥ N bytes arrive (packet-oriented protocols).
- **Delimiter Mode**: De-blocks when a specific delimiter byte (e.g. `\n` or `0x00`) arrives. If an oversized frame fills the buffer without a delimiter, `SYN_Stream` automatically flushes the corrupted frame, enters a resync state, and discards trailing bytes until the delimiter arrives. This guarantees protothreads on `PT_STREAM_WAIT` wake ONLY on 100% complete, un-truncated frames.

### Stream Data Flow

```mermaid
flowchart LR
    ISR["UART ISR / DMA"] -->|syn_stream_put| RingBuffer["Lock-Free Ring Buffer"]
    RingBuffer -->|PT_STREAM_WAIT| Protothread["Protothread Task"]
    Protothread -->|Read Line / Frame| Processing["Process Message"]
```

### Complete Code Example (Line-Oriented Serial Stream)

```c
#include <syntropic/pt/syn_pt.h>
#include <syntropic/sched/syn_task.h>
#include <syntropic/util/syn_stream.h>

static uint8_t raw_buffer[128];
static SYN_Stream rx_stream;

void stream_setup(void) {
    syn_stream_init(&rx_stream, raw_buffer, sizeof(raw_buffer));

    // De-block task only when a newline '\n' character is received
    syn_stream_set_delimiter(&rx_stream, '\n');
}

// ISR Producer: Lock-free put
void USART1_IRQHandler(void) {
    uint8_t byte = (uint8_t)USART1->DR;
    syn_stream_put(&rx_stream, byte);
}

// Consumer Protothread: Reads complete lines
SYN_PT_Status line_consumer_task(SYN_PT *pt, SYN_Task *task) {
    static uint8_t line_buf[64];
    static size_t bytes_read;
    (void)task;

    PT_BEGIN(pt);
    for (;;) {
        // Yields CPU until '\n' delimiter arrives in rx_stream
        PT_STREAM_WAIT(pt, &rx_stream);

        bytes_read = syn_stream_read_line(&rx_stream, line_buf, sizeof(line_buf));
        process_cmd((char *)line_buf, bytes_read);
    }
    PT_END(pt);
}
```

---

## 2. Binary Serialization & Structure Packing (`util/syn_pack.h`)

Provides endianness-independent serialization (`syn_pack_u16`, `syn_pack_u32_le`, `syn_unpack_u16`, `syn_peek_u16`, `syn_poke_u16`, etc.) to prevent unaligned memory access traps on Cortex-M0/M3 microcontrollers.

```c
#include <syntropic/util/syn_pack.h>

void serialize_packet(uint8_t *dest, uint16_t id, uint32_t val) {
    size_t pos = 0;
    syn_pack_u16(dest, &pos, id);   // Big-endian uint16_t (auto-advances pos)
    syn_pack_u32(dest, &pos, val);  // Big-endian uint32_t (use _le suffix for little-endian)
}
```

---

## 3. Lock-Free SPSC Ring Buffer & Queue (`util/syn_spsc_queue.h` & `util/syn_ringbuf.h`)

SyntropicOS provides single-producer single-consumer lock-free ring buffers for ISR-to-thread communication.

### Usage Example
```c
#include <syntropic/util/syn_spsc_queue.h>

static uint8_t queue_buf[256];
static SYN_SPSC_Queue rx_queue;

void setup_queue(void) {
    syn_spsc_queue_init(&rx_queue, queue_buf, sizeof(queue_buf[0]),
                        sizeof(queue_buf) / sizeof(queue_buf[0]));
}

// In USART RX ISR:
void USART1_IRQHandler(void) {
    uint8_t byte = (uint8_t)USART1->DR;
    syn_spsc_queue_push(&rx_queue, &byte);
}

// In main task loop:
void process_rx(void) {
    uint8_t byte;
    while (syn_spsc_queue_pop(&rx_queue, &byte) == SYN_OK) {
        // Feed protocol state machine (Modbus, COBS, BACnet, DALI)
        syn_modbus_feed(&mb_slave, byte);
    }
}
```

