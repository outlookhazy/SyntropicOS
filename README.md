# SyntropicOS

**High-Performance Bare-Metal Application Framework & Cooperative OS**

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![C99](https://img.shields.io/badge/C-C99-blue.svg)]()

SyntropicOS is a zero-overhead, production-grade C99 framework designed for deeply embedded systems. Built for professional toolchains (GCC, Clang, IAR, Keil), it provides a modular ecosystem for building structured bare-metal software on high-performance targets like STM32 and RP2040.

> **🚀 Development Status:** SyntropicOS is in active development. While core modules are tested and verified, the API is evolving. Feedback and contributions from the embedded community are welcome.

## Features

- **2-byte cooperative threads** — Priority round-robin scheduler with protothreads
- **Zero heap allocation** — All state is caller-owned or static
- **60+ modules** — Drivers, DSP, control loops, networking, graphics, CLI, logging, and more
- **Pure C99** — GCC, Clang, Keil, IAR — any C99 compiler
- **Integer-only math** — No floating point, no `libm.a`
- **Pay for what you use** — Every module toggleable via `#define`

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
