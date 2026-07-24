# SyntropicOS

**High-Performance Bare-Metal Application Framework & Cooperative OS**

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![C99](https://img.shields.io/badge/C-C99-blue.svg)]()

SyntropicOS is a zero-overhead, production-grade C99 framework designed for deeply embedded systems. Built for professional toolchains (GCC, Clang, IAR, Keil), it provides a modular ecosystem for building structured bare-metal software on high-performance targets like STM32 and RP2040.

> **🚀 Development Status:** SyntropicOS is in active development. While core modules are tested and verified, the API is evolving. Feedback and contributions from the embedded community are welcome.

## Features

- **Stackless cooperative threads** — Protothread coroutines (2 bytes RAM continuation; ~16–28 bytes per scheduled task)
- **Zero heap allocation** — All state is caller-owned or static
- **70+ native modules** — Drivers, DSP, fixed-point math, Kalman filter, FOC motor control, networking, graphics, CLI, logging, and more
- **Industrial & Commercial Vehicle Protocols** — Complete SAE J1939 (heavy machinery/BMS), CANopen (CiA 401/402/LSS), Modbus RTU/TCP, NMEA 0183
- **Hardware Peripherals & DMA Safety** — Bare-metal safe DMA transaction engine with address alignment verification, D-cache coherency invalidation, and atomic busy protection
- **Cooperative-by-design** — All modules use non-blocking state machines built specifically for coroutines
- **Pure C99** — GCC, Clang, Keil, IAR — any C99 compiler
- **Integer-only math** — Full Q16.16 fixed-point library: trigonometry, sqrt, exp/log, matrix algebra, and string I/O — no floating point, no `libm.a`
- **Pay for what you use** — Every module toggleable via `#define`

## Architectural Philosophy: Why SyntropicOS?

Many embedded frameworks try to combine a coroutine scheduler with third-party libraries (e.g. LWIP, MQTT, micro-DSP). However, standard third-party libraries rely on **blocking loops** (`while (!ready) delay();`) or require **heavy preemptive threads with multi-kilobyte stacks** (e.g., 512B–4KB stack per task in FreeRTOS or Zephyr). Wrapping these libraries in a cooperative scheduler creates a severe impedance mismatch: opaque internal blocking calls starve other tasks and destroy non-preemptive concurrency.

SyntropicOS takes a **batteries-included, cooperative-by-design** approach:

1. **Native Non-Blocking State Machines**: All 70+ modules — including networking (MQTT, HTTP, WebSocket, DNS, SNTP), DSP (FFT, Biquad IIR, Kalman filter), motor control (PID, FOC, SVPWM), protocols (NMEA, Modbus), and GUI engine — are built natively from the ground up as explicit state machines. During wait states (network handshakes, hardware I/O, sensor sampling), modules defer control back to the scheduler (`PT_DEFER` / `PT_WAIT_UNTIL`) rather than blocking CPU execution.
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

## Fixed-Point Math & DSP

SyntropicOS includes a comprehensive Q16.16 fixed-point math library — no floating point, no `libm.a`, no heap allocation. All operations use `int64_t` intermediates for full precision.

### Q16.16 Math (`syn_qmath`)
- **Arithmetic**: add, sub, mul, div, abs, lerp, clamp, saturating variants
- **Trigonometry**: sin, cos, tan, atan2, asin, acos
- **Algebraic**: sqrt, hypot
- **Exponential**: exp, log, pow
- **String I/O**: `q16_to_str` / `q16_from_str` — zero-allocation decimal formatting

### Matrix Algebra (`syn_matrix`)
- **Compile-time-arbitrary dimensions** — declare any size via `SYN_MAT_DECL(name, rows, cols)`, compiler constant-folds loop bounds
- **Operations**: multiply, transpose, determinant, inverse (2×2, 3×3, 4×4), trace
- **Linear Solvers**: LU decomposition with partial pivoting (`syn_matrix_solve_lu`), Cholesky decomposition (`syn_matrix_solve_cholesky`), overdetermined least-squares (`syn_matrix_least_squares`)
- **Transforms**: 2D/3D rotation, translation, scaling (homogeneous matrices)
- **Vector**: dot product, cross product, normalize
- **Zero heap**: all storage is caller-owned stack/static arrays

### DSP
- **Biquad IIR filters**: lowpass, highpass, bandpass, notch — Butterworth design with configurable Q
- **FFT**: fixed-point radix-2 with cooperative protothread support
- **Signal statistics**: sliding-window min/max/mean/variance/RMS/stddev
- **Kalman filter** (`syn_kalman`): general-purpose, arbitrary-dimension state estimation using `syn_matrix`

### Field-Oriented Control (`syn_foc`)
- **Clarke transform**: 3-phase (a,b,c) → 2-phase stationary (α,β)
- **Park transform**: stationary (α,β) → rotating (d,q) frame
- **Inverse transforms** for voltage command generation
- **SVPWM**: space-vector PWM duty cycle computation

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
