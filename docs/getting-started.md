# Getting Started

## Adding SyntropicOS to Your Project

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

    # Optional: include weak port stubs for development
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
    - `SYN_INC` — include path (the repo root)

=== "Manual / IDE"

    1. Add `lib/SyntropicOS/` to your include paths.
    2. Add the `.c` files from the `syntropic/` subdirectories you need to your build.
    3. Optionally add `syntropic/port_stubs/syn_port_stubs.c`.

---

## Configuration

SyntropicOS is configured through a single header file, `syn_config.h`, placed on your include path.

### Creating Your Config

```bash
cp lib/SyntropicOS/syntropic/syn_config_template.h your_project/include/syn_config.h
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
```

### Tuning Parameters

Beyond module switches, several modules expose tuning knobs:

| Parameter | Default | Description |
|---|---|---|
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

## Quick Example

A minimal blink task using the cooperative scheduler:

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

**What's happening here:**

1. `PT_BEGIN` / `PT_END` wrap the protothread body. The `SYN_PT` struct costs **2 bytes of RAM** — it stores a `uint16_t` line continuation using Duff's device (`switch`/`__LINE__`).
2. `PT_TASK_DELAY_MS` saves a deadline tick in `task->delay_until` and yields. On each subsequent scheduler tick, the protothread resumes at this line and checks if `syn_port_get_tick_ms() >= deadline`. No blocking, no busy-wait.
3. `syn_task_create` sets priority `0` (highest). The scheduler runs all ready tasks per tick in priority order, with round-robin among equal priorities.
4. `syn_sched_run_forever` loops forever calling `syn_sched_run()`. The scheduler does **not** own the task array — you allocate it, keeping everything on the stack or in static memory.

!!! tip "Local variables in protothreads"
    Local variables are **not preserved** across yield/wait points (because there's no stack save). Store persistent state in `static` variables or in a struct passed via `task->user_data`.

---

## Next Steps

- [Implement the port layer](porting-guide.md) for your MCU
- [Browse the module reference](modules/core.md) to see what's available
- [Run the test suite](testing.md) to verify your setup
