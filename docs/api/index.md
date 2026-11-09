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
| **📚 Overview & Catalog** | Complete directory of all 278 portable C99 module headers and `SYN_USE_*` switches. | [Module Catalog](../modules/index.md) |
| **🧱 Core & Utilities** | Fixed-point & matrix math, CRCs, CBOR/JSON/Protobuf/Base64 codecs, lock-free queues, pools, FSM, pub/sub. | [Core & Utilities Guide](../modules/core.md) |
| **⚡ Multitasking** | Protothreads, Task Scheduler, Active Objects, Workqueues, Mailboxes, Event Flags, Multicore. | [Multitasking Guide](../modules/multitasking.md) |
| **🎛️ Drivers & Sensors** | GPIO, EXTI, UART, CAN, SPI, I2C, ADC, DAC, PWM/HRPWM, DMA, RTC/PPS, SD, USB 2.0, Sensors. | [Drivers Guide](../modules/drivers.md) |
| **🛠️ Services** | Interactive CLI shell, leveled logger, binary ring datalogger, flight blackbox recorder. | [Services Guide](../modules/services.md) |
| **🔘 Input / Output** | Debounced Buttons, Tap Gestures, Combos, Rotary Encoders, LEDs, WS2812, Soft PWM, DShot. | [I/O Guide](../modules/io.md) |
| **🖥️ Display & UI** | Framebuffer Canvas, Graphics Primitives, Immediate-Mode GUI (`syn_imgui`), Menus, OLED/LCD. | [Display Guide](../modules/display.md) |
| **⚙️ Motor & Control** | Integer & Flight PID, Auto-Tuning, Steppers, Servos, DC/BLDC Motors, FOC, G-code CNC. | [Control Guide](../modules/control.md) |
| **📐 DSP & Signal Processing** | Biquad IIR & Filter Synthesis, FIR, EMA, Median, Radix-2 FFT, DDS, Goertzel, MFCC, Kalman. | [DSP Guide](../modules/dsp.md) |
| **📡 Communications** | Ethernet/IPv4/TCP/UDP, TLS/DTLS 1.3, WireGuard, HTTP/MQTT/CoAP/LwM2M, CANopen, Modbus, UDS. | [Communication Guide](../modules/communication.md) |
| **📶 Bluetooth Low Energy** | Zero-heap BLE Host stack over H:4 UART/SPI (HCI, L2CAP, ATT, GAP, GATT Server). | [BLE Guide](../modules/ble.md) |
| **🔊 Audio & Codecs** | Non-blocking I2S/DMA PCM playback, multi-voice mixer, IMA ADPCM, Bluetooth SBC, WAV, VAD. | [Audio Guide](../modules/audio.md) |
| **🔒 Cryptography & PKI** | AES-128/256, ChaCha20-Poly1305, SHA-256/512, BLAKE2s, X25519, Ed25519, P-256, X.509, TPM 2.0. | [Crypto Guide](../modules/crypto.md) |
| **💾 Storage** | Parameter Store, Wear-Leveled Flash Settings, LittleFS, FAT16/FAT32, Virtual VFS. | [Storage Guide](../modules/storage.md) |
| **🛡️ System & Power** | Boot Manager, A/B OTA & Secure Boot, Coredump Capture, Power Modes, Watchdogs. | [System Guide](../modules/system.md) |
| **🧩 WebAssembly VM** | Cooperative fuel-metered 32-bit WebAssembly bytecode interpreter (experimental). | [Wasm VM Guide](../modules/wasm.md) |
| **🔬 Diagnostics & Debug** | ISR Event Tracer (`syn_trace`), CPU Profiler (`syn_profiler`), Runtime Metrics. | [Debug Guide](../modules/debug.md) |
| **🔗 Integration Patterns** | End-to-end pipelines combining scheduler, drivers, DSP, storage, and protocols. | [Integration Guide](../modules/integration.md) |

---

## Generating Doxygen HTML Reference

Generate full source-file trees (`files.html`), data structure indexes (`annotated.html`), and module API pages under `build/doxygen/html/`:

```bash
make container-dox
```
