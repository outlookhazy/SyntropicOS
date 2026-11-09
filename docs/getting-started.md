# Getting Started with SyntropicOS

SyntropicOS is a static, zero-heap C99 cooperative operating system and driver/protocol framework for microcontrollers. It combines 2-byte stackless protothreads (`syn_pt`), a priority-ranked scheduler (`syn_sched`), and non-blocking peripheral, storage, crypto, network, and fieldbus state machines.

---

## 1. Core Architecture & Rules

### Stackless Protothreads (`syn_pt`)
Preemptive RTOS tasks allocate a dedicated hardware stack per thread (typically 512 B to 4 KB RAM each). SyntropicOS executes cooperative **protothreads** on the single main system stack.

- **RAM Cost**: **2 bytes** per standalone `SYN_PT` (`uint16_t lc` line continuation); **44 bytes** per scheduled `SYN_Task` descriptor on 32-bit targets.
- **Explicit Yield Intent**: `PT_BEGIN(pt)` and `PT_END(pt)` wrap a `switch (pt->lc)` continuation jump table:
  - `PT_PASS(pt)` and `PT_WAIT_UNTIL(pt, cond)` return `PT_IDLE` (no immediate work remaining this round).
  - `PT_CONTINUE(pt)` and `PT_CONTINUE_IF_NEEDED(pt, task)` return `PT_MORE` (work chunk completed; keep task in the scheduler's active `Work` rank until another task needs CPU time).
- **Timed Delays & True Blocking**:
  - `PT_TASK_DELAY_MS(pt, task, ms)` arms a wrap-safe millisecond deadline on `task->delay_until`.
  - `PT_BLOCK_EVENT(pt, task, &grp, mask)`, `PT_MUTEX_LOCK(pt, task, &mtx)`, `PT_BLOCK_STREAM(pt, task, &stream)`, and `PT_BLOCK_CONDITION(pt, task, cond)` park the task in `SYN_TASK_BLOCKED` so the scheduler skips polling it (and can enter tickless sleep) until an ISR or peer task wakes it.

!!! warning "Two Mandatory Protothread Rules"
    1. **Local Variable Lifetime**: Non-`static` local variables do **not** persist across yield points (`PT_PASS`, `PT_CONTINUE`, `PT_WAIT_*`, `PT_TASK_DELAY_MS`, `PT_BLOCK_*`). Keep persistent state in `static` variables or in a caller-owned context struct passed to `syn_task_create(..., user_data)`.
    2. **No `switch` Inside `PT_BEGIN`..`PT_END`**: Because `PT_BEGIN(pt)` expands to `switch (pt->lc)`, placing a C `switch` statement directly inside the protothread body corrupts continuation `case` labels. Use `if` / `else if` chains or move `switch` logic into a separate helper function.

### Zero-Heap Static Allocation
SyntropicOS never calls `malloc()`, `calloc()`, `realloc()`, or `free()`. Every task array (`SYN_Task`), scheduler (`SYN_Sched`), ring buffer (`SYN_RingBuf`), timer (`SYN_Timer`), and protocol context is allocated statically by the caller at compile time.

### Non-Blocking Drivers & Protocol Engines
Every peripheral driver (GPIO, UART, I2C, SPI, CAN, ADC, DMA, USB) and protocol stack (Ethernet/IP, TCP/UDP, TLS 1.3, WireGuard, BLE, Modbus, CANopen, J1939, UDS, BACnet, EtherCAT, MQTT, CoAP, LwM2M) is implemented as a non-blocking state machine exposing `_poll()`, `_update()`, or `_process()` steps that perform bounded work and return immediately.

---

## 2. Installation & Build Integration

**Compiler Prerequisite**: A C99 (or newer) compiler supporting `__has_include` — GCC >= 5 (`arm-none-eabi-gcc`, `riscv-none-embed-gcc`, host `gcc`), Clang / ARM Compiler 6, or IAR EWARM >= 8.

### Choose Your Setup Path

=== "CMake (Submodule)"

    Add SyntropicOS to your repository:

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
        # Add the reference port matching your MCU (or your own port_*.c):
        lib/SyntropicOS/src/port/stm32_hal/port_stm32_hal.c
    )

    # Put the directory containing your syn_config.h on the include path:
    target_include_directories(my_firmware PRIVATE include)

    # syntropic is an INTERFACE library compiled with your target's flags.
    # syn_stubs provides weak fallbacks for any port functions you don't implement.
    target_link_libraries(my_firmware PRIVATE syntropic syn_stubs)

    target_compile_options(my_firmware PRIVATE -ffunction-sections -fdata-sections)
    target_link_options(my_firmware PRIVATE -Wl,--gc-sections)
    ```

=== "Makefile (Submodule)"

    ```makefile
    SYN_DIR := lib/SyntropicOS
    include $(SYN_DIR)/sources.mk

    # SYN_INC expands to $(SYN_DIR)/src
    CFLAGS  += -std=c99 -ffunction-sections -fdata-sections -Iinclude -I$(SYN_INC)
    LDFLAGS += -Wl,--gc-sections

    SRCS += src/main.c
    SRCS += $(SYN_SRCS)
    SRCS += $(SYN_DIR)/src/port/stm32_hal/port_stm32_hal.c
    SRCS += $(SYN_STUB_SRCS)
    ```

    `sources.mk` exports:

    - `SYN_INC` — include root (`lib/SyntropicOS/src`) so `#include "syntropic/syntropic.h"` resolves
    - `SYN_SRCS` — all portable SyntropicOS `.c` source files
    - `SYN_STUB_SRCS` — `src/syntropic/port_stubs/syn_port_stubs.c`

=== "PlatformIO"

    SyntropicOS ships `library.json` for native PlatformIO integration. Add to `platformio.ini`:

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

    See [IDE Integration & Setup Guides](ide-guides.md) for STM32CubeIDE, VS Code + Cortex-Debug, Keil MDK-ARM, and IAR configurations.

=== "Arduino IDE"

    1. Open **Tools → Manage Libraries...** (`Ctrl+Shift+I`).
    2. Search for **SyntropicOS** and click **Install**.
    3. Open **File → Examples → SyntropicOS → Blink** to verify.

    See the [Arduino Compatibility Guide](arduino.md) for sketch structure and board notes.

=== "Manual / Vendor IDE"

    1. Add **`lib/SyntropicOS/src`** (not `lib/SyntropicOS/`) to your compiler include paths, alongside the folder containing your `syn_config.h`.
    2. Enable C99 mode (`-std=c99`) and dead-code stripping (`-ffunction-sections -fdata-sections` and `-Wl,--gc-sections`).
    3. Add `.c` sources from `lib/SyntropicOS/src/syntropic/` plus your platform port from `lib/SyntropicOS/src/port/<platform>/`.
    4. Optionally add `lib/SyntropicOS/src/syntropic/port_stubs/syn_port_stubs.c`.

### Ready-Made Hardware Ports (`src/port/`)

You do not need to write a port from scratch for supported targets — include the matching port source file from `src/port/`:

| Target Platform | Port Source | Notes |
|---|---|---|
| **STM32 (Cube HAL)** | `src/port/stm32_hal/port_stm32_hal.c` | Portable across STM32F0/F1/F4/F7/G0/H7/L4/C0/U0 |
| **STM32F4 (Bare-Metal)** | `src/port/stm32f4/port_stm32f4.c` | Direct register access, zero HAL dependency |
| **WCH CH32V307 (RISC-V)** | `src/port/ch32v307/port_ch32v307.c` | QingKe V4F bare-metal port (`-DCH32V307=ON` in CMake) |
| **Raspberry Pi RP2040 / RP2350** | `src/port/rp2040/port_rp2040.c` | Pico SDK (`+ port_rp2040_multicore.c` for dual-core AMP) |
| **ESP32** | `src/port/esp32/port_esp32.c` | ESP-IDF peripheral & tick port |
| **Arduino** | `src/port/arduino/port_arduino.cpp` | Arduino C++ SDK (`+ port_arduino_multicore.cpp` on RP2040) |
| **POSIX Host Sockets** | `src/port/posix/port_posix_socket.c` | Non-blocking TCP/UDP socket backend for Linux/macOS host runs |

!!! caution "How `syn_stubs` (`syn_port_stubs.c`) Behaves at Runtime"
    `syn_port_stubs.c` provides `SYN_WEAK` fallbacks so your firmware links even if you only implement the port headers you actually call (for example `syn_port_system.h` and `syn_port_gpio.h`, skipping SPI/CAN/Flash). **Weak stubs are not no-ops**: calling an unimplemented port function invokes `syn_assert_failed()`, which spins in `for (;;)` by default. Always link a real port implementation for every peripheral your `main()` initializes.

---

## 3. First Application Tutorials

=== "Host C99 Quickstart (No Hardware)"

    Try the scheduler, event-flag blocking (`PT_BLOCK_EVENT`), and structured logging on Linux or macOS in under a minute from a SyntropicOS checkout:

    Save as `host_demo.c` in the repository root:

    ```c
    #define _POSIX_C_SOURCE 199309L
    #include <stdio.h>
    #include <time.h>

    #include "syntropic/syntropic.h"

    /* ── Minimal host system port & console log sink ───────────────────────── */
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
        static uint32_t count;
        PT_BEGIN(pt);
        for (count = 1; count <= 3; count++) {
            PT_TASK_DELAY_MS(pt, task, 200);
            SYN_LOG_I("PROD", "Signaling ping #%lu", (unsigned long)count);
            syn_event_flags_set(&s_events, EVT_PING);
        }
        PT_END(pt);
    }

    static SYN_PT_Status consumer_task(SYN_PT *pt, SYN_Task *task) {
        static uint32_t received = 0;
        PT_BEGIN(pt);
        while (received < 3) {
            /* Parks task in SYN_TASK_BLOCKED until EVT_PING is set */
            PT_BLOCK_EVENT(pt, task, &s_events, EVT_PING);
            received++;
            SYN_LOG_I("CONS", "Woke on ping #%lu", (unsigned long)received);
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

        while (syn_sched_run(&sched)) {
            struct timespec nap = {0, 1000000L}; /* 1 ms host idle sleep */
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

=== "MCU Target (Blink + Serial Log)"

    Two-task firmware blinking an LED every 500 ms and emitting periodic status logs over the console UART (shown here with STM32Cube HAL + `port_stm32_hal.c`):

    ```c
    #include "syntropic/syntropic.h"

    /* Pin encoding depends on your target port:
     * - STM32 / CH32 ports pack (port_idx, pin_num): PA5 = SYN_GPIO_PIN(0, 5)
     * - RP2040 / ESP32 / Arduino ports use flat GPIO numbers (e.g. 25 or 13) */
    #define LED_PIN  SYN_GPIO_PIN(0, 5)
    #define LOG_TAG  "APP"

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
            SYN_LOG_I(LOG_TAG, "Uptime: %lu ms", (unsigned long)syn_port_get_tick_ms());
            PT_TASK_DELAY_MS(pt, task, 2000);
        }
        PT_END(pt);
    }

    int main(void) {
        static SYN_Task tasks[2];
        static SYN_Sched sched;

        /* 1. Initialize MCU clocks & 1 kHz SysTick so syn_port_get_tick_ms() advances */
        HAL_Init();
        SystemClock_Config();

        /* 2. Initialize GPIO and console serial before logging.
         *    Passing NULL to syn_log_init() routes output to syn_port_serial_write();
         *    pass a SYN_LogOutputFn callback instead for RTT, SWO/ITM, or USB CDC. */
        syn_gpio_init(LED_PIN, SYN_GPIO_OUTPUT);
        syn_port_serial_init(115200);
        syn_log_init(NULL, SYN_LOG_INFO);

        /* 3. Register static tasks (SYN_PRIO_CRITICAL = 0 highest .. SYN_PRIO_LOW = 3) */
        syn_task_create(&tasks[0], "blink", blink_task, SYN_PRIO_NORMAL, NULL);
        syn_task_create(&tasks[1], "log",   log_task,   SYN_PRIO_LOW,    NULL);

        /* 4. Bind task array and run cooperative scheduler loop */
        syn_sched_init(&sched, tasks, 2);
        syn_sched_run_forever(&sched);
    }
    ```

---

## 4. Configuration (`syn_config.h`) & Code Size

SyntropicOS headers and `.c` units auto-discover a user `syn_config.h` on the compiler include path via `#if __has_include("syn_config.h")` (or `-DSYN_CONFIG_FILE="my_config.h"` for `syn_defs.h`).

### Creating `syn_config.h`

```bash
cp lib/SyntropicOS/src/syntropic/syn_config_template.h your_project/include/syn_config.h
```

### Default-Enabled vs. Opt-In Modules

Standard modules follow an opt-out guard in `syntropic/syntropic.h` and their `.c` translation units:

```c
#if !defined(SYN_USE_GPIO) || SYN_USE_GPIO
#include "drivers/syn_gpio.h"
#endif
```

- **Standard modules (199 switches)**: Enabled (`1`) when `syn_config.h` is omitted or the macro is left undefined; set `#define SYN_USE_<MODULE> 0` to compile out both the header declarations and `.c` implementation.
- **Opt-in hardware/system features (9 switches)**: Default to **`0` (disabled)** until explicitly enabled (`#define SYN_USE_<FEATURE> 1`) because they require extra platform port hooks or reserve dedicated RAM/Flash:
  - `SYN_USE_TICKLESS` — Low-power tickless idle scheduler (`syn_sched_run_tickless*`; requires `syn_port_sleep_until()`)
  - `SYN_USE_MULTICORE` — Asymmetric multiprocessor (AMP) spinlocks and cross-core mailboxes (`syn_port_spinlock.h`)
  - `SYN_USE_COREDUMP` — Persistent crash core dump to flash (`SYN_COREDUMP_FLASH_ADDR`)
  - `SYN_USE_CAN_FD` — 64-byte CAN FD frame payloads in `syn_can`
  - `SYN_USE_DMA` — Hardware DMA channel & cache coherency port (`syn_port_dma.h`)
  - `SYN_USE_I2C_ASYNC` / `SYN_USE_SPI_ASYNC` — Non-blocking interrupt/DMA bus transaction queues
  - `SYN_USE_PORT_AES` — Hardware AES/GHASH crypto accelerator offload (`syn_port_aes.h`)
  - `SYN_USE_METRICS` — Runtime telemetry counters, gauges, and histograms

!!! tip "Minimizing Flash & Compile Time"
    `CMakeLists.txt` and `sources.mk` pass all `.c` files to the compiler so you never have to maintain file lists manually:
    - Always pass `-ffunction-sections -fdata-sections` (compile) and `-Wl,--gc-sections` (link) so unused functions are stripped from the final binary even without `syn_config.h`.
    - Copy `syn_config_template.h` (208 switches across all subsystems) and set unused subsystems to `0` to skip compiling unneeded modules and avoid pulling in unimplemented port symbols. See `examples/PicoBlink/syn_config.h` for a minimal bring-up profile.

### Representative Subsystem Switches

```c
/* Multitasking & Concurrency */
#define SYN_USE_PT           1  /* Stackless protothreads & semaphores      */
#define SYN_USE_SCHED        1  /* Ranked cooperative scheduler             */
#define SYN_USE_TIMER        1  /* One-shot & periodic software timers      */
#define SYN_USE_EVENT        1  /* 32-bit event flag groups                 */
#define SYN_USE_MUTEX        1  /* Priority-inheriting cooperative mutex    */
#define SYN_USE_WORKQUEUE    1  /* Deferred ISR-to-task work queue          */
#define SYN_USE_AO           1  /* Active Object (FSM + mailbox + task)     */
#define SYN_USE_TICKLESS     0  /* Opt-in: tickless sleep coordinator       */

/* Peripheral Drivers & USB */
#define SYN_USE_GPIO         1
#define SYN_USE_UART         1
#define SYN_USE_I2C          1
#define SYN_USE_SPI          1
#define SYN_USE_CAN          0
#define SYN_USE_ADC          0
#define SYN_USE_PWM          0
#define SYN_USE_USB_CDC      0  /* USB 2.0 CDC ACM, HID, MIDI, MSC, Host    */

/* Core Utilities & Data Codecs */
#define SYN_USE_FMT          1  /* Zero-heap string/number formatter        */
#define SYN_USE_FSM          1  /* Table-driven finite state machine        */
#define SYN_USE_POOL         1  /* Fixed-block static memory pool           */
#define SYN_USE_PUBSUB       0  /* Topic-based publish/subscribe bus        */
#define SYN_USE_JSON         0  /* Streaming JSON reader & writer           */
#define SYN_USE_CBOR         0  /* RFC 8949 CBOR encoder & decoder          */
#define SYN_USE_PROTOBUF     0  /* Protocol Buffers wire codec              */
#define SYN_USE_LZ4          0  /* Zero-heap LZ4 block compression          */

/* Services, Storage & System */
#define SYN_USE_LOG          1  /* Structured logging (requires SYN_USE_FMT)*/
#define SYN_USE_CLI          0  /* Interactive console shell                */
#define SYN_USE_PARAM        0  /* Wear-leveled flash parameter store       */
#define SYN_USE_SETTINGS     0  /* CRC-checked persistent settings          */
#define SYN_USE_VFS          0  /* VFS + LittleFS / FAT adapters            */
#define SYN_USE_BOOT         0  /* Boot manager & signed image verification */
#define SYN_USE_OTA          0  /* Streaming A/B OTA update orchestrator    */

/* Networking, Crypto, BLE & Fieldbus Protocols */
#define SYN_USE_ETH          0  /* Ethernet II, ARP, IPv4, ICMP, DHCP       */
#define SYN_USE_TLS          0  /* TLS 1.3 / DTLS 1.3 (RFC 8446 / 9147)     */
#define SYN_USE_HTTP         0  /* HTTP client, HTTPD server, WebSocket     */
#define SYN_USE_MQTT         0  /* MQTT 3.1.1, MQTT-SN, CoAP, LwM2M         */
#define SYN_USE_WG           0  /* WireGuard VPN tunnel                     */
#define SYN_USE_AES          0  /* AES-128/256, GCM, CCM, CMAC, SHA-2, ECC  */
#define SYN_USE_BLE          0  /* BLE Host stack (HCI, L2CAP, ATT, GATT)   */
#define SYN_USE_MODBUS       0  /* Plus CANopen, J1939, UDS, BACnet, etc.   */
```

### Key Tuning Parameters

| Parameter | Default | Description |
|---|---|---|
| `SYN_SCHED_PRIO_LEVELS` | `4` | Scheduler priority count (`SYN_PRIO_CRITICAL = 0` .. `SYN_PRIO_LOW = 3`, max `255`) |
| `SYN_SCHED_MAX_WAIT_CRITICAL_MS` | `1U` | Default max starvation window for `SYN_PRIO_CRITICAL` (ms) |
| `SYN_SCHED_MAX_WAIT_HIGH_MS` | `5U` | Default max starvation window for `SYN_PRIO_HIGH` (ms) |
| `SYN_SCHED_MAX_WAIT_NORMAL_MS` | `15U` | Default max starvation window for `SYN_PRIO_NORMAL` (ms) |
| `SYN_SCHED_MAX_WAIT_LOW_MS` | `50U` | Default max starvation window for `SYN_PRIO_LOW` (ms) |
| `SYN_MUTEX_MAX_WAITERS` | `4U` | Tracked contended waiters per `SYN_Mutex` for priority inheritance |
| `SYN_LOG_LEVEL` | `1` (`SYN_LOG_DEBUG`) | Compile-time minimum log level (`0` = TRACE .. `5` = FATAL, `6` = NONE) |
| `SYN_LOG_BUF_SIZE` | `192` | Stack formatting buffer size for `syn_log()` (bytes) |
| `SYN_LOG_TIMESTAMP` | `1` | Prefix log lines with `[tick]` millisecond timestamp |
| `SYN_LOG_COLOR` | `0` | Emit ANSI severity color codes |
| `SYN_CLI_LINE_BUF_SIZE` | `128` | Maximum CLI command line length (bytes) |
| `SYN_CLI_MAX_ARGS` | `16` | Maximum parsed CLI `argc` tokens |
| `SYN_CLI_HISTORY_DEPTH` | `0` | CLI command history ring depth (`0` = disabled) |
| `SYN_UART_TX_BUF_SIZE` / `RX_BUF_SIZE` | `128` | Per-instance UART ring buffer sizes (bytes) |
| `SYN_UART_MAX_INSTANCES` | `2` | Maximum simultaneous buffered `SYN_UART` handles |
| `SYN_FILTER_MAX_WINDOW` | `32` | Maximum sliding window length for `syn_filter` |
| `SYN_CRC_USE_TABLE` | `1` | `1` = lookup-table CRC, `0` = compact bitwise CRC |
| `SYN_GFX_BACKEND` | `SYN_GFX_BACKEND_CANVAS` | Framebuffer canvas renderer (`SYN_GFX_BACKEND_DIRECT` reserved) |
| `SYN_DISABLE_ASSERT` | *(undefined)* | Define to strip all runtime `SYN_ASSERT()` checks in release builds |

---

## Next Steps

- **[Browse the Module Catalog](modules/index.md)** — Complete directory of all 17 subsystem guides and 278 module headers.
- **[Explore Runnable Examples](https://github.com/outlookhazy/SyntropicOS/tree/main/examples)** — 100+ reference projects across STM32, RP2040, ESP32, CH32V307, and Arduino.
- **[Use or Write a Hardware Port](porting-guide.md)** — Reference ports in `src/port/` and HAL port contracts.
- **[IDE & Toolchain Setup Guides](ide-guides.md)** — STM32CubeIDE, VS Code (PlatformIO & CMake), Keil MDK, IAR, and [Arduino IDE](arduino.md).
- **[Contributor Verification & Test Suite](testing.md)** — Running containerized unit tests, QEMU/Renode emulation, sanitizers, and daemon integration tests inside a SyntropicOS checkout.
