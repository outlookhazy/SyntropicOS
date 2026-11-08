# API Reference Hub

Welcome to the SyntropicOS C API Reference. This directory provides direct access to functional module subsystems, Doxygen-annotated group references, and global type indexes.

---

## Technical Architecture & Memory Guarantees

| Property | Guarantee |
|---|---|
| **Standards Compliance** | Pure C99, compatible with GCC, Clang, IAR, Keil, and MSVC. |
| **Heap Allocation** | **0 Bytes**. No `malloc()`, `free()`, or dynamic memory pools. |
| **Concurrency Model** | Cooperative stackless coroutines (`syn_pt`). RAM footprint = **2 bytes/thread**. |
| **Reentrancy & Safety** | Lock-free SPSC data structures for safe ISR ↔ Task communication. |

---

## Subsystem Navigation Directory

Jump directly to module documentation guides and Doxygen API groups:

| Subsystem | Functional Scope | Documentation Guide |
|---|---|---|
| **⚡ Core & Multitasking** | Protothreads, Task Scheduler, Active Objects, Workqueues, Mailboxes, Event Flags. | [Multitasking Guide](../modules/multitasking.md) |
| **🎛️ Drivers & Peripherals** | GPIO, EXTI, UART, CAN, SPI, I2C, ADC, DAC, SD Card, RTC, DMA Engine. | [Drivers Guide](../modules/drivers.md) |
| **🔘 Input / Output** | Debounced Buttons, Tap Gestures, Combos, Rotary Encoders, LEDs, Soft PWM. | [I/O Guide](../modules/io.md) |
| **📡 Communications** | COBS Framing, Addressed Packet Router, Modbus RTU/TCP, M-Bus, ISO-TP, J1939. | [Communication Guide](../modules/communication.md) |
| **⚙️ Motor & Control** | Integer PID Controller, Auto-Tuning, Steppers, Servos, DC Motors, FOC & Observer. | [Control Guide](../modules/control.md) |
| **🖥️ Display & UI** | Framebuffer Canvas, Graphics Primitives, Immediate-Mode GUI (`syn_imgui`), Menus. | [Display Guide](../modules/display.md) |
| **💾 Storage** | Persistent Settings Manager, Wear-Leveled Flash, LittleFS, FAT32, Virtual VFS. | [Storage Guide](../modules/storage.md) |
| **🔬 Diagnostics & Debug** | ISR Event Tracer (`syn_trace`), CPU Profiler (`syn_profiler`), Serial CLI, Watchdog. | [Debug Guide](../modules/debug.md) |
| **🛡️ System & Resilience** | Boot Manager, Crash-Loop Recovery, Core Dump Flash Capture, OTA Updates. | [System Guide](../modules/system.md) |
| **📐 DSP & Signal Processing** | Biquad Butterworth IIR Filters, EMA, Median, Radix-2 FFT, Signal Statistics. | [DSP Guide](../modules/dsp.md) |
| **🔒 Cryptography** | BLAKE2s Hash / MAC, ChaCha20-Poly1305 AEAD Cipher, X25519 Key Exchange. | [Crypto Guide](../modules/crypto.md) |

---

## Generating Doxygen HTML Reference

Generate full source-file trees (`files.html`), data structure indexes (`annotated.html`), and module API pages under `build/doxygen/html/`:

```bash
make container-dox
```
