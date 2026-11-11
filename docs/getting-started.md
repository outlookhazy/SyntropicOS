# Getting Started with SyntropicOS

SyntropicOS is a C99 cooperative multitasking operating system, driver layer, and protocol framework for microcontrollers. Tasks run on the main stack with **zero heap allocation** (`malloc` is never called) and **2 bytes of continuation state** per protothread.

**Choose where to start:**

- **No hardware handy?** Run a two-task demo on Linux or macOS in **[Section 1](#1-try-it-in-60-seconds-no-hardware)**.
- **New to protothreads?** Read **[Section 2: How Tasks Work (3-Minute Tour)](#2-how-tasks-work-3-minute-tour)**.
- **Setting up a board or IDE?** Jump to **[Section 3: Add to Your Project](#3-add-to-your-project)** and **[Section 4: First MCU Firmware](#4-first-mcu-firmware-blink-and-serial-log)**.

---

## 1. Try It in 60 Seconds (No Hardware)

You can run the scheduler, timed delays (`PT_TASK_DELAY_MS`), event-flag blocking (`PT_BLOCK_EVENT`), and structured logging on Linux or macOS with any C99 compiler (`gcc` or `clang`).

From a SyntropicOS checkout, save this file as `host_demo.c`:

```c
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <time.h>

#include "syntropic/syntropic.h"

/* ── Minimal host system port & console output ─────────────────────────── */
void syn_port_enter_critical(void) {}
void syn_port_exit_critical(void) {}

uint32_t syn_port_get_tick_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)((uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL);
}

static void console_out(const char *str, size_t len) {
    (void)fwrite(str, 1, len, stdout);
    (void)fflush(stdout);
}

/* ── Application tasks ─────────────────────────────────────────────────── */
#define EVT_PING (1UL << 0)
static SYN_EventFlags s_events;

static SYN_PT_Status producer_task(SYN_PT *pt, SYN_Task *task) {
    static uint32_t sent;
    PT_BEGIN(pt);
    for (sent = 1; sent <= 3; sent++) {
        PT_TASK_DELAY_MS(pt, task, 200);          /* Pause 200 ms without blocking */
        SYN_LOG_I("PROD", "Ping #%lu", (unsigned long)sent);
        syn_event_flags_set(&s_events, EVT_PING); /* Wake consumer_task */
    }
    PT_END(pt);
}

static SYN_PT_Status consumer_task(SYN_PT *pt, SYN_Task *task) {
    static uint32_t received = 0;
    PT_BEGIN(pt);
    while (received < 3) {
        PT_BLOCK_EVENT(pt, task, &s_events, EVT_PING); /* Sleep until EVT_PING */
        received++;
        SYN_LOG_I("CONS", "Got ping #%lu", (unsigned long)received);
    }
    PT_END(pt);
}

int main(void) {
    static SYN_Task tasks[2];
    static SYN_Sched sched;

    syn_log_init(console_out, SYN_LOG_INFO);
    syn_event_flags_init(&s_events);

    syn_task_create(&tasks[0], "producer", producer_task, SYN_PRIO_NORMAL, NULL);
    syn_task_create(&tasks[1], "consumer", consumer_task, SYN_PRIO_HIGH,   NULL);
    syn_sched_init(&sched, tasks, 2);

    /* Returns false once both tasks reach PT_END(pt) */
    while (syn_sched_run(&sched)) {
        struct timespec nap = {0, 1000000L}; /* 1 ms idle sleep */
        nanosleep(&nap, NULL);
    }
    return 0;
}
```

Compile and run:

```bash
cc -std=c99 -Wall -Wextra -Isrc host_demo.c \
  src/syntropic/sched/syn_sched.c \
  src/syntropic/sched/syn_mutex.c \
  src/syntropic/sched/syn_event_flags.c \
  src/syntropic/log/syn_log.c \
  src/syntropic/util/syn_fmt.c \
  src/syntropic/port_stubs/syn_port_stubs.c \
  -o host_demo && ./host_demo
```

Expected output:

```text
[     200] I/PROD: Ping #1
[     201] I/CONS: Got ping #1
[     401] I/PROD: Ping #2
[     402] I/CONS: Got ping #2
[     602] I/PROD: Ping #3
[     603] I/CONS: Got ping #3
```

**What happened:**

1. Both tasks ran sequentially on the single main stack — no OS threads and no `malloc()`.
2. `PT_TASK_DELAY_MS` paused `producer_task` for 200 ms while letting the rest of the program run.
3. `PT_BLOCK_EVENT` parked `consumer_task` in `SYN_TASK_BLOCKED` (zero polling) until `syn_event_flags_set()` woke it.

---

## 2. How Tasks Work (3-Minute Tour)

### Anatomy of a Task

Think of each task as an independent `for (;;)` loop that pauses cooperatively instead of calling a blocking `delay()`:

```c
static SYN_PT_Status my_task(SYN_PT *pt, SYN_Task *task) {
    PT_BEGIN(pt);                        /* 1. Required at top of task body */

    for (;;) {
        do_some_nonblocking_work();
        PT_TASK_DELAY_MS(pt, task, 100); /* 2. Bookmark line & return to scheduler */
    }

    PT_END(pt);                          /* 3. Required at bottom of task body */
}
```

Preemptive RTOS threads allocate a separate 512 B–4 KB stack per task. A SyntropicOS protothread (`SYN_PT`) stores a **2-byte bookmark** (`uint16_t` line number; **44 bytes** total per scheduled `SYN_Task` on 32-bit MCUs). When a task hits a `PT_*` macro, it saves its line number and returns to `syn_sched_run()`. On its next turn, it jumps straight back to that bookmark.

### Which Pause Macro Should I Use?

| Goal | Macro | Behavior |
|---|---|---|
| Wait `ms` milliseconds | `PT_TASK_DELAY_MS(pt, task, ms)` | Pauses this task for at least `ms`; other tasks keep running. |
| Sleep until signaled by ISR or task | `PT_BLOCK_EVENT(pt, task, &ev, mask)` | Parks task (`SYN_TASK_BLOCKED`, zero polling) until event bit is set. Related: `PT_MUTEX_LOCK`, `PT_BLOCK_STREAM`, `PT_BLOCK_CONDITION`. |
| Wait until an expression is true | `PT_WAIT_UNTIL(pt, cond)` | Checks `cond` once per scheduler round until true. |
| Yield for the rest of this round | `PT_PASS(pt)` | Reports no more work this round (`PT_IDLE`). |
| Slice a long loop (DSP, crypto, I/O) | `PT_CONTINUE_IF_NEEDED(pt, task)` | Yields (`PT_MORE`) **only** when a peer task needs CPU time. |

### Keeping State Across Pauses (The Two Rules)

Because a task returns from the C function on every pause, keep two rules in mind:

1. **Stack variables reset across pauses.** Store values that must survive across a `PT_*` yield in a **context struct passed via `user_data`** (best when running multiple instances of a task) or in **`static` variables** (fine for single-instance tasks). Re-reading `task->user_data` *above* `PT_BEGIN(pt)` is safe because the function runs from the top before jumping to its bookmark.
2. **Do not place a C `switch` directly inside `PT_BEGIN`..`PT_END`.** (`PT_BEGIN` expands to a `switch` jump table.) Use `if` / `else if`, or put `switch` statements inside a normal helper function.

```c
typedef struct {
    SYN_GPIO_Pin pin;
    uint32_t period_ms;
    uint32_t toggles;
} BlinkCtx;

static void log_milestone(uint32_t count) {
    switch (count) { /* switch inside a helper function is always fine */
    case 10: SYN_LOG_I("LED", "10 toggles"); break;
    default: break;
    }
}

static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task) {
    BlinkCtx *ctx = (BlinkCtx *)task->user_data; /* Re-fetched on every wake */

    PT_BEGIN(pt);
    for (;;) {
        syn_gpio_toggle(ctx->pin);
        ctx->toggles++;                  /* Persists inside caller-owned BlinkCtx */
        log_milestone(ctx->toggles);
        PT_TASK_DELAY_MS(pt, task, ctx->period_ms);
    }
    PT_END(pt);
}
```

With `user_data`, two independent tasks can share `blink_task`:

```c
static BlinkCtx fast_led = { .pin = LED_A_PIN, .period_ms = 200,  .toggles = 0 };
static BlinkCtx slow_led = { .pin = LED_B_PIN, .period_ms = 1000, .toggles = 0 };

syn_task_create(&tasks[0], "led_fast", blink_task, SYN_PRIO_NORMAL, &fast_led);
syn_task_create(&tasks[1], "led_slow", blink_task, SYN_PRIO_LOW,    &slow_led);
```

---

## 3. Add to Your Project

**Compiler Requirement**: C99 or newer with `__has_include` support — GCC >= 5 (`arm-none-eabi-gcc`, `riscv-none-embed-gcc`, host `gcc`), Clang / ARM Compiler 6, IAR EWARM >= 8, or Arduino IDE.

=== "Arduino IDE"

    1. Open **Tools → Manage Libraries...** (`Ctrl+Shift+I`).
    2. Search for **SyntropicOS** and click **Install**.
    3. Open **File → Examples → SyntropicOS → Blink** and upload to your board.

    See the [Arduino Compatibility Guide](arduino.md) for multi-tab sketch patterns and board notes.

=== "PlatformIO"

    Add `SyntropicOS` to `platformio.ini`:

    ```ini
    [env:disco_f407vg]
    platform = ststm32
    board = disco_f407vg
    framework = stm32cube
    build_flags =
        -std=c99
        -Iinclude
    lib_deps =
        https://github.com/outlookhazy/SyntropicOS.git
    ```

=== "CMake (Submodule)"

    Add the submodule to your project:

    ```bash
    git submodule add https://github.com/outlookhazy/SyntropicOS lib/SyntropicOS
    git submodule update --init
    ```

    In `CMakeLists.txt`:

    ```cmake
    cmake_minimum_required(VERSION 3.16)
    project(my_firmware LANGUAGES C)
    set(CMAKE_C_STANDARD 99)

    add_subdirectory(lib/SyntropicOS)

    add_executable(my_firmware
        src/main.c
        # Pick the port matching your target from src/port/:
        lib/SyntropicOS/src/port/stm32_hal/port_stm32_hal.c
    )

    # Optional: directory holding your syn_config.h (if you use one)
    target_include_directories(my_firmware PRIVATE include)

    # Define your MCU family when using port_stm32_hal.c outside CubeIDE/PlatformIO:
    target_compile_definitions(my_firmware PRIVATE STM32F4xx)

    # syntropic = portable headers & sources; syn_stubs = weak fallbacks for unused ports
    target_link_libraries(my_firmware PRIVATE syntropic syn_stubs)

    # Strip unused functions and data at link time
    target_compile_options(my_firmware PRIVATE -ffunction-sections -fdata-sections)
    target_link_options(my_firmware PRIVATE -Wl,--gc-sections)
    ```

=== "Makefile (Submodule)"

    `sources.mk` exports `SYN_INC` (`lib/SyntropicOS/src`), `SYN_SRCS` (all portable `.c` modules), and `SYN_STUB_SRCS` (`syn_port_stubs.c`):

    ```makefile
    SYN_DIR := lib/SyntropicOS
    include $(SYN_DIR)/sources.mk

    CFLAGS  += -std=c99 -ffunction-sections -fdata-sections -DSTM32F4xx -Iinclude -I$(SYN_INC)
    LDFLAGS += -Wl,--gc-sections

    SRCS += src/main.c
    SRCS += $(SYN_SRCS)
    SRCS += $(SYN_DIR)/src/port/stm32_hal/port_stm32_hal.c
    SRCS += $(SYN_STUB_SRCS)
    ```

=== "STM32CubeIDE / Keil / IAR"

    1. Add **`SyntropicOS/src`** (not the repo root) to your compiler include paths so `#include "syntropic/syntropic.h"` resolves.
    2. Enable **C99** mode and dead-code stripping (`-ffunction-sections -fdata-sections` and `-Wl,--gc-sections`).
    3. Compile `src/syntropic/**.c`, your target port from `src/port/<platform>/`, and `src/syntropic/port_stubs/syn_port_stubs.c`.

    See [IDE Setup Guides](ide-guides.md) for click-by-click screenshots/paths in STM32CubeIDE, VS Code + Cortex-Debug, Keil uVision, and IAR.

### Ready-Made Hardware Ports (`src/port/`)

Include the port file matching your target — no custom HAL code is needed for supported boards:

| Target Platform | Port Source | Target Notes |
|---|---|---|
| **STM32 (Cube HAL)** | `src/port/stm32_hal/port_stm32_hal.c` | Supports F0/F1/F4/F7/G0/H7/L4/C0/U0. Include `port/stm32_hal/port_stm32_hal.h` for `SYN_PORT_STM32_PIN()` and `syn_port_stm32_register_uart()`. |
| **STM32F4 (Bare-Metal)** | `src/port/stm32f4/port_stm32f4.c` | Direct CMSIS register port (zero HAL dependency; includes `startup_stm32f407.c` & `stm32f407.ld`). |
| **Raspberry Pi RP2040 / RP2350** | `src/port/rp2040/port_rp2040.c` | Pico SDK (`-DPICO_BOARD=...`; add `port_rp2040_multicore.c` when `SYN_USE_MULTICORE=1`). |
| **ESP32** | `src/port/esp32/port_esp32.c` | ESP-IDF GPIO, UART, SPI, I2C, ADC, flash, and system tick port. |
| **WCH CH32V307 (RISC-V)** | `src/port/ch32v307/port_ch32v307.c` | QingKe V4F bare-metal port (`-DCH32V307=ON` in CMake). |
| **Arduino** | `src/port/arduino/port_arduino.cpp` | Cross-board Arduino C++ core (`+ port_arduino_multicore.cpp` on Arduino-Pico). |
| **POSIX Host Sockets** | `src/port/posix/port_posix_socket.c` | Non-blocking TCP/UDP socket backend for Linux/macOS host testing. |

For custom MCUs, see the [Porting Guide](porting-guide.md) — a minimal scheduler + GPIO bring-up only requires `syn_port_system.h` and `syn_port_gpio.h`.

!!! note "What `syn_stubs` (`syn_port_stubs.c`) Does"
    Linking `syn_port_stubs.c` provides `SYN_WEAK` defaults for every `syn_port_*` function so your project links even if your port only implements GPIO and UART (skipping CAN, SPI, or Flash). **Stubs are fail-fast traps, not silent no-ops**: calling an unimplemented port function invokes `syn_assert_failed(__FILE__, __LINE__)`.

---

## 4. First MCU Firmware (Blink and Serial Log)

Two cooperative tasks — one blinking an LED every 500 ms, one logging uptime every 2 seconds:

=== "STM32 (Cube HAL)"

    ```c
    #include "main.h" /* CubeMX-generated clocks, GPIO, and UART handles */
    #include "syntropic/syntropic.h"
    #include "port/stm32_hal/port_stm32_hal.h"

    extern UART_HandleTypeDef huart2;

    /* Works with CubeMX labels (LD2_GPIO_Port, LD2_Pin) or raw (GPIOA, GPIO_PIN_5) */
    #define LED_PIN SYN_PORT_STM32_PIN(GPIOA, GPIO_PIN_5)

    static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task) {
        PT_BEGIN(pt);
        for (;;) {
            syn_gpio_toggle(LED_PIN);
            PT_TASK_DELAY_MS(pt, task, 500);
        }
        PT_END(pt);
    }

    static SYN_PT_Status log_task(SYN_PT *pt, SYN_Task *task) {
        PT_BEGIN(pt);
        for (;;) {
            SYN_LOG_I("APP", "Uptime: %lu ms", (unsigned long)syn_port_get_tick_ms());
            PT_TASK_DELAY_MS(pt, task, 2000);
        }
        PT_END(pt);
    }

    int main(void) {
        static SYN_Task tasks[2];
        static SYN_Sched sched;

        /* 1. Standard CubeMX clock + UART init (1 kHz SysTick feeds syn_port_get_tick_ms) */
        HAL_Init();
        SystemClock_Config();
        MX_USART2_UART_Init();

        /* 2. Bind huart2 to UART slot 0 for console logging, and configure LED pin */
        syn_port_stm32_register_uart(0, &huart2);
        syn_port_serial_init(115200);
        syn_log_init(SYN_LOG_INFO);
        syn_gpio_init(LED_PIN, SYN_GPIO_OUTPUT);

        /* 3. Register static tasks (SYN_PRIO_CRITICAL = 0 highest .. SYN_PRIO_LOW = 3) */
        syn_task_create(&tasks[0], "blink", blink_task, SYN_PRIO_NORMAL, NULL);
        syn_task_create(&tasks[1], "log",   log_task,   SYN_PRIO_LOW,    NULL);

        /* 4. Start cooperative scheduler */
        syn_sched_init(&sched, tasks, 2);
        syn_sched_run_forever(&sched);
    }
    ```

=== "Raspberry Pi Pico (Pico SDK)"

    ```c
    #include "pico/stdlib.h"
    #include "syntropic/syntropic.h"

    #define LED_PIN 25 /* On-board GP25 LED on Pico */

    static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task) {
        PT_BEGIN(pt);
        for (;;) {
            syn_gpio_toggle(LED_PIN);
            PT_TASK_DELAY_MS(pt, task, 500);
        }
        PT_END(pt);
    }

    static SYN_PT_Status log_task(SYN_PT *pt, SYN_Task *task) {
        PT_BEGIN(pt);
        for (;;) {
            SYN_LOG_I("PICO", "Uptime: %lu ms", (unsigned long)syn_port_get_tick_ms());
            PT_TASK_DELAY_MS(pt, task, 2000);
        }
        PT_END(pt);
    }

    int main(void) {
        static SYN_Task tasks[2];
        static SYN_Sched sched;

        stdio_init_all(); /* Initializes USB CDC / UART stdio used by port_rp2040.c */
        syn_port_serial_init(115200);
        syn_log_init(SYN_LOG_INFO);
        syn_gpio_init(LED_PIN, SYN_GPIO_OUTPUT);

        syn_task_create(&tasks[0], "blink", blink_task, SYN_PRIO_NORMAL, NULL);
        syn_task_create(&tasks[1], "log",   log_task,   SYN_PRIO_LOW,    NULL);

        syn_sched_init(&sched, tasks, 2);
        syn_sched_run_forever(&sched);
    }
    ```

=== "Arduino / ESP32 / STM32duino"

    ```cpp
    #include <SyntropicOS.h>

    static SYN_Task tasks[2];
    static SYN_Sched sched;

    static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task) {
        PT_BEGIN(pt);
        for (;;) {
            syn_gpio_toggle(LED_BUILTIN);
            PT_TASK_DELAY_MS(pt, task, 500);
        }
        PT_END(pt);
    }

    static SYN_PT_Status log_task(SYN_PT *pt, SYN_Task *task) {
        PT_BEGIN(pt);
        for (;;) {
            SYN_LOG_I("APP", "Uptime: %lu ms", (unsigned long)syn_port_get_tick_ms());
            PT_TASK_DELAY_MS(pt, task, 2000);
        }
        PT_END(pt);
    }

    void setup() {
        syn_port_serial_init(115200);
        syn_log_init(SYN_LOG_INFO);
        syn_gpio_init(LED_BUILTIN, SYN_GPIO_OUTPUT);

        syn_task_create(&tasks[0], "blink", blink_task, SYN_PRIO_NORMAL, NULL);
        syn_task_create(&tasks[1], "log",   log_task,   SYN_PRIO_LOW,    NULL);
        syn_sched_init(&sched, tasks, 2);
    }

    void loop() {
        syn_sched_run(&sched);
    }
    ```

---

## 5. Optional Configuration (`syn_config.h`)

**You do not need `syn_config.h` to get started.** All 199 standard modules are enabled by default, and compiling with `-ffunction-sections -fdata-sections` plus `-Wl,--gc-sections` strips unused code automatically.

Create `syn_config.h` on your include path (auto-detected via `#if __has_include("syn_config.h")`) when you want to:

1. **Speed up builds or save static RAM** by compiling out unused subsystems (`#define SYN_USE_BLE 0`, `#define SYN_USE_TLS 0`, etc.).
2. **Tune static buffer sizes** (for example `SYN_LOG_BUF_SIZE` or `SYN_UART_RX_BUF_SIZE` on small-RAM MCUs).
3. **Enable any of the 9 opt-in hardware features** (off by default because they require extra port hooks or dedicated RAM/Flash):
   - `SYN_USE_TICKLESS` — Low-power tickless idle sleep (`syn_sched_run_tickless*`; requires `syn_port_sleep_until()`)
   - `SYN_USE_MULTICORE` — Dual-core AMP spinlocks & cross-core mailboxes (`syn_port_spinlock.h`)
   - `SYN_USE_DMA` — Hardware DMA & cache coherency port (`syn_port_dma.h`)
   - `SYN_USE_I2C_ASYNC` / `SYN_USE_SPI_ASYNC` — Interrupt/DMA non-blocking bus queues
   - `SYN_USE_CAN_FD` — 64-byte CAN FD payloads in `syn_can`
   - `SYN_USE_PORT_AES` — Hardware AES/GHASH offload (`syn_port_aes.h`)
   - `SYN_USE_COREDUMP` — Crash core dump to flash (`SYN_COREDUMP_FLASH_ADDR`)
   - `SYN_USE_METRICS` — Runtime counters, gauges, and histograms

Copy the documented template into your include folder:

```bash
cp lib/SyntropicOS/src/syntropic/syn_config_template.h include/syn_config.h
```

??? info "Common `syn_config.h` Tuning Parameters & Defaults"

    | Parameter | Default | Description |
    |---|---|---|
    | `SYN_SCHED_PRIO_LEVELS` | `4` | Priority levels (`SYN_PRIO_CRITICAL = 0` .. `SYN_PRIO_LOW = 3`, max `255`) |
    | `SYN_SCHED_MAX_WAIT_*_MS` | `1 / 5 / 15 / 50` | Max starvation window per priority (`CRITICAL` / `HIGH` / `NORMAL` / `LOW`) |
    | `SYN_MUTEX_MAX_WAITERS` | `4U` | Tracked contended waiters per `SYN_Mutex` for priority inheritance |
    | `SYN_LOG_LEVEL` | `SYN_LOG_DEBUG` (`1`) | Compile-time minimum log level (`0` = TRACE .. `5` = FATAL, `6` = NONE) |
    | `SYN_LOG_BUF_SIZE` | `192` | Stack buffer size for `syn_log()` formatting (bytes) |
    | `SYN_LOG_TIMESTAMP` | `1` | Prefix log lines with `[tick]` millisecond timestamp |
    | `SYN_LOG_COLOR` | `0` | Emit ANSI color codes by severity |
    | `SYN_CLI_LINE_BUF_SIZE` | `128` | Maximum CLI command line length (bytes) |
    | `SYN_CLI_HISTORY_DEPTH` | `0` | CLI command history ring depth (`0` = disabled) |
    | `SYN_UART_TX_BUF_SIZE` / `RX_BUF_SIZE` | `128` | Per-instance buffered `SYN_UART` ring sizes (bytes) |
    | `SYN_UART_MAX_INSTANCES` | `2` | Maximum simultaneous buffered `SYN_UART` instances |
    | `SYN_CRC_USE_TABLE` | `1` | `1` = lookup-table CRC, `0` = compact bitwise CRC |
    | `SYN_DISABLE_ASSERT` | *(undefined)* | Define to strip runtime `SYN_ASSERT()` checks in release builds |

    See `examples/PicoBlink/syn_config.h` for a minimal bring-up configuration and `src/syntropic/syn_config_template.h` for all 208 switches.

---

## 6. First-Boot Troubleshooting

- **Firmware halts immediately in an infinite loop**: A `SYN_ASSERT()` failed or your code called an unimplemented `syn_port_*` weak stub. Set a breakpoint on `syn_assert_failed`, or define a strong handler in `main.c` to print the offending file and line:
  ```c
  void syn_assert_failed(const char *file, int line) {
      /* Print over UART/SWO/RTT or inspect 'file' and 'line' in debugger */
      (void)file;
      (void)line;
      for (;;) {}
  }
  ```
- **`PT_TASK_DELAY_MS` pauses forever**: `syn_port_get_tick_ms()` is stuck at `0`. Verify your 1 kHz timer interrupt is enabled (for example `SysTick_Handler` calling `HAL_IncTick()` on STM32).
- **No `SYN_LOG_*` output on STM32Cube HAL**: Call `syn_port_stm32_register_uart(0, &huartX)` before `syn_port_serial_init()`, or pass a custom sink callback (`syn_log_init(my_rtt_write, SYN_LOG_INFO)`).
- **`syn_config.h` in an Arduino sketch folder has no effect on library `.c` files**: Arduino IDE compiles libraries without `-I<sketch_dir>`. Place `syn_config.h` in `libraries/SyntropicOS/src/` or use PlatformIO `build_flags` (see [Arduino Guide](arduino.md#disabling-unused-modules)).

---

## Next Steps

- **[Module Catalog](modules/index.md)** — Browse all 17 subsystem guides (I/O, CLI, Display/IMGUI, Storage, DSP, Motor Control, Networking, Crypto, BLE, Fieldbuses).
- **[Examples Directory](https://github.com/outlookhazy/SyntropicOS/tree/main/examples)** — 100+ complete projects across STM32, RP2040, ESP32, CH32V307, and Arduino.
- **[IDE Setup Guides](ide-guides.md)** & **[Arduino Guide](arduino.md)** — Toolchain-specific project configurations.
- **[Porting Guide](porting-guide.md)** — Implementing `syn_port_*` contracts on custom hardware.
- **[Testing Guide](testing.md)** — Running containerized unit tests, QEMU/Renode emulation, and sanitizers.
