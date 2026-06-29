# SyntropicOS

**Modular Application Framework & Cooperative OS for Microcontrollers**

A standalone, zero-overhead C99 cooperative operating system and application micro-framework for microcontrollers. Add as a **git submodule**, configure in `syn_config.h`, and start building structured bare-metal software with cooperative multitasking, a command-line shell, diagnostic logging, parameter storage, control loops, and a lightweight immediate-mode GUI.

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
cp lib/SyntropicOS/syntropic/syn_config_template.h include/syn_config.h
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
[Browse Modules →](modules/core.md){ .md-button }
