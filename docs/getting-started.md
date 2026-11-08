# Getting Started with SyntropicOS

SyntropicOS is a cooperative C99 framework designed to deliver multi-tasking and structured software architecture for resource-constrained microcontrollers without the memory overhead of a preemptive RTOS.

---

## 1. Understanding the Core Architecture

Before adding SyntropicOS to your codebase, understand three core concepts:

### Stackless Protothreads (`syn_pt`)
Traditional RTOS tasks require separate hardware stacks (512 B to 4 KB RAM per thread). SyntropicOS uses **protothreads** — stackless C coroutines that run on the system's single main stack.
- **RAM Cost**: Exactly **2 bytes** per thread (`uint16_t lc` continuation variable).
- **Execution Mechanism**: `PT_BEGIN()` and `PT_END()` implement a Duff's device continuation (`switch(pt->lc)`). When yielding (`PT_PASS`, `PT_CONTINUE`, `PT_WAIT_UNTIL`, or `PT_TASK_DELAY_MS`), execution returns its scheduling intent (`PT_IDLE` or `PT_MORE`) to the scheduler and resumes at the saved line on the next dispatch.
- **Variable Lifetime Rule**: Local function variables do **not** persist across yield points. Persistent variables must be declared as `static`, allocated globally, or stored in a task context structure passed to `syn_task_create()`.

### Zero-Heap Allocation
SyntropicOS requires **zero dynamic memory** (`malloc()` / `free()`). Task structures (`SYN_Task`), queues (`SYN_RingBuf`), software timers (`SYN_Timer`), and protocol handles are allocated statically by the application at build time.

### Ground-Up Non-Blocking Drivers & Protocol Stacks
Every peripheral driver (GPIO, UART, SPI, I2C, CAN) and protocol stack (Modbus, BACnet, DALI, M-Bus, COBS, BLE) is written from scratch as a cooperative, non-blocking state machine. Modules expose `_poll()`, `_update()`, or `_process()` entry points that do a bounded unit of work and return immediately. No module internally blocks, busy-waits, or calls `delay()`.

---

## 2. First 5 Minutes: Minimal Application Tutorial

A complete two-task example: a non-blocking LED blink task and a periodic system status log task.

```c
#include "syntropic/syntropic.h"

#define LED_PIN  13
#define LOG_TAG  "APP"

/* Task 1: Non-blocking LED blink protothread */
static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task) {
    PT_BEGIN(pt);
    for (;;) {
        syn_gpio_toggle(LED_PIN);
        PT_TASK_DELAY_MS(pt, task, 500); /* Yields CPU for 500 ms without blocking */
    }
    PT_END(pt);
}

/* Task 2: Periodic heartbeat status log task */
static SYN_PT_Status log_task(SYN_PT *pt, SYN_Task *task) {
    PT_BEGIN(pt);
    for (;;) {
        SYN_LOG_I(LOG_TAG, "System tick: %lu ms", (unsigned long)syn_port_get_tick_ms());
        PT_TASK_DELAY_MS(pt, task, 2000); /* Log every 2 seconds */
    }
    PT_END(pt);
}

int main(void) {
    /* 1. Initialize hardware via HAL port interface */
    syn_gpio_init(LED_PIN, SYN_GPIO_OUTPUT);
    syn_log_init(NULL, SYN_LOG_INFO); /* Console serial output */

    /* 2. Statically allocate tasks and scheduler control block */
    static SYN_Task tasks[2];
    static SYN_Sched sched;

    /* 3. Initialize task descriptors (SYN_PRIO_CRITICAL = 0 highest .. SYN_PRIO_LOW = 3) */
    syn_task_create(&tasks[0], "blink", blink_task, SYN_PRIO_NORMAL, NULL);
    syn_task_create(&tasks[1], "log",   log_task,   SYN_PRIO_LOW,    NULL);

    /* 4. Pass static task array to scheduler */
    syn_sched_init(&sched, tasks, 2);

    /* 5. Start cooperative execution loop */
    syn_sched_run_forever(&sched);
}
```

---

## 3. Adding SyntropicOS to Your Project

### As a Git Submodule

```bash
cd your_project
git submodule add https://github.com/outlookhazy/SyntropicOS lib/SyntropicOS
git submodule update --init
```

### Build System Integration

=== "CMake"

    In your project's `CMakeLists.txt`:

    ```cmake
    add_subdirectory(lib/SyntropicOS)
    target_link_libraries(your_target PRIVATE syntropic)

    # Optional: include weak port stubs for initial prototyping
    target_link_libraries(your_target PRIVATE syn_stubs)
    ```

    The `syntropic` target is an **INTERFACE library** — it adds the include path and source files to your target, compiled with your project's own flags and toolchain. No separate compilation step, no flag mismatches.

=== "Makefile"

    ```makefile
    SYN_DIR := lib/SyntropicOS
    include $(SYN_DIR)/sources.mk

    CFLAGS += -I$(SYN_INC)
    SRCS   += $(SYN_SRCS)

    # Optional: weak stubs
    SRCS   += $(SYN_STUB_SRCS)
    ```

    `sources.mk` exports three variables:

    - `SYN_SRCS` — all SyntropicOS `.c` source files
    - `SYN_STUB_SRCS` — weak port stubs
    - `SYN_INC` — include path (`src/` subdirectory)

=== "Manual / IDE"

    1. Add `lib/SyntropicOS/` to your include paths.
    2. Add the `.c` files from the `src/syntropic/` subdirectories you need to your build.
    3. Optionally add `src/syntropic/port_stubs/syn_port_stubs.c`.

---

## 4. Configuration

SyntropicOS is configured through a single header file, `syn_config.h`, placed on your include path.

### Creating Your Config

```bash
cp lib/SyntropicOS/src/syntropic/syn_config_template.h your_project/include/syn_config.h
```

### How It Works

The umbrella header (`syntropic/syntropic.h`) checks for each module using this pattern:

```c
#if !defined(SYN_USE_GPIO) || SYN_USE_GPIO
  #include "drivers/syn_gpio.h"
#endif
```

This means:

- If `SYN_USE_GPIO` is **not defined** → module is **enabled** (included by default)
- If `SYN_USE_GPIO` is **defined as `1`** → module is **enabled**
- If `SYN_USE_GPIO` is **defined as `0`** → module is **disabled**

!!! info "No config file? No problem."
    If no `syn_config.h` is found on the include path, all modules default to **enabled**. This is useful for quick prototyping, but you'll want a config file in production to minimize code size.

### Module Switches

Edit `syn_config.h` to enable or disable modules:

```c
/* Drivers */
#define SYN_USE_GPIO       1
#define SYN_USE_UART       1
#define SYN_USE_ADC        1
#define SYN_USE_EXTI       1

/* Multitasking */
#define SYN_USE_PT         1
#define SYN_USE_SCHED      1
#define SYN_USE_TIMER      1
#define SYN_USE_EVENT      1
#define SYN_USE_WORKQUEUE  1

/* Services */
#define SYN_USE_LOG        1
#define SYN_USE_CLI        1

/* I/O */
#define SYN_USE_BUTTON     1
#define SYN_USE_LED        1
#define SYN_USE_ENCODER    1

/* Control & Motor */
#define SYN_USE_PID        1
#define SYN_USE_STEPPER    1
#define SYN_USE_SERVO      1
#define SYN_USE_DC_MOTOR   1
#define SYN_USE_MOTOR_CTRL 1

/* DSP */
#define SYN_USE_FILTER     1
#define SYN_USE_SIGNAL     1
#define SYN_USE_FSM        1

/* Communication */
#define SYN_USE_COBS       1
#define SYN_USE_MODBUS     1
#define SYN_USE_DALI       1
#define SYN_USE_BACNET     1
```


### Tuning Parameters

Beyond module switches, several modules expose tuning knobs:

| Parameter | Default | Description |
|---|---|---|
| `SYN_SCHED_PRIO_LEVELS` | `4` | Number of scheduler priority levels (`SYN_PRIO_CRITICAL = 0` .. `SYN_PRIO_LOW = 3`) |
| `SYN_SCHED_MAX_WAIT_CRITICAL_MS` | `1U` | Default max starvation wait window for `SYN_PRIO_CRITICAL` (ms) |
| `SYN_SCHED_MAX_WAIT_HIGH_MS` | `5U` | Default max starvation wait window for `SYN_PRIO_HIGH` (ms) |
| `SYN_SCHED_MAX_WAIT_NORMAL_MS` | `15U` | Default max starvation wait window for `SYN_PRIO_NORMAL` (ms) |
| `SYN_SCHED_MAX_WAIT_LOW_MS` | `50U` | Default max starvation wait window for `SYN_PRIO_LOW` (ms) |
| `SYN_LOG_LEVEL` | `1` (DEBUG) | Compile-time minimum log level (0=TRACE .. 5=FATAL, 6=NONE) |
| `SYN_LOG_BUF_SIZE` | `192` | Log output buffer size in bytes |
| `SYN_LOG_TIMESTAMP` | `1` | Include `[tick]` timestamp prefix in log output |
| `SYN_LOG_COLOR` | `0` | Enable ANSI color codes in log output |
| `SYN_CLI_LINE_BUF_SIZE` | `128` | Maximum command line length |
| `SYN_CLI_MAX_ARGS` | `16` | Maximum argc (including command name) |
| `SYN_CLI_HISTORY_DEPTH` | `0` | Command history depth (0 = disabled) |
| `SYN_UART_TX_BUF_SIZE` | `128` | UART TX ring buffer size in bytes |
| `SYN_UART_RX_BUF_SIZE` | `128` | UART RX ring buffer size in bytes |
| `SYN_UART_MAX_INSTANCES` | `2` | Maximum simultaneous UART handles |
| `SYN_FILTER_MAX_WINDOW` | `32` | Maximum filter window size |
| `SYN_CRC_USE_TABLE` | `1` | 1 = fast lookup table, 0 = small bitwise computation |
| `SYN_GFX_BACKEND` | `CANVAS` | Graphics backend: `SYN_GFX_BACKEND_CANVAS` (framebuffer) or `SYN_GFX_BACKEND_DIRECT` (no framebuffer) |

### Assert Configuration

SyntropicOS uses `SYN_ASSERT()` throughout for runtime safety checks. To strip all asserts in release builds:

```c
#define SYN_DISABLE_ASSERT
```

---

## Next Steps

- [Implement the port layer](porting-guide.md) for your MCU
- [Browse the module reference](modules/core.md) to see what's available
- [Run the test suite](testing.md) to verify your setup
