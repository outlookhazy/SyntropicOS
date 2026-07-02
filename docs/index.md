# SyntropicOS

**High-Performance Bare-Metal Application Framework & Cooperative OS**

SyntropicOS is a zero-overhead, production-grade C99 framework designed for deeply embedded systems. Built for professional toolchains (GCC, Clang, IAR, Keil), it provides a modular ecosystem for building structured bare-metal software on high-performance targets like STM32 and RP2040.

## Why SyntropicOS?

- **Cooperative OS & Kernel** — Priority round-robin scheduler (`syn_sched`) and protothreads (`syn_pt`) provide lightweight execution threads using as little as **2 bytes of RAM per task**.
- **Zero dynamic allocation** — 100% of state is caller-owned or static. No heap allocation, safe for critical systems.
- **Embedded Graphics Engine** — Hardware-independent pixel canvas (`syn_canvas`) and a zero-allocation immediate-mode GUI (`syn_imgui`) supporting button, encoder, and touchscreen inputs.
- **Rich System Services** — Interactive CLI shell, severity-filtered logger, persistent wear-leveled parameter store, error log registry, and crash-loop recovery boot manager.
- **Digital Control & DSP** — Hardware-agnostic PID loops, stepper/servo/DC motor controllers, digital filters, and fixed-point math lookup tables.
- **Pure C99 & Low Overhead** — Compiles with any C99 compiler (GCC, Clang, Keil, IAR). Every module is individually toggleable via compile switches — only compile and link what you use.
- **No floating point** — Integer-only math throughout. No `libm.a` dependencies, saving flash and execution ticks.

## Quick Start

```bash
cd your_project
git submodule add https://github.com/outlookhazy/SyntropicOS lib/SyntropicOS
git submodule update --init
```

Copy the configuration template and enable the modules you need:

```bash
cp lib/SyntropicOS/src/syntropic/syn_config_template.h include/syn_config.h
```

Include in your build system:

=== "CMake"

    ```cmake
    add_subdirectory(lib/SyntropicOS)
    target_link_libraries(your_target PRIVATE syntropic)
    ```

=== "Makefile"

    ```makefile
    SYN_DIR := lib/SyntropicOS
    include $(SYN_DIR)/sources.mk
    CFLAGS += -I$(SYN_INC)
    SRCS   += $(SYN_SRCS)
    ```

Write your first task:

```c
#include "syntropic/syntropic.h"

#define TAG "main"

static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_gpio_toggle(LED_PIN);
        SYN_LOG_D(TAG, "blink");
        PT_TASK_DELAY_MS(pt, task, 500);
    }
    PT_END(pt);
}

int main(void)
{
    syn_gpio_init(LED_PIN, SYN_GPIO_OUTPUT);
    syn_log_init(my_uart_output, SYN_LOG_DEBUG);

    static SYN_Task tasks[1];
    static SYN_Sched sched;

    syn_task_create(&tasks[0], "blink", blink_task, 0, NULL);
    syn_sched_init(&sched, tasks, 1);
    syn_sched_run_forever(&sched);
}
```

[Getting Started →](getting-started.md){ .md-button .md-button--primary }
[Arduino Guide →](arduino.md){ .md-button }
[Browse Modules →](modules/core.md){ .md-button }
