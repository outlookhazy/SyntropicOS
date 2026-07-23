# SyntropicOS

**High-Performance Bare-Metal Application Framework & Cooperative OS**

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![C99](https://img.shields.io/badge/C-C99-blue.svg)]()

SyntropicOS is a zero-overhead, production-grade C99 framework designed for deeply embedded systems. Built for professional toolchains (GCC, Clang, IAR, Keil), it provides a modular ecosystem for building structured bare-metal software on high-performance targets like STM32 and RP2040.

> **🚀 Development Status:** SyntropicOS is in active development. While core modules are tested and verified, the API is evolving. Feedback and contributions from the embedded community are welcome.

## Features

- **Stackless cooperative threads** — Protothread coroutines (2 bytes RAM continuation; ~16–28 bytes per scheduled task)
- **Zero heap allocation** — All state is caller-owned or static
- **60+ native modules** — Drivers, DSP, control loops, networking, graphics, CLI, logging, and more
- **Cooperative-by-design** — All modules use non-blocking state machines built specifically for coroutines
- **Pure C99** — GCC, Clang, Keil, IAR — any C99 compiler
- **Integer-only math** — No floating point, no `libm.a`
- **Pay for what you use** — Every module toggleable via `#define`

## Architectural Philosophy: Why SyntropicOS?

Many embedded frameworks try to combine a coroutine scheduler with third-party libraries (e.g. LWIP, MQTT, micro-DSP). However, standard third-party libraries rely on **blocking loops** (`while (!ready) delay();`) or require **heavy preemptive threads with multi-kilobyte stacks** (e.g., 512B–4KB stack per task in FreeRTOS or Zephyr). Wrapping these libraries in a cooperative scheduler creates a severe impedance mismatch: opaque internal blocking calls starve other tasks and destroy non-preemptive concurrency.

SyntropicOS takes a **batteries-included, cooperative-by-design** approach:

1. **Native Non-Blocking State Machines**: All 60+ modules — including networking (MQTT, HTTP, WebSocket, DNS, SNTP), DSP (FFT, Biquad IIR, PID), protocols (NMEA, Modbus), and GUI engine — are built natively from the ground up as explicit state machines. During wait states (network handshakes, hardware I/O, sensor sampling), modules defer control back to the scheduler (`PT_DEFER` / `PT_WAIT_UNTIL`) rather than blocking CPU execution.
2. **Zero Heap Allocation**: 100% of state is caller-owned or statically allocated (`SYN_MAILBOX_DEFINE`, fixed Q16.16 math). Eliminates dynamic allocation (`malloc`/`free`), fragmentation, and memory leaks over indefinite runtimes.
3. **Stackless & Lightweight Footprint**: Tasks run as stackless coroutines — bare protothreads cost just **2 bytes of RAM** (`uint16_t lc`), while full scheduled tasks with priority, timer delay, and event blocking cost only **~16–28 bytes per task descriptor** (compared to 1KB+ stack per thread in traditional RTOSs). The entire OS, networking stack, and DSP pipeline execute deterministically on small 8-bit MCUs (AVR Mega 8KB RAM) up to dual-core 32-bit platforms (RP2040, STM32).
4. **Ecosystem Harmonization**: Every module shares the exact same concurrency model, timer ticks, logging primitives (`SYN_LOG`), and error handling — eliminating competing event loops or conflicting threading paradigms.

## Quick Start

### Git Submodule (CMake, Makefile, IDE)

```bash
git submodule add https://github.com/outlookhazy/SyntropicOS lib/SyntropicOS
```

```cmake
add_subdirectory(lib/SyntropicOS)
target_link_libraries(your_target PRIVATE syntropic)
```

### Arduino Compatibility

SyntropicOS is fully compatible with the Arduino ecosystem, providing a professional upgrade path for complex projects that require multitasking and structured drivers without leaving the Arduino IDE.

**Installation:**
- **Recommended:** Search for "SyntropicOS" in the **Arduino Library Manager** (`Ctrl+Shift+I`) and install it directly. No manual ZIP downloads or cloning required.
- **Manual:** Add as a git submodule or download the ZIP from GitHub if you need a specific development branch.

Then explore the included examples via **File → Examples → SyntropicOS**.

### Minimal Example

```c
#include "syntropic/syntropic.h"

static SYN_PT_Status blink(SYN_PT *pt, SYN_Task *task) {
    PT_BEGIN(pt);
    for (;;) {
        syn_gpio_toggle(LED_PIN);
        PT_TASK_DELAY_MS(pt, task, 500);
    }
    PT_END(pt);
}

int main(void) {
    syn_gpio_init(LED_PIN, SYN_GPIO_OUTPUT);
    static SYN_Task tasks[1];
    static SYN_Sched sched;
    syn_task_create(&tasks[0], "blink", blink, 0, NULL);
    syn_sched_init(&sched, tasks, 1);
    syn_sched_run_forever(&sched);
}
```

## Documentation

📖 **[Read the full documentation →](https://outlookhazy.github.io/SyntropicOS/)**

- [Getting Started](https://outlookhazy.github.io/SyntropicOS/getting-started/)
- [Module Reference](https://outlookhazy.github.io/SyntropicOS/modules/core/)
- [Porting Guide](https://outlookhazy.github.io/SyntropicOS/porting-guide/)
- [Testing](https://outlookhazy.github.io/SyntropicOS/testing/)

## Supported Platforms

Reference port implementations included for:

- **STM32F4** (bare-metal, direct register access)
- **STM32** (HAL, cross-family)
- **ESP32** (ESP-IDF)
- **RP2040/RP2350** (Pico SDK)
- **Arduino** (C++ SDK)

## License

Apache 2.0 — see [LICENSE](LICENSE).
