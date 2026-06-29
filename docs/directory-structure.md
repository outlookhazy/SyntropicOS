# Directory Structure

```
SyntropicOS/                          ← this repo (add as submodule)
├── CMakeLists.txt             ← add_subdirectory() from parent
├── sources.mk                 ← include() from parent Makefile
├── syntropic/
│   ├── syntropic.h                 ← umbrella header
│   ├── syn_config_template.h
│   ├── common/                ← types, compiler macros
│   ├── port/                  ← port interfaces (you implement)
│   │   ├── syn_port_system.h
│   │   ├── syn_port_gpio.h
│   │   ├── syn_port_uart.h
│   │   ├── syn_port_spi.h
│   │   ├── syn_port_i2c.h
│   │   ├── syn_port_flash.h
│   │   ├── syn_port_adc.h
│   │   ├── syn_port_dac.h
│   │   ├── syn_port_pwm.h
│   │   ├── syn_port_rtc.h
│   │   ├── syn_port_exti.h
│   │   ├── syn_port_can.h
│   │   ├── syn_port_wdt.h
│   │   └── syn_port_socket.h
│   ├── port_stubs/            ← weak stubs (optional)
│   ├── drivers/               ← GPIO, UART, ADC, EXTI, I2C/SPI device helpers
│   ├── pt/                    ← protothreads, semaphores
│   ├── sched/                 ← scheduler, timers, watchdog, sequencer, workqueue, mailbox
│   ├── log/                   ← logging, data logger
│   ├── cli/                   ← command-line interface (with built-in diagnostics)
│   ├── util/                  ← ring buffer, assert, bits, events, CRC, FSM, timeout,
│   │                            hysteresis, LUT, Q-math, rate limit, ping-pong, fmt
│   ├── input/                 ← button debouncer, rotary encoder
│   ├── output/                ← LED controller, soft PWM
│   ├── display/               ← hardware-independent canvas, shape primitives & bitmap drawing
│   ├── ui/                    ← interactive menu layouts & zero-allocation IMGUI framework
│   ├── control/               ← PID controller, auto-tuner
│   ├── motor/                 ← stepper, servo, DC motor, closed-loop motor ctrl, actuator
│   ├── dsp/                   ← digital filters, signal statistics, biquad, FFT
│   ├── proto/                 ← COBS framing, Modbus RTU
│   ├── net/                   ← cooperative network stack (HTTP, WebSockets, MQTT, DNS, mDNS, CoAP)
│   ├── sensor/                ← sensor polling framework (with signal stats integration)
│   ├── storage/               ← wear-leveled parameter store, VFS, LittleFS
│   ├── system/                ← boot manager, error log, version info, sleep coordinator
│   └── debug/                 ← trace buffer, task profiler
├── port/                      ← reference port implementations
│   ├── stm32f4/               ← STM32F4 bare-metal + Renode test infrastructure
│   ├── stm32_hal/             ← STM32 HAL (cross-family)
│   ├── esp32/                 ← ESP-IDF
│   ├── rp2040/                ← Raspberry Pi Pico SDK
│   └── arduino/               ← Arduino C++ SDK
├── examples/                  ← example projects
│   ├── arduino_serial/        ← Arduino CLI example (PlatformIO)
│   └── esp32_ota/             ← ESP32 OTA + web server + IMGUI example
├── tests/                     ← host-side test suite
│   ├── Makefile.unity         ← Unity test runner
│   ├── Makefile.check         ← Static analysis
│   ├── Makefile.fuzz          ← Fuzz testing
│   ├── imgui_host/            ← Native IMGUI/canvas test harness (HTTP + PNG)
│   ├── sim/                   ← simulation harness
│   ├── mocks/                 ← mock port layer
│   ├── fuzz/                  ← fuzz test inputs
│   └── unity/                 ← Unity test framework
└── docs/                      ← this documentation site
```
