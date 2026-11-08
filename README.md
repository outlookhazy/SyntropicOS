<p align="center">
  <img src="docs/assets/banner.png" alt="SyntropicOS Banner" width="700"/>
</p>

# SyntropicOS

**High-Performance Bare-Metal Application Framework & Cooperative OS**

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![C99](https://img.shields.io/badge/C-C99-blue.svg)]()
[![Build & Test](https://github.com/outlookhazy/SyntropicOS/actions/workflows/ci.yml/badge.svg)](https://github.com/outlookhazy/SyntropicOS/actions)

SyntropicOS is a zero-overhead, production-grade C99 framework designed for deeply embedded microcontrollers (STM32, RP2040, ESP32, AVR, RISC-V). It combines stackless coroutines, non-blocking hardware drivers, industrial fieldbuses, fixed-point DSP, and display graphics into a single cooperative ecosystem.

---

## Why SyntropicOS? (Design Rationale & Philosophy)

Deeply embedded systems present a fundamental architectural choice:

1. **Bare-Metal Super-Loops (`while(1)`)**: Simple to start and zero memory overhead. However, managing timers, state machines, protocol decoders, and non-blocking I/O in a single main loop quickly results in unmaintainable code. Any blocking call halts the entire microcontroller.
2. **Preemptive RTOS (FreeRTOS, Zephyr)**: Multi-tasking where every task requires an allocated stack (512 B to 4 KB RAM per task). On microcontrollers with 2 KB to 16 KB total RAM, allocating thread stacks severely constrains memory. Preemptive switches also introduce race conditions, mutex contention, context-switch overhead, and potential stack overflows.
3. **The SyntropicOS Approach**: A cooperative OS built around **stackless coroutines (protothreads `syn_pt`)**. Tasks execute sequentially on a single system stack while syntax macros handle yielding and resumption. Continuation state costs **2 bytes of RAM per thread** (`uint16_t lc`). Memory allocation is **100% static** (zero `malloc()`), preventing runtime heap fragmentation.

SyntropicOS is not the first system to use stackless coroutines — Contiki OS (created by Adam Dunkels, who invented protothreads) popularized them for microcontrollers. The difference is that **every SyntropicOS module is built from the ground up with the cooperative model in mind**. All 70+ drivers, protocol stacks, and subsystems expose non-blocking `_poll()`, `_update()`, or `_process()` APIs that yield cooperatively. There are no blocking wrappers around third-party libraries or hidden busy-waits inside module code.

---

## Architecture & Framework Comparison

| Feature | Bare-Metal Super-Loop | Preemptive RTOS (FreeRTOS / Zephyr) | SyntropicOS |
|---|---|---|---|
| **RAM per Thread** | 0 B | 512 B – 4 KB per task (stack pointer) | **2 B per thread** (`uint16_t lc`) |
| **Concurrency Model** | Manual state machines in main loop | Preemptive multi-threading | Cooperative protothreads (`syn_pt`) |
| **Memory Allocation** | Static | Dynamic heap or static pool allocation | **100% Zero-Heap / Static** |
| **Context Switch Cost** | Zero | CPU register push/pop & stack frame swap | **Zero** (C99 `switch` continuation jump) |
| **Race Conditions** | None (single thread execution) | High (requires mutexes, semaphores, spinlocks) | None across yield points |
| **Execution Safety** | Low (blocking functions freeze system) | Stack overflow risk | High (no thread stack overflow risk) |
| **Target Hardware** | 8-bit to 32-bit MCUs | 32-bit MCUs (typically >32 KB RAM) | **8-bit to 32-bit MCUs (2 KB+ RAM)** |

---

## Core Concepts Explained

### 1. Stackless Protothreads (`syn_pt`)
Protothreads provide sequential, non-blocking flow control inside standard C functions without requiring separate stacks.
- **Continuation & Explicit Yield Intent**: `PT_BEGIN()` expands to a `switch(pt->lc)` jump table. Yield macros record `__LINE__` in `pt->lc` and return explicit scheduling intent: `PT_PASS()` / `PT_WAIT_UNTIL()` return `PT_IDLE` (no immediate work left this round), while `PT_CONTINUE()` / `PT_CONTINUE_IF_NEEDED()` return `PT_MORE` (chunk finished, more work waiting).
- **RAM Footprint**: Stores only a `uint16_t` continuation variable (2 bytes RAM).
- **Variable Lifetime**: Local variables inside a protothread function do not persist across yields. Persistent state must be stored in `static` variables, global contexts, or a struct passed via `user_data`.

### 2. Cooperative Task Scheduler (`syn_sched`)
The scheduler dispatches one eligible `SYN_Task` per `syn_sched_run()` call using ranked selection (`Overdue -> Work -> Poll -> Round Reset`) ordered by `SYN_Prio` (`SYN_PRIO_CRITICAL = 0` .. `SYN_PRIO_LOW = 3`) and per-priority round-robin.
- **Bounded Starvation Windows**: Configurable per-priority wait bounds (`1 / 5 / 15 / 50 ms` defaults via `syn_sched_set_max_wait()`) prevent active high-priority work from starving lower-priority tasks indefinitely.
- **Zero Dynamic Allocation**: The application owns and allocates the `SYN_Task` array statically.
- **Tickless Idle & Native Blocking**: Includes `PT_BLOCK_*` event blocking (`SYN_TASK_BLOCKED`) and low-power sleep (`syn_sched_run_tickless()`, or `syn_sched_run_tickless_ex()` when `SYN_Timer` deadlines must also wake the CPU) when no tasks are ready.

### 3. Ground-Up Non-Blocking Module Ecosystem
Every driver and protocol module in SyntropicOS is written from scratch as a cooperative, non-blocking state machine. Modules expose `_poll()`, `_update()`, or `_process()` entry points (e.g. `syn_modbus_poll()`, `syn_button_update()`, `syn_ble_gatt_process_att_pdu()`) that do a bounded unit of work and return immediately. No module internally blocks, busy-waits, or calls `delay()`.

---

## Technical Specifications At-a-Glance

| Property | Design Specification |
|---|---|
| **Concurrency** | Cooperative stackless coroutines (`syn_pt`). Continuation state costs **2 bytes RAM** per thread. |
| **Scheduler** | Cooperative task runner (`syn_sched`). Task descriptors (`SYN_Task`) cost **44 bytes RAM** per task on 32-bit targets (80 bytes on 64-bit targets; standalone `SYN_PT` is **2 bytes**). |
| **Memory Allocation** | **100% Zero-Heap / Static Allocation**. No `malloc()` or dynamic pool fragmentation over long runtimes. |
| **Execution Model** | All 70+ drivers & protocol stacks are written as **non-blocking state machines**. |
| **Compatibility** | Standard **C99**. Compiles with GCC, Clang, IAR, Keil, STM32CubeIDE, and Arduino IDE. |

---

## System Architecture

> [!NOTE]
> *If the Mermaid architecture diagram below fails to render on first load, please refresh your browser (GitHub's client-side Mermaid renderer occasionally takes a coffee break).*

```mermaid
flowchart TD
    App["Application Logic & Callbacks"] --> Modules["SyntropicOS Non-Blocking State Machine Modules"]
    
    subgraph Modules
        IO["Input/Output (Buttons, Encoders, LEDs)"]
        Comm["Communication (COBS, Router, Modbus, DALI, BACnet, M-Bus)"]
        Storage["Storage (Settings, LittleFS, VFS)"]
        Display["Display (Canvas 1bpp/16bpp, IMGUI)"]
        DSP["DSP & Control (PID, Biquad, FFT, FOC, TinyML)"]

    end
    
    Modules --> Kernel["Cooperative Kernel (syn_pt + syn_sched)"]
    Kernel --> HAL["Hardware Port Layer (syn_port_*)"]
    HAL --> Hardware["Microcontroller Hardware (STM32, RP2040, ESP32, AVR)"]
```

---

## Quick Navigation & Documentation Index

- 📖 **[Documentation Hub](https://outlookhazy.github.io/SyntropicOS/)** — Complete online documentation & API reference.
- 🚀 **[Getting Started Guide](https://outlookhazy.github.io/SyntropicOS/getting-started/)** — CMake, Makefile, and C99 bare-metal setup.
- 🛠️ **[IDE Integration Guides](https://outlookhazy.github.io/SyntropicOS/ide-guides/)** — STM32CubeIDE, VS Code, Keil MDK, IAR, and Arduino IDE setup.
- 🔌 **[Arduino Compatibility Guide](https://outlookhazy.github.io/SyntropicOS/arduino/)** — Arduino Library Manager installation & IDE setup.
- 🔧 **[MCU Porting Guide](https://outlookhazy.github.io/SyntropicOS/porting-guide/)** — Implementing custom HAL ports (`syn_port_*`).
- 🧪 **[Testing & Containerization Guide](https://outlookhazy.github.io/SyntropicOS/testing/)** — Unity unit tests, QEMU emulation, sanitizers, and integration daemons.


### Module Guides
- ⚡ **[Core & Multitasking](https://outlookhazy.github.io/SyntropicOS/modules/multitasking/)** — Protothreads, Task Scheduler, Active Objects, Workqueues.
- 🎛️ **[Input / Output](https://outlookhazy.github.io/SyntropicOS/modules/io/)** — Debounced Buttons, Tap Gestures, Combos, Rotary Encoders, LEDs, Soft PWM.
- 📡 **[Communication Protocols](https://outlookhazy.github.io/SyntropicOS/modules/communication/)** — COBS Framing, Addressed Router, Modbus RTU/TCP, DALI, BACnet MS/TP, M-Bus, ISO-TP, J1939, NMEA 2000, CCP v2.1, ASAM XCP v1.x, UDS (ISO 14229), ODVA DeviceNet.
- 💾 **[Storage & Filesystems](https://outlookhazy.github.io/SyntropicOS/modules/storage/)** — Persistent Settings Manager, Wear-Leveled Flash, LittleFS, FAT.
- 🖥️ **[Display & Embedded UI](https://outlookhazy.github.io/SyntropicOS/modules/display/)** — Framebuffer Canvas, 2D Graphics, Zero-Heap IMGUI.
- 🔬 **[Diagnostics & Debug](https://outlookhazy.github.io/SyntropicOS/modules/debug/)** — Lightweight Event Tracer (`syn_trace`), Task CPU Profiler (`syn_profiler`), Serial CLI.


---

## Minimal Example

```c
#include "syntropic/syntropic.h"

#define LED_PIN 13

static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task) {
    PT_BEGIN(pt);
    for (;;) {
        syn_gpio_toggle(LED_PIN);
        PT_TASK_DELAY_MS(pt, task, 500); // Non-blocking 500ms delay
    }
    PT_END(pt);
}

int main(void) {
    syn_gpio_init(LED_PIN, SYN_GPIO_OUTPUT);
    
    static SYN_Task tasks[1];
    static SYN_Sched sched;
    
    syn_task_create(&tasks[0], "blink", blink_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&sched, tasks, 1);
    syn_sched_run_forever(&sched);
}
```

---

## Example Projects Directory ([`examples/`](examples/))

SyntropicOS includes over 50 complete hardware and SDK examples across bare-metal C, STM32 HAL, PlatformIO, and Arduino. See **[`examples/README.md`](examples/README.md)** for the full categorized directory.

### Featured Example Highlights
- **Industrial Automation**: **[`examples/stm32_modbus_tcp`](examples/stm32_modbus_tcp)** — Dual Modbus TCP Server (port 502) & Client running in a single project.
- **Motion Control & EtherCAT**: **[`examples/stm32_ethercat_servo`](examples/stm32_ethercat_servo)** — EtherCAT Slave node & CiA 402 drive control.
- **Automotive Fieldbus**: **[`examples/stm32_canopen`](examples/stm32_canopen)** — CANopen (CiA 301) SDO/PDO dictionary node.
- **Smart Energy**: **[`examples/stm32_mbus_meter`](examples/stm32_mbus_meter)** — M-Bus (EN 13757) utility meter reader.
- **Power Management**: **[`examples/stm32_pmbus_power`](examples/stm32_pmbus_power)** — PMBus 1.2/1.3 digital power telemetry & Linear11/16 decoder.
- **Multi-Task Protothreads (`SYN_PT`)**: **[`examples/stm32_multitask_demo`](examples/stm32_multitask_demo)** — Integrated 4-task bare-metal STM32 demo (LED, USART CLI, Button Gestures, Global IPC).
- **Embedded Shell & UI**: **[`examples/stm32_cli_shell`](examples/stm32_cli_shell)** — Interactive USART CLI shell (`led`, `status`, `temp`).
- **Rotary Input & Debounce**: **[`examples/stm32_encoder_button`](examples/stm32_encoder_button)** — EC11 rotary encoder & push-button gesture controller.
- **Closed-Loop Control**: **[`examples/PID_TempControl`](examples/PID_TempControl)** — Non-blocking integer PID temperature controller.

👉 *Explore all 50+ example projects in the **[Examples Directory](examples/README.md)**.*


---

## Containerized Verification Workflow

```bash
make test         # Run Unity unit test suite (2000+ unit tests passing)
make san          # Execute AddressSanitizer & UBSan memory safety audit
make qemu         # Bare-metal ARM Cortex-M4 boot emulation
make fuzz         # Run LLVM libFuzzer protocol targets (COBS, Modbus, MQTT, HTTP)
make cov          # Generates LCOV HTML code coverage reports
make static       # Run Cppcheck and Clang scan-build static analysis
make dox          # Build Doxygen API documentation (0 warnings tolerance)
make integration  # Run E2E tests against 8 genuine production container daemons
```
