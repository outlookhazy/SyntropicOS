# API Reference

Welcome to the SyntropicOS C API Reference. This section contains the auto-generated API documentation extracted directly from the Doxygen-annotated source code.

## Subsystems

The API is organized into functional modules to help you find the relevant function signatures, structures, macros, and configuration options:

| Module | Description | API Link |
|--------|-------------|----------|
| **Core & Utilities** | Status codes, compiler attributes, assertions, ring buffers, CRC, Q16.16 fixed-point math, matrix algebra, lookups, and timeouts. | [Core API](../syntropic/group__syn__core.md) |
| **Scheduling** | Protothreads, cooperative scheduler, tasks, timers, watchdogs, sequencers, active objects, and mailboxes. | [Scheduling API](../syntropic/group__syn__sched.md) |
| **Drivers** | Hardware peripheral driver interfaces (GPIO, UART, ADC, DAC, CAN, EXTI, RTC, soft-protocols). | [Drivers API](../syntropic/group__syn__drivers.md) |
| **Input / Output** | Physical user interface components (Buttons, encoders, LEDs, sensors). | [I/O API](../syntropic/group__syn__io.md) |
| **Motor & Control** | DC motors, servos, steppers, actuators, FOC (Clarke/Park/SVPWM), and PID control algorithms. | [Motor API](../syntropic/group__syn__motor.md) |
| **Display & UI** | Graphics primitive rendering, canvas, menus, and immediate-mode GUI components. | [Display API](../syntropic/group__syn__display.md) |
| **Networking** | Packet router, transports, DNS, HTTP, WebSocket, CoAP, and MQTT. | [Networking API](../syntropic/group__syn__net.md) |
| **Protocols** | Framing and serialization protocols (COBS, Modbus, CBOR, JSON). | [Protocols API](../syntropic/group__syn__protocol.md) |
| **Storage** | Virtual Filesystem (VFS) abstraction, LittleFS, FAT, and parameter/settings store. | [Storage API](../syntropic/group__syn__storage.md) |
| **System** | Bootloader integration, power management, watchdog, faults, and version information. | [System API](../syntropic/group__syn__system.md) |
| **DSP & Filters** | Digital signal processing, FFT, Kalman filter, biquad IIR (lowpass/highpass/bandpass/notch), signal statistics (RMS, stddev), and EMA/median filters. | [DSP API](../syntropic/group__syn__dsp.md) |
| **Debug & Diagnostics** | Logging systems, execution profiler, diagnostics tracing, and CLI commands. | [Debug API](../syntropic/group__syn__debug.md) |

## Indexes

If you are looking for a specific file or data structure, you can browse the global indexes:

* **[All Source & Header Files](../syntropic/files.md)** — A complete directory tree of all files in the project.
* **[All Data Structures](../syntropic/annotated.md)** — An alphabetical list of all structs, unions, and custom types.
