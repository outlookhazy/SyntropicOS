# Module Overview & Catalog

SyntropicOS ships **278 portable C99 module headers** across 23 categories under `src/syntropic/`, backed by **24 core platform port contracts** (`src/syntropic/port/`) and **3 optional MAC/USB controller contracts** (`src/port/`). Every module uses static, caller-owned state structures with zero heap allocation (`malloc`/`free` are never called).

## Including Modules & Trimming Firmware Size

- **Umbrella Header**: `#include <syntropic/syntropic.h>` pulls in all enabled subsystem headers (controlled by `syn_config.h`).
- **Targeted Header**: `#include <syntropic/<category>/syn_<name>.h>` includes a single module directly for faster incremental builds or standalone modules.
- **Compile-Time Pruning**: Copy `src/syntropic/syn_config_template.h` to `syn_config.h` on your include path to disable unused `SYN_USE_*` switches or tune static table bounds. Pair with `-ffunction-sections -fdata-sections` and `-Wl,--gc-sections` so the linker strips unreferenced functions even when building the full source tree.

---

## Subsystem Guides

| Subsystem Guide | Source Directories | Headers | Functional Scope |
|---|---|---|---|
| **[Core & Utilities](core.md)** | `common/`, `util/` | 43 | Status codes, fixed-point & matrix math, CRCs, CBOR/JSON/Protobuf/Base64/LZ4 codecs, lock-free queues, pools/slabs, FSM, pub/sub |
| **[Multitasking & Scheduling](multitasking.md)** | `pt/`, `sched/` | 13 | 2-byte stackless protothreads, priority/EDF cooperative scheduler, mutexes with priority inheritance, event flags, mailboxes, timers, multicore |
| **[Hardware Drivers & Sensors](drivers.md)** | `drivers/`, `sensor/` | 45 | GPIO, EXTI, UART, I2C, SPI, 1-Wire, CAN, ADC, DAC, PWM/HRPWM, DMA, RTC/PPS time discipline, SD, RFID, USB 2.0 Device & Host, sensor framework & IMU fusion |
| **[Services (CLI & Logging)](services.md)** | `cli/`, `log/` | 4 | Interactive serial/transport CLI shell, leveled logger, structured binary ring datalogger, high-rate flight blackbox recorder |
| **[Input / Output](io.md)** | `input/`, `output/` | 13 | Debounced buttons, tap gestures, combos, rotary encoders, matrix keypads, analog joysticks, PPM, LED patterns, WS2812/SK6812, buzzers, soft PWM, DShot |
| **[Display & UI](display.md)** | `display/`, `ui/` | 7 | 1bpp/16bpp framebuffer canvas, compile-time renderer macros, immediate-mode GUI widgets, declarative ROM menus, SSD1306/SH1106 OLED, HD44780 LCD, 7-segment |
| **[Control & Motor](control.md)** | `control/`, `motor/` | 19 | Integer/cascaded flight PID, relay auto-tuner, trajectory & S-curve planners, steppers, servos, DC/BLDC 6-step, FOC & observer, G-code CNC interpreter |
| **[DSP & Filters](dsp.md)** | `dsp/` | 10 | Biquad IIR & runtime filter synthesis, FIR, EMA, median, Radix-2 FFT, NCO/DDS, Goertzel tone detector, MFCC extractor, 1D/2D Q16.16 Kalman filters |
| **[Communication & Networking](communication.md)** | `net/`, `proto/` | 77 | Zero-heap Ethernet/ARP/IPv4/TCP/UDP, TLS/DTLS 1.3, WireGuard, HTTP/WebSocket/MQTT/CoAP/LwM2M, CANopen/EtherCAT/Modbus/BACnet/UDS/DoIP/J1939/MAVLink |
| **[Bluetooth Low Energy](ble.md)** | `ble/` | 5 | Zero-heap BLE Host stack over H:4 UART/SPI: HCI transport, L2CAP CID 0x0004 reassembly, ATT PDU codec, GAP Central/Peripheral, static GATT Server |
| **[Audio & Codecs](audio.md)** | `audio/` | 6 | Non-blocking I2S/DMA PCM playback, saturating multi-voice mixer, IMA ADPCM 4:1 codec, Bluetooth A2DP SBC codec, RIFF/WAVE parser, energy VAD |
| **[Cryptography & PKI](crypto.md)** | `crypto/` | 15 | AES-128/256 (CBC/CTR/GCM/CCM/CMAC), ChaCha20-Poly1305, SHA-256/512, BLAKE2s, HMAC, HKDF, HMAC-DRBG, X25519, Ed25519, P-256 ECDH/ECDSA, ASN.1/X.509, TPM 2.0 |
| **[Storage & Filesystems](storage.md)** | `storage/` | 5 | Typed parameter table, wear-leveled flash settings, mountable VFS layer, power-loss resilient LittleFS, FAT16/FAT32 SD driver |
| **[System, Boot & Power](system.md)** | `system/` | 12 | A/B OTA & Ed25519/ECDSA secure boot verification, crash-loop boot manager, fault/coredump capture, power modes & tickless sleep, task/hardware watchdogs |
| **[WebAssembly VM](wasm.md)** | `vm/` | 1 | Cooperative fuel-metered 32-bit WebAssembly bytecode interpreter (MVP integer subset) with host function imports (experimental) |
| **[Debug & Profiling](debug.md)** | `debug/` | 3 | Lock-free ISR/task event trace ring, per-task CPU utilization & WCET profiler, optional runtime communication/scheduler counters |
| **[Cross-Module Integration](integration.md)** | Multi-subsystem | — | End-to-end pipelines combining scheduler, drivers, DSP, storage, and protocols |
| **[Porting Guide](../porting-guide.md)** | `port/`, `port_stubs/`, `src/port/` | 27 | Hardware abstraction contracts (`syn_port_*.h`) and 7 in-tree MCU/host reference ports |

---

## Complete Header Directory (All 278 Module Headers)

Use browser search (`Ctrl+F` / `Cmd+F`) or the site search bar to jump to any header, `SYN_USE_*` configuration switch, or subsystem guide.

### Core & Utilities ([Guide →](core.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `common/syn_barrier.h` | Always available | Memory ordering primitives for cross-core and ISR safety |
| `common/syn_compiler.h` | Always available | Compiler-portable macros for SyntropicOS |
| `common/syn_defs.h` | Always available | Common type definitions and status codes for SyntropicOS |
| `util/syn_assert.h` | Always available | Configurable assertion macro for SyntropicOS |
| `util/syn_backoff.h` | Always available | Exponential backoff helper with jitter (RFC 7252 style) |
| `util/syn_base64.h` | `SYN_USE_BASE64` | RFC 4648 Base64 and Base64URL encoding and decoding |
| `util/syn_bits.h` | Always available | Bit manipulation macros |
| `util/syn_cbor_read.h` | `SYN_USE_CBOR` | CBOR decoder — zero-alloc, streaming read from a byte buffer |
| `util/syn_cbor_write.h` | `SYN_USE_CBOR` | Streaming CBOR encoder — zero-alloc, caller-provided buffer |
| `util/syn_change_filter.h` | Always available | Dead-band + rate-limited change detector (header-only) |
| `util/syn_crc.h` | Always available | CRC calculation for communication protocols |
| `util/syn_event.h` | `SYN_USE_EVENT` | Event flag groups — 32-bit bitmask signaling |
| `util/syn_fmt.h` | `SYN_USE_FMT` | Lightweight formatting — no libc printf dependency |
| `util/syn_fsm.h` | `SYN_USE_FSM` | Lightweight table-driven finite state machine |
| `util/syn_geo.h` | `SYN_USE_GEO` | Geodetic coordinate transformations and distance calculation library |
| `util/syn_hysteresis.h` | Always available | Threshold comparator with hysteresis (deadband) |
| `util/syn_json_read.h` | `SYN_USE_JSON` | Minimal JSON reader — in-place, zero-alloc |
| `util/syn_json_write.h` | `SYN_USE_JSON` | Streaming JSON builder — zero-alloc, caller-provided buffer |
| `util/syn_lut.h` | Always available | Lookup table with linear interpolation |
| `util/syn_lz4.h` | `SYN_USE_LZ4` | Zero-Heap LZ4 Raw Block Compression & Decompression |
| `util/syn_matrix.h` | `SYN_USE_MATRIX` | Fixed-point Q16.16 matrix operations — zero heap allocation |
| `util/syn_metrics.h` | `SYN_USE_METRICS` | Lightweight telemetry and metrics (Counter/Gauge) |
| `util/syn_nn.h` | Always available | Zero-heap TinyML Neural Network Engine with Attention & Quantization Scaling |
| `util/syn_pack.h` | Always available | Binary message packer / unpacker (header-only) |
| `util/syn_pingpong.h` | Always available | Ping-pong (double) buffer — header-only, zero-copy DMA pattern |
| `util/syn_pool.h` | `SYN_USE_POOL` | Fixed-size block memory pool — zero-heap, header-only |
| `util/syn_protobuf.h` | `SYN_USE_PROTOBUF` | Zero-Allocation Protocol Buffers Serializer & Deserializer |
| `util/syn_pubsub.h` | `SYN_USE_PUBSUB` | Synchronous publish/subscribe event broker |
| `util/syn_qmath.h` | `SYN_USE_QMATH` | Fixed-point Q16.16 arithmetic — no floating point |
| `util/syn_quaternion.h` | Always available | Fixed-point Q16.16 3D Quaternion operations — zero heap allocation |
| `util/syn_ramp.h` | `SYN_USE_RAMP` | Ramp / motion profile generator |
| `util/syn_random.h` | Always available | Hardware-agnostic random number generation |
| `util/syn_rate_limit.h` | Always available | Token bucket rate limiter — header-only |
| `util/syn_ringbuf.h` | Always available | Fixed-size, statically-allocated ring buffer |
| `util/syn_scurve.h` | `SYN_USE_SCURVE` | Jerk-limited S-curve trajectory generator |
| `util/syn_slab.h` | Always available | Multi-Class Slab Memory Allocator |
| `util/syn_spinlock.h` | `SYN_USE_MULTICORE` | Scoped spinlock helper for shared resource protection |
| `util/syn_spsc_queue.h` | Always available | Lock-free Single-Producer Single-Consumer (SPSC) queue |
| `util/syn_str.h` | Always available | Zero-allocation string parsing, trimming, and tokenization utilities |
| `util/syn_stream.h` | Always available | Cooperative byte stream — protothread-aware ringbuf wrapper |
| `util/syn_timeout.h` | Always available | Non-blocking timeout helper — tick wrap-around safe |
| `util/syn_transform.h` | Always available | Fixed-point Q16.16 Coordinate System Transformations (Polar, Spherical, Cartesian) |
| `util/syn_vector.h` | Always available | Fixed-point Q16.16 Vector operations and signal statistics — zero heap allocation |

### Multitasking & Scheduling ([Guide →](multitasking.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `pt/syn_pt.h` | `SYN_USE_PT` | Protothreads — stackless cooperative coroutines for C |
| `pt/syn_pt_sem.h` | `SYN_USE_PT` | Lightweight counting semaphores for protothreads |
| `sched/syn_ao.h` | `SYN_USE_AO` | Active Object (Actor model) framework wrapper |
| `sched/syn_event_flags.h` | Always available | 32-bit Event Flag Group Synchronization |
| `sched/syn_mailbox.h` | Always available | Typed message queue — fixed-size SPSC ring for inter-task IPC |
| `sched/syn_mutex.h` | `SYN_USE_MUTEX` | Priority-inheriting mutual exclusion primitive for tasks and protothreads |
| `sched/syn_sched.h` | `SYN_USE_SCHED` | Cooperative scheduler for protothread tasks |
| `sched/syn_sequencer.h` | `SYN_USE_SEQUENCER` | Timed action sequencer — choreograph steps with delays |
| `sched/syn_task.h` | Always available | Task descriptor for the cooperative scheduler |
| `sched/syn_timer.h` | `SYN_USE_TIMER` | Software timer service — one-shot and periodic timers |
| `sched/syn_timer_wheel.h` | Always available | Hashed Timing Wheel Scheduler |
| `sched/syn_watchdog.h` | `SYN_USE_WATCHDOG` | Task-level watchdog monitor |
| `sched/syn_workqueue.h` | `SYN_USE_WORKQUEUE` | Deferred work queue — ISR-safe function dispatch |

### Hardware Drivers & Sensors ([Guide →](drivers.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `drivers/syn_adc.h` | `SYN_USE_ADC` | General-purpose multi-channel ADC driver — single-shot and DMA background scanning |
| `drivers/syn_can.h` | `SYN_USE_CAN` | CAN bus driver abstraction |
| `drivers/syn_comp.h` | `SYN_USE_COMP` | High-Speed Rail-to-Rail Analog Comparator Driver |
| `drivers/syn_dac.h` | `SYN_USE_DAC` | DAC (Digital-to-Analog Converter) driver |
| `drivers/syn_dma.h` | `SYN_USE_DMA` | Bare-Metal Safe DMA Transaction Engine |
| `drivers/syn_exti.h` | `SYN_USE_EXTI` | GPIO interrupt dispatcher — register callbacks per pin |
| `drivers/syn_gpio.h` | `SYN_USE_GPIO` | GPIO driver — high-level convenience functions |
| `drivers/syn_hpclock.h` | `SYN_USE_HPCLOCK` | High-precision clock — 64-bit system-clock-precision timestamps |
| `drivers/syn_hrpwm.h` | `SYN_USE_HRPWM` | High-Resolution Power & Precision Motion Control Driver |
| `drivers/syn_i2c.h` | `SYN_USE_I2C` | Hardware-decoupled I2C driver supporting Master/Slave roles & register access |
| `drivers/syn_i2c_dev.h` | Always available | I2C device register helpers — thin layer over port I2C |
| `drivers/syn_i2c_queue.h` | `SYN_USE_I2C` | Non-blocking I2C transaction queue driver for multi-client bus access |
| `drivers/syn_ioexp.h` | `SYN_USE_IOEXP` | Generic I2C GPIO Expander Driver (MCP23017, MCP23008, PCF8574, TCA9555) |
| `drivers/syn_pwm.h` | `SYN_USE_PWM` | Hardware PWM (Pulse Width Modulation) driver |
| `drivers/syn_rfid.h` | `SYN_USE_RFID` | Generic RFID & NFC Card Reader Driver (MFRC522 13.56MHz, PN532, RDM6300 125kHz) |
| `drivers/syn_rtc.h` | `SYN_USE_RTC` | RTC (Real-Time Clock) driver |
| `drivers/syn_sd.h` | `SYN_USE_SD` | SD card SPI block driver |
| `drivers/syn_shiftreg.h` | `SYN_USE_SHIFTREG` | Generic Shift Register Driver (74HC595 Output Expander & 74HC165 Input Expander) |
| `drivers/syn_soft_i2c.h` | `SYN_USE_SOFT_I2C` | Software I2C (bit-banging) driver |
| `drivers/syn_soft_onewire.h` | `SYN_USE_ONEWIRE` | Software bit-bang 1-Wire master driver |
| `drivers/syn_soft_spi.h` | `SYN_USE_SOFT_SPI` | Software SPI (bit-banging) driver |
| `drivers/syn_spi.h` | `SYN_USE_SPI` | Hardware-decoupled SPI driver supporting Master/Slave roles & full-duplex transfers |
| `drivers/syn_spi_dev.h` | Always available | SPI device register helpers — thin layer over port SPI |
| `drivers/syn_spi_queue.h` | `SYN_USE_SPI` | Non-blocking SPI transaction queue driver with CS management & parameter switching |
| `drivers/syn_timesync.h` | `SYN_USE_TIMESYNC` | High-precision time discipline and clock synchronization service |
| `drivers/syn_transport_usb_cdc.h` | Always available | USB CDC Transport Binding Header |
| `drivers/syn_transport_usb_host_cdc.h` | Always available | USB Host CDC Transport Binding Header |
| `drivers/syn_uart.h` | `SYN_USE_UART` | UART driver — buffered I/O and formatted output |
| `drivers/syn_usb.h` | Always available | Zero-Heap Modular USB 2.0 Device Core Protocol Engine |
| `drivers/syn_usb_cdc.h` | `SYN_USE_USB_CDC` | Zero-Heap USB 2.0 CDC ACM Virtual COM Port Class Driver |
| `drivers/syn_usb_hid.h` | Always available | Zero-Heap USB 2.0 Human Interface Device (HID) Class Driver |
| `drivers/syn_usb_hid_keyboard.h` | Always available | Zero-Heap USB HID Boot Keyboard Device Helper Module |
| `drivers/syn_usb_hid_mouse.h` | Always available | Zero-Heap USB HID Boot Mouse Device Helper Module |
| `drivers/syn_usb_host.h` | Always available | Zero-Heap USB 2.0 Host Core Protocol Engine |
| `drivers/syn_usb_host_cdc.h` | Always available | Zero-Heap USB 2.0 Host CDC ACM Class Driver |
| `drivers/syn_usb_midi.h` | `SYN_USE_USB_MIDI` | Zero-Heap USB 2.0 MIDI Class Device Driver |
| `drivers/syn_usb_msc.h` | `SYN_USE_USB_MSC` | Zero-Heap USB 2.0 Mass Storage Class (MSC) Device Driver |
| `sensor/syn_biometric.h` | `SYN_USE_BIOMETRIC` | Generic Pulse Oximeter & Heart Rate Sensor Driver (MAX30102, MAX30100) |
| `sensor/syn_climate.h` | `SYN_USE_CLIMATE` | Generic Climate & Environmental Sensor Driver (SHT3x, BME280, HTU21D, DHT22) |
| `sensor/syn_distance.h` | `SYN_USE_DISTANCE` | Generic Distance & Proximity Sensor Driver (HC-SR04, VL53L0X, Sharp IR) |
| `sensor/syn_lux.h` | `SYN_USE_LUX` | Generic Ambient Light & RGB Color Sensor Driver (BH1750, TSL2561, TCS34725) |
| `sensor/syn_powermon.h` | `SYN_USE_POWERMON` | Generic Power & Current Monitor Driver (INA219, INA226, INA3221) |
| `sensor/syn_scale.h` | `SYN_USE_SCALE` | Generic Weight Scale & Load Cell Driver (HX711 24-bit ADC, NAU7802) |
| `sensor/syn_sensor.h` | `SYN_USE_SENSOR` | Sensor polling framework — periodic read → filter → threshold → event |
| `sensor/syn_sensor_fusion.h` | Always available | 6-DOF IMU Sensor Fusion & AHRS Complementary / Mahony Filter (Q16.16 fixed-point) |

### Services (CLI & Logging) ([Guide →](services.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `cli/syn_cli.h` | `SYN_USE_CLI` | Command-line interpreter for embedded systems |
| `log/syn_blackbox.h` | `SYN_USE_BLACKBOX` | Zero-Heap Flight Telemetry Blackbox Binary Recorder |
| `log/syn_datalog.h` | `SYN_USE_DATALOG` | High-speed binary telemetry logger |
| `log/syn_log.h` | `SYN_USE_LOG` | Severity-filtered logging system |

### Input / Output ([Guide →](io.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `input/syn_button.h` | `SYN_USE_BUTTON` | Debounced button input with press/release/long-press/repeat |
| `input/syn_dipswitch.h` | `SYN_USE_DIPSWITCH` | DIP Switch & Multi-Bit Rotary Selector Driver |
| `input/syn_encoder.h` | `SYN_USE_ENCODER` | Quadrature rotary encoder decoder |
| `input/syn_joystick.h` | `SYN_USE_JOYSTICK` | Generic Analog Joystick & Multi-Axis Potentiometer Driver |
| `input/syn_keypad.h` | `SYN_USE_KEYPAD` | Non-blocking Matrix Keypad Scanner (3x4, 4x4, etc.) |
| `input/syn_ppm.h` | `SYN_USE_PPM` | Zero-Heap PPM (Pulse-Position Modulation) Multi-Channel RC Receiver Decoder |
| `input/syn_touch.h` | `SYN_USE_TOUCH` | Capacitive Touch Sensor Driver (relaxation/ADC charge sensing) |
| `output/syn_buzzer.h` | `SYN_USE_BUZZER` | Non-blocking Piezo Buzzer & Tone Generator module |
| `output/syn_dshot.h` | `SYN_USE_DSHOT` | Zero-Heap DShot (DShot150/300/600) Digital ESC Driver |
| `output/syn_dshot_telemetry.h` | `SYN_USE_DSHOT_TELEMETRY` | Zero-Heap Bidirectional DShot (BDShot) Telemetry Decoder |
| `output/syn_led.h` | `SYN_USE_LED` | Non-blocking LED controller — blink, flash, patterns |
| `output/syn_smartled.h` | `SYN_USE_SMARTLED` | Generic Addressable RGB/RGBW Smart LED Strip Driver (WS2812B, SK6812, APA102, SK9822) |
| `output/syn_soft_pwm.h` | `SYN_USE_SOFT_PWM` | Software PWM on any GPIO pin |

### Display & UI ([Guide →](display.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `display/syn_canvas.h` | `SYN_USE_CANVAS` | Display canvas — hardware-independent framebuffer + drawing |
| `display/syn_charlcd.h` | `SYN_USE_CHARLCD` | Generic Character LCD Driver (HD44780, ST7066, KS0066 over I2C PCF8574 or 4-Bit Parallel GPIO) |
| `display/syn_gfx.h` | `SYN_USE_CANVAS` | Compile-time graphics renderer abstraction |
| `display/syn_oled.h` | `SYN_USE_OLED` | Generic Monochrome OLED Display Driver (SSD1306, SH1106, SSD1309 over I2C) |
| `display/syn_seg7.h` | `SYN_USE_SEG7` | 7-Segment LED Display & Multi-Digit Array Driver |
| `ui/syn_imgui.h` | `SYN_USE_IMGUI` | Lightweight, zero-allocation Immediate Mode GUI (IMGUI) for embedded systems |
| `ui/syn_menu.h` | `SYN_USE_MENU` | Tree-structured menu system |

### Control & Motor ([Guide →](control.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `control/syn_autotune.h` | `SYN_USE_AUTOTUNE` | Motor controller auto-tuner — feedforward ID + relay PID tuning |
| `control/syn_control_stats.h` | Always available | Performance monitoring and "Tuning Scorecard" for control loops |
| `control/syn_flight_pid.h` | `SYN_USE_FLIGHT_PID` | Zero-Heap 3-Axis Quadcopter Flight PID Stabilization & Motor Mixer |
| `control/syn_pid.h` | `SYN_USE_PID` | General-purpose PID controller — integer arithmetic |
| `control/syn_rc_curve.h` | `SYN_USE_RC_CURVE` | Zero-Heap RC Stick Exponential (Expo) & Deadband Curve Mapper |
| `control/syn_rc_failsafe.h` | `SYN_USE_RC_FAILSAFE` | Zero-Heap RC Safety Failsafe Manager & Watchdog |
| `motor/syn_actuator.h` | `SYN_USE_ACTUATOR` | Linear actuator with potentiometer feedback |
| `motor/syn_bldc_6step.h` | `SYN_USE_BLDC_6STEP` | Zero-Heap 6-Step (Trapezoidal) BLDC Motor Commutation Driver |
| `motor/syn_dc_motor.h` | `SYN_USE_DC_MOTOR` | DC motor controller for H-bridge drivers |
| `motor/syn_foc.h` | `SYN_USE_FOC` | Fixed-point Field-Oriented Control transforms (Clarke & Park) |
| `motor/syn_foc_encoder.h` | Always available | Sensored FOC Rotor Position & Speed Interface (Encoder / Hall / Absolute) |
| `motor/syn_foc_observer.h` | Always available | Sensorless FOC Rotor Position & Speed Sliding Mode Observer (SMO) (Q16.16 fixed-point) |
| `motor/syn_gcode.h` | `SYN_USE_GCODE` | G-Code & NIST RS-274 Motion Interpreter and Controller |
| `motor/syn_interpolator.h` | `SYN_USE_INTERPOLATOR` | Coordinated multi-axis linear & circular arc trajectory interpolator |
| `motor/syn_kinematics.h` | `SYN_USE_KINEMATICS` | Multi-Axis Robot Forward & Inverse Kinematics Engine (Q16.16 fixed-point) |
| `motor/syn_motor_ctrl.h` | `SYN_USE_MOTOR_CTRL` | Closed-loop motor controller — generic feedback + PID + motor |
| `motor/syn_motor_output.h` | Always available | Abstract motor output interface |
| `motor/syn_servo.h` | `SYN_USE_SERVO` | Hobby servo controller — pulse-width positioning |
| `motor/syn_stepper.h` | `SYN_USE_STEPPER` | Stepper motor driver — step/direction with acceleration ramps |

### DSP & Filters ([Guide →](dsp.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `dsp/syn_biquad.h` | `SYN_USE_BIQUAD` | Fixed-point Q16.16 Biquad filter (Direct Form I) |
| `dsp/syn_dds.h` | `SYN_USE_DDS` | Direct Digital Synthesis (DDS) Waveform Synthesizer |
| `dsp/syn_dsp.h` | Always available | Zero-heap Fixed-Point DSP & Spectral Processing Primitives |
| `dsp/syn_fft.h` | `SYN_USE_FFT` | Fixed-point Q16.16 Fast Fourier Transform (FFT) |
| `dsp/syn_filter.h` | `SYN_USE_FILTER` | Integer-only digital filters for sensor signal conditioning |
| `dsp/syn_filter_design.h` | Always available | Dynamic runtime IIR Biquad filter coefficient generator (Q16.16 fixed-point) |
| `dsp/syn_goertzel.h` | `SYN_USE_GOERTZEL` | Integer-Only Goertzel Algorithm Single-Frequency Tone Detector |
| `dsp/syn_kalman.h` | `SYN_USE_KALMAN` | General-purpose fixed-point Kalman filter |
| `dsp/syn_mfcc.h` | `SYN_USE_MFCC` | Zero-Heap Fixed-Point Mel-Frequency Cepstral Coefficients (MFCC) Extractor |
| `dsp/syn_signal.h` | `SYN_USE_SIGNAL` | Signal statistics — sliding window min/max/mean/variance/RMS |

### Communication & Networking ([Guide →](communication.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `net/syn_autoip.h` | `SYN_USE_AUTOIP` | Zero-Heap Native RFC 3927 AutoIP (Link-Local 169.254.x.x) Engine |
| `net/syn_coap.h` | `SYN_USE_COAP` | Constrained Application Protocol (CoAP - RFC 7252) implementation |
| `net/syn_dhcp.h` | `SYN_USE_DHCP` | Zero-Heap Native DHCP Client Protocol Engine |
| `net/syn_dns.h` | `SYN_USE_DNS` | UDP DNS resolver and mDNS responder |
| `net/syn_dnssd.h` | Always available | DNS-Based Service Discovery (DNS-SD - RFC 6763 / RFC 6762) |
| `net/syn_dtls.h` | `SYN_USE_DTLS` | Native Zero-Heap DTLS 1.3 Datagram Protocol Engine (RFC 9147) |
| `net/syn_eth.h` | `SYN_USE_ETH` | Zero-Heap Native Ethernet II & ARP Protocol Engine |
| `net/syn_heartbeat.h` | `SYN_USE_HEARTBEAT` | Heartbeat / keep-alive monitor |
| `net/syn_http.h` | `SYN_USE_HTTP` | Cooperative HTTP/1.1 client — streaming, zero-alloc, non-blocking |
| `net/syn_httpd.h` | `SYN_USE_HTTPD` | Minimal HTTP/1.1 server — route-based, zero-alloc |
| `net/syn_icmp.h` | `SYN_USE_ICMP` | Zero-Heap Native ICMP Protocol Engine |
| `net/syn_igmp.h` | `SYN_USE_IGMP` | Zero-Heap Native IGMPv2 Protocol Engine |
| `net/syn_mqtt.h` | `SYN_USE_MQTT` | Lightweight MQTT 3.1.1 client |
| `net/syn_netbuf.h` | Always available | Zero-Copy Packet Buffer Pool |
| `net/syn_netcfg.h` | `SYN_USE_NETCFG` | Unified Zero-Heap Network IP Address Manager |
| `net/syn_ntp_server.h` | `SYN_USE_NTP_SERVER` | Zero-Heap NTP v4 Server Protocol Engine over UDP port 123 |
| `net/syn_router.h` | `SYN_USE_ROUTER` | Packet router — addressed message dispatch |
| `net/syn_sntp.h` | `SYN_USE_SNTP` | SNTP client — lightweight time synchronization over UDP |
| `net/syn_tcp.h` | Always available | Zero-Alloc Native TCP State Machine & Segment Engine |
| `net/syn_tls.h` | `SYN_USE_TLS` | Native Zero-Heap TLS 1.3 Protocol Engine & Transport Adapter (RFC 8446) |
| `net/syn_transport.h` | Always available | Transport abstraction — pluggable send/receive (header-only) |
| `net/syn_transport_tcp.h` | `SYN_USE_TRANSPORT_TCP` | Bridge for syn_transport over a TCP socket |
| `net/syn_transport_udp.h` | `SYN_USE_TRANSPORT_UDP` | Bridge binding syn_port_udp_* API to native syn_udp stack |
| `net/syn_udp.h` | Always available | Zero-Alloc Native UDP Demuxing Engine & Task-Blocking Layer |
| `net/syn_websocket.h` | `SYN_USE_WEBSOCKET` | WebSocket protocol support on httpd |
| `net/syn_wg.h` | `SYN_USE_WG` | WireGuard VPN client — Noise IK, pure C99, cooperative |
| `proto/syn_at_parser.h` | `SYN_USE_AT_PARSER` | Zero-allocation, stream-oriented AT command parser for cellular & serial modems |
| `proto/syn_bacnet.h` | `SYN_USE_BACNET` | Zero-malloc, cleanroom BACnet MS/TP & APDU protocol stack (ISO 16484-5) |
| `proto/syn_cannm.h` | `SYN_USE_CANNM` | AUTOSAR CAN Network Management (CanNm) Protocol |
| `proto/syn_canopen.h` | `SYN_USE_CANOPEN` | CANopen DS301 Slave Protocol Engine |
| `proto/syn_canopen_mgr.h` | Always available | CANopen CiA 302 Network Manager, SDO Client & NMT Master |
| `proto/syn_ccp.h` | `SYN_USE_CCP` | ASAM CAN Calibration Protocol v2.1 (CCP) Slave Implementation |
| `proto/syn_cia303.h` | `SYN_USE_CANOPEN` | CANopen CiA 303-3 Indicator & Status LED Specification implementation |
| `proto/syn_cia401.h` | Always available | CANopen CiA 401 Device Profile for Generic I/O Modules |
| `proto/syn_cia402.h` | `SYN_USE_CIA402` | CANopen CiA 402 Device Profile for Drives and Motion Control |
| `proto/syn_cia418.h` | Always available | CANopen CiA 418 Device Profile Object Dictionary Definitions |
| `proto/syn_cjt188.h` | `SYN_USE_CJT188` | CJ/T 188-2004 / CJ/T 188-2018 Smart Metering Protocol |
| `proto/syn_cobs.h` | `SYN_USE_COBS` | COBS (Consistent Overhead Byte Stuffing) packet framing |
| `proto/syn_cose.h` | `SYN_USE_COSE` | Zero-Heap CBOR Object Signing and Encryption (COSE - RFC 9052 / RFC 8152) |
| `proto/syn_crsf.h` | `SYN_USE_CRSF` | Zero-Heap CRSF (TBS Crossfire / ExpressLRS) Protocol Parser |
| `proto/syn_dali.h` | `SYN_USE_DALI` | Digital Addressable Lighting Interface (DALI / IEC 62386) Protocol Stack |
| `proto/syn_devicenet.h` | `SYN_USE_DEVICENET` | ODVA DeviceNet (Common Industrial Protocol over CAN) Protocol Stack |
| `proto/syn_dlt645.h` | Always available | DL/T 645 Electricity Meter Communication Protocol (1997 & 2007) |
| `proto/syn_dmx512.h` | `SYN_USE_DMX512` | USITT DMX512-A Stage & Architectural Lighting Protocol Stack |
| `proto/syn_doip.h` | `SYN_USE_DOIP` | SyntropicOS ISO 13400-2 Diagnostic over IP (DoIP) Protocol Stack |
| `proto/syn_ethercat.h` | `SYN_USE_ETHERCAT` | EtherCAT (IEEE 802.3 EtherType 0x88A4) Bare-Metal Protocol Stack |
| `proto/syn_gbt27930.h` | `SYN_USE_GBT27930` | GB/T 27930 (EV DC Fast Charging Protocol on SAE J1939 CAN Bus) |
| `proto/syn_ibus.h` | `SYN_USE_IBUS` | Zero-Heap IBUS (FlySky) 14-Channel Receiver Decoder |
| `proto/syn_iolink.h` | `SYN_USE_IOLINK` | IO-Link (IEC 61131-9) Single-Drop Digital Point-to-Point Master & Device Protocol Engine |
| `proto/syn_ir.h` | `SYN_USE_IR` | Infrared (IR) Remote Control Protocol Engine (NEC, Sony, RC5, RC6, Samsung, Panasonic, Denon, Apple) |
| `proto/syn_isotp.h` | `SYN_USE_ISOTP` | ISO 15765-2 (DoCAN / ISO-TP) Multi-Frame CAN Transport Protocol |
| `proto/syn_j1939.h` | `SYN_USE_J1939` | SAE J1939 Heavy-Duty Vehicle CAN Protocol Stack |
| `proto/syn_kwp2000.h` | Always available | ISO 14230-3 Key Word Protocol 2000 (KWP2000) Server Implementation |
| `proto/syn_lin.h` | `SYN_USE_LIN` | LIN (Local Interconnect Network 2.1 / 2.2a & ISO 17987) Protocol Engine |
| `proto/syn_lintp.h` | `SYN_USE_LINTP` | ISO 17987-2 (LIN Transport Layer & Network Layer Services / UDSonLIN) |
| `proto/syn_lss.h` | Always available | CANopen CiA 305 Layer Setting Services (LSS) & Protocols |
| `proto/syn_lwm2m.h` | `SYN_USE_LWM2M` | Zero-Heap OMA Lightweight M2M (LwM2M v1.1/v1.2) Core Client Engine |
| `proto/syn_lwm2m_task.h` | `SYN_USE_LWM2M_TASK` | Autonomous OMA LwM2M Client Supervisor Task & Engine |
| `proto/syn_mavlink.h` | `SYN_USE_MAVLINK` | Zero-Heap MAVLink v2 Protocol Encoder & Streaming Decoder |
| `proto/syn_mbus.h` | `SYN_USE_MBUS` | M-Bus (Meter-Bus, EN 13757-2 / EN 13757-3) Protocol Engine |
| `proto/syn_modbus.h` | `SYN_USE_MODBUS` | Modbus RTU slave implementation |
| `proto/syn_modbus_master.h` | `SYN_USE_MODBUS_MASTER` | Non-blocking Modbus RTU Master / Client protocol engine |
| `proto/syn_modbus_tcp.h` | `SYN_USE_MODBUS` | Modbus TCP ADU (MBAP) framing layer |
| `proto/syn_mqttsn.h` | `SYN_USE_MQTTSN` | MQTT for Sensor Networks (MQTT-SN v1.2) Protocol Client Engine |
| `proto/syn_msp.h` | `SYN_USE_MSP` | Zero-Heap MSP v1 (MultiWii Serial Protocol v1) Encoder & Decoder |
| `proto/syn_n2k.h` | `SYN_USE_N2K` | NMEA 2000 Marine CAN Protocol Stack |
| `proto/syn_nmea.h` | `SYN_USE_NMEA` | NMEA 0183 / GNSS sentence parser and encoder |
| `proto/syn_ocpp.h` | Always available | Open Charge Point Protocol over JSON (OCPP-J 1.6 / 2.0.1) Dual-Role Engine |
| `proto/syn_opcua.h` | `SYN_USE_OPCUA` | Micro OPC UA (IEC 62541) Binary Protocol Engine & Embedded Server |
| `proto/syn_pmbus.h` | `SYN_USE_PMBUS` | PMBus (Power Management Bus 1.2 / 1.3) Protocol Engine & Linear Format Converters |
| `proto/syn_sbus.h` | `SYN_USE_SBUS` | Zero-Heap SBUS (Futaba/FrSky) 16-Channel Receiver Decoder |
| `proto/syn_smbus.h` | `SYN_USE_SMBUS` | SMBus (System Management Bus 1.1 / 2.0 / 3.0) Protocol Engine |
| `proto/syn_uds.h` | `SYN_USE_UDS` | ISO 14229 Unified Diagnostic Services (UDS) Server Implementation |
| `proto/syn_uds_util.h` | Always available | SAE J2012 / ISO 15031-6 UDS Diagnostic Trouble Code (DTC) Utilities |
| `proto/syn_xcp.h` | `SYN_USE_XCP` | ASAM Universal Measurement and Calibration Protocol v1.x (XCP) Slave |
| `proto/syn_xrce_dds.h` | `SYN_USE_XRCE_DDS` | Micro-ROS & eProsima Micro XRCE-DDS v1.2 Protocol Client Engine |
| `proto/syn_ymodem.h` | `SYN_USE_YMODEM` | YMODEM / XMODEM-1K Serial File Transfer Receiver Protocol |

### Bluetooth Low Energy ([Guide →](ble.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `ble/syn_ble_att.h` | `SYN_USE_BLE` | Zero-Heap Attribute Protocol (ATT) Encoding and Decoding |
| `ble/syn_ble_gap.h` | `SYN_USE_BLE` | Zero-Heap Generic Access Profile (GAP) Layer |
| `ble/syn_ble_gatt.h` | `SYN_USE_BLE` | Zero-Heap Generic Attribute Profile (GATT) Server Engine |
| `ble/syn_ble_hci.h` | `SYN_USE_BLE` | Zero-Heap Host Controller Interface (HCI) Driver (H:4 UART Framing) |
| `ble/syn_ble_l2cap.h` | `SYN_USE_BLE` | Zero-Heap BLE Logical Link Control and Adaptation Protocol (L2CAP) |

### Audio & Codecs ([Guide →](audio.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `audio/syn_adpcm.h` | `SYN_USE_ADPCM` | IMA/DVI ADPCM Speech and Audio Codec (Encoder & Decoder) |
| `audio/syn_audio.h` | `SYN_USE_AUDIO` | Codec-Agnostic PCM Double-Buffered Audio Playback Engine |
| `audio/syn_audio_mixer.h` | `SYN_USE_AUDIO` | Zero-Heap Multi-Channel Q15 Fixed-Point Audio Mixer |
| `audio/syn_sbc.h` | `SYN_USE_SBC` | Bluetooth A2DP Sub-Band Codec (SBC) Decoder |
| `audio/syn_vad.h` | `SYN_USE_VAD` | Zero-Heap Acoustic Voice Activity Detector (VAD) Engine (Q15/Q16 fixed-point) |
| `audio/syn_wav.h` | `SYN_USE_AUDIO` | Zero-Heap RIFF/WAVE Header Parser |

### Cryptography & PKI ([Guide →](crypto.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `crypto/syn_aes.h` | `SYN_USE_AES` | Unified AES cipher engine (128/192/256-bit keys, ECB, CBC, CTR, and GCM AEAD) |
| `crypto/syn_aes_cmac.h` | `SYN_USE_AES_CMAC` | AES-CMAC (Cipher-based Message Authentication Code, RFC 4493 / NIST SP 800-38B) |
| `crypto/syn_asn1.h` | Always available | Zero-Heap ASN.1 DER (Distinguished Encoding Rules) TLV Parser |
| `crypto/syn_blake2s.h` | `SYN_USE_BLAKE2S` | BLAKE2s cryptographic hash — RFC 7693, pure C99 |
| `crypto/syn_chacha20poly1305.h` | `SYN_USE_CHACHA20POLY1305` | ChaCha20-Poly1305 AEAD — RFC 8439, pure C99 |
| `crypto/syn_ed25519.h` | `SYN_USE_ED25519` | Pure C99 Ed25519 Key Generation, Signing & Verification (RFC 8032) |
| `crypto/syn_hkdf.h` | `SYN_USE_HKDF` | HMAC-based Extract-and-Expand Key Derivation Function (HKDF-SHA256, RFC 5869) & TLS 1.3 HKDF-Expand-Label |
| `crypto/syn_hmac.h` | `SYN_USE_SHA256` | HMAC-SHA256 — keyed-hash message authentication code |
| `crypto/syn_hmac_drbg.h` | `SYN_USE_HMAC_DRBG` | NIST SP 800-90A HMAC-DRBG (Deterministic Random Bit Generator) with SHA-256 |
| `crypto/syn_p256.h` | `SYN_USE_P256` | NIST P-256 (secp256r1 / prime256v1) Elliptic Curve Cryptography |
| `crypto/syn_sha256.h` | `SYN_USE_SHA256` | SHA-256 cryptographic hash — pure C99, zero dependencies |
| `crypto/syn_sha512.h` | `SYN_USE_SHA512` | SHA-512 & SHA-384 cryptographic hash and HMAC — pure C99, zero dependencies |
| `crypto/syn_tpm2.h` | `SYN_USE_TPM2` | TCG TPM 2.0 Command Marshaller, Measured Boot, & Hardware Root-of-Trust Engine |
| `crypto/syn_x25519.h` | `SYN_USE_X25519` | X25519 Diffie-Hellman key exchange — RFC 7748, pure C99 |
| `crypto/syn_x509.h` | `SYN_USE_X509` | Zero-Heap X.509 v3 Certificate & Chain Parser and Validator |

### Storage & Filesystems ([Guide →](storage.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `storage/syn_fat.h` | `SYN_USE_FAT` | Custom lightweight FAT16/FAT32 VFS adapter |
| `storage/syn_lfs.h` | `SYN_USE_VFS` | LittleFS filesystem VFS adapter |
| `storage/syn_param.h` | `SYN_USE_PARAM` | Persistent parameter store with wear leveling |
| `storage/syn_settings.h` | `SYN_USE_SETTINGS` | Persistent settings manager with change detection |
| `storage/syn_vfs.h` | `SYN_USE_VFS` | Virtual File System (VFS) abstraction layer |

### System, Boot & Power ([Guide →](system.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `system/syn_boot.h` | `SYN_USE_BOOT` | Boot manager — reset reason, boot counter, safe-mode detection |
| `system/syn_coredump.h` | `SYN_USE_COREDUMP` | Persistent core dump — saves fault context to flash |
| `system/syn_errlog.h` | `SYN_USE_ERRLOG` | Persistent error registry — survives resets via param store |
| `system/syn_fault.h` | `SYN_USE_FAULT` | CPU Hard Fault / Post-Mortem Diagnostic log collector |
| `system/syn_fwboot.h` | `SYN_USE_BOOT` | A/B firmware boot manager — slot selection, rollback, confirm |
| `system/syn_fwimage.h` | `SYN_USE_BOOT` | Firmware image header — standardized format for OTA slots |
| `system/syn_fwupdate.h` | `SYN_USE_BOOT` | Streaming firmware updater — transport-agnostic, zero-alloc |
| `system/syn_hwwdt.h` | `SYN_USE_HWWDT` | Hardware Watchdog Timer driver |
| `system/syn_ota.h` | `SYN_USE_OTA` | Secure Streaming OTA Orchestrator — zero-heap, transport-agnostic |
| `system/syn_power.h` | `SYN_USE_POWER` | Power / voltage monitor |
| `system/syn_sleep.h` | Always available | Sleep coordinator — low-power mode management |
| `system/syn_version.h` | Always available | Build version and metadata — compile-time constants |

### WebAssembly VM ([Guide →](wasm.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `vm/syn_wasm.h` | `SYN_USE_WASM` | [EXPERIMENTAL] Zero-heap, 32-bit WebAssembly (Wasm MVP) cooperative interpreter |

### Debug & Profiling ([Guide →](debug.md))

| Header (`<syntropic/...>`) | Config Switch | Description |
|---|---|---|
| `debug/syn_profiler.h` | `SYN_USE_PROFILER` | Task profiler — CPU time tracking per scheduler task |
| `debug/syn_task_profile.h` | `SYN_USE_PROFILER` | Opt-In Task Execution & CPU Jitter Profiler |
| `debug/syn_trace.h` | `SYN_USE_TRACE` | Lightweight trace buffer — timestamped event recorder |
