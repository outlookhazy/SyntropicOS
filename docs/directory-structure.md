# Directory Structure

```text
SyntropicOS/                          ← SyntropicOS repository
├── library.properties         ← Arduino Library Manager metadata
├── CONTRIBUTING.md            ← Architecture invariants, scope & PR workflow
├── SECURITY.md                ← Private vulnerability reporting policy
├── CMakeLists.txt             ← CMake build system
├── Makefile                   ← Top-level Makefile for building & testing
├── sources.mk                 ← Include file for external Makefiles
├── Doxyfile                   ← Doxygen API documentation configuration
├── mkdocs.yml                 ← MkDocs documentation site configuration
├── src/                       ← Core C99 OS & driver source files
│   ├── syntropic/
│   │   ├── syntropic.h             ← Umbrella header
│   │   ├── syn_config_template.h   ← System configuration template
│   │   ├── common/                ← Types, error codes, compiler macros
│   │   ├── port/                  ← Platform port interface headers (HAL)
│   │   ├── port_stubs/            ← Weak fallback stubs
│   │   ├── drivers/               ← Peripherals (GPIO, UART, ADC, I2C, SPI, CAN, USB)
│   │   ├── pt/                    ← Protothread coroutines, semaphores
│   │   ├── sched/                 ← Scheduler, timers, workqueues, mailboxes
│   │   ├── log/                   ← Logging, data logging, flight blackbox
│   │   ├── cli/                   ← Command-line interface
│   │   ├── debug/                 ← Event trace buffer & task CPU profiler
│   │   ├── util/                  ← Fixed-point math, matrices, CRC, CBOR, JSON, ring buffer
│   │   ├── crypto/                ← AES, SHA-2, BLAKE2s, ChaCha20-Poly1305, X25519, Ed25519, P-256, X.509, TPM2
│   │   ├── ble/                   ← BLE Host stack (HCI, L2CAP, ATT, GATT, GAP)
│   │   ├── audio/                 ← PCM playback, mixer, ADPCM, SBC, WAV, VAD
│   │   ├── input/                 ← Button debouncing, rotary encoders, keypads, RC receivers
│   │   ├── output/                ← LED control, smart LEDs, buzzers, soft PWM, DShot
│   │   ├── display/               ← Framebuffer canvas, OLED, character LCD, 7-segment
│   │   ├── ui/                    ← Zero-allocation IMGUI framework & menus
│   │   ├── control/               ← PID controller, flight PID, auto-tuner
│   │   ├── motor/                 ← Motor control, stepper, servo, BLDC, FOC, G-code
│   │   ├── dsp/                   ← Digital filters, FFT, DDS, Goertzel, MFCC, Kalman
│   │   ├── proto/                 ← Fieldbus, automotive & serial protocols (Modbus, CANopen, UDS, J1939, BACnet, etc.)
│   │   ├── net/                   ← Network stack (Ethernet/IP, TCP/UDP, TLS/DTLS, HTTP, WS, MQTT, CoAP, LwM2M, WireGuard)
│   │   ├── sensor/                ← Sensor polling framework & environmental/power/scale drivers
│   │   ├── storage/               ← Parameter store, settings, VFS, LittleFS, FAT
│   │   ├── system/                ← Boot manager, OTA, fault diagnostics, coredump, power
│   │   └── vm/                    ← Cooperative 32-bit WebAssembly interpreter (experimental)
│   └── port/                      ← Native port implementations
│       ├── posix/                 ← POSIX socket & timer port
│       ├── stm32f4/               ← STM32F4 bare-metal
│       ├── stm32_hal/             ← STM32 HAL
│       ├── ch32v307/              ← WCH CH32V307 RISC-V bare-metal
│       ├── esp32/                 ← ESP-IDF
│       ├── rp2040/                ← Raspberry Pi Pico SDK
│       └── arduino/               ← Arduino C++ SDK
├── tests/                     ← Test & verification suites
│   ├── unit/                  ← Unity unit test framework & unit test drivers
│   │   ├── mocks/             ← Weak hardware port mocks
│   │   └── unity/             ← Unity test engine
│   ├── integration/           ← 3rd-party containerized integration suite
│   │   ├── docker-compose.yml ← Service orchestration for integration daemons
│   │   ├── run_integration.sh ← Integration test driver runner
│   │   ├── services/          ← Service definitions (Nginx, CoreDNS, Mosquitto, SocketCAN, etc.)
│   │   └── test_*_integration.c
│   ├── emulation/             ← Bare-metal QEMU Cortex-M4 & Renode STM32F4 emulation
│   ├── fuzz/                  ← LLVM libFuzzer protocol fuzzing targets
│   ├── platformio/            ← Multi-MCU PlatformIO cross-compilation matrix
│   ├── sim/                   ← Physical motor & plant simulation harness
│   ├── wasm/                  ← WebAssembly test modules & fixtures
│   └── imgui_host/            ← Host GUI screenshot & timeline generator
├── tools/                     ← Containerization & CI automation tooling
│   └── containers/
│       ├── Containerfile      ← Podman/Docker verification image definition
│       └── Makefile           ← Container execution targets
├── examples/                  ← Hardware & SDK example projects
└── docs/                      ← Project documentation
```
