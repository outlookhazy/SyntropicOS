# System Services & Utility Modules

SyntropicOS provides high-level system services for interactive command line interfaces, software watchdog monitoring, logging, and time synchronization.

---

## 1. Interactive Serial CLI (`cli/syn_cli.h`)

The `syn_cli` module enables zero-allocation, interactive command-line interfaces over UART or USB Virtual COM Port.

### Features
- Command registry with argument parsing (`argc`, `argv`).
- Built-in `help` command automatically listing all registered commands and descriptions.
- Backspace, Line-feed (`\r\n`), and prompt string management (`"> "`).

### Complete Code Example

```c
#include <syntropic/cli/syn_cli.h>
#include <syntropic/port/syn_port_serial.h>

static SYN_CLI cli;

// Command 1: Echo handler
static int cmd_echo(int argc, char *argv[]) {
    if (argc < 2) {
        syn_cli_printf(&cli, "Usage: echo <text>\r\n");
        return 1;
    }
    syn_cli_printf(&cli, "Echo: %s\r\n", argv[1]);
    return 0;
}

// Command 2: System info handler
static int cmd_info(int argc, char *argv[]) {
    (void)argc; (void)argv;
    syn_cli_printf(&cli, "SyntropicOS v1.0.0\r\nUptime: %lu ms\r\n",
                   (unsigned long)syn_port_get_tick_ms());
    return 0;
}

static const SYN_CLI_Command command_table[] = {
    { "echo", "Echo argument back to terminal", cmd_echo },
    { "info", "Print system version and uptime", cmd_info },
};

void app_init(void) {
    syn_port_serial_init(115200);
    
    // Initialize CLI with command table and prompt
    syn_cli_init(&cli, command_table, sizeof(command_table)/sizeof(command_table[0]), "> ");
    syn_cli_printf(&cli, "\r\n--- SyntropicOS Serial CLI ---\r\n");
    syn_cli_print_prompt(&cli);
}

void app_loop(void) {
    uint8_t rx_char;
    if (syn_port_serial_read(&rx_char, 1) > 0) {
        // Feed character into CLI state machine
        syn_cli_process_char(&cli, (char)rx_char);
    }
}
```

---

## 2. Multi-Task Software Watchdog (`sched/syn_watchdog.h`)

The software watchdog monitors per-task check-in deadlines and invokes a timeout callback (typically `syn_port_system_reset()`) if any registered task misses its window.

```c
#include <stdio.h>
#include <syntropic/port/syn_port_system.h>
#include <syntropic/sched/syn_watchdog.h>

static SYN_Watchdog wdt;
static SYN_WDT_Entry entries[3];
static int8_t sensor_wdt_id;

static void on_wdt_timeout(SYN_Watchdog *w, const SYN_WDT_Entry *entry, void *ctx) {
    (void)w;
    (void)ctx;
    printf("ERROR: Task '%s' missed %lu ms deadline! Resetting system...\n",
           entry->name, (unsigned long)entry->timeout_ms);
    syn_port_system_reset();
}

void task_worker(void) {
    // Reset check-in deadline for registered task
    syn_watchdog_checkin(&wdt, sensor_wdt_id);
}

void app_init(void) {
    // Initialize watchdog with 3 caller-owned entry slots and timeout callback
    syn_watchdog_init(&wdt, entries, 3, on_wdt_timeout, NULL);
    sensor_wdt_id = syn_watchdog_register(&wdt, "sensor", 2000);
}

void app_check_watchdog(void) {
    // Evaluate all active task deadlines
    syn_watchdog_update(&wdt);
}
```

---

## 3. Structured Logging, Binary Data Logger & Flight Blackbox (`log/*.h`)

- **Structured Console & Custom-Sink Logger (`log/syn_log.h`, `SYN_USE_LOG`)**:
  - Tagged, severity-filtered logging (`SYN_LOG_T`, `SYN_LOG_D`, `SYN_LOG_I`, `SYN_LOG_W`, `SYN_LOG_E`, `SYN_LOG_F`) plus `syn_log_raw()` and `syn_log_hexdump()`.
  - Calls below compile-time `SYN_LOG_LEVEL` (`0` = TRACE .. `5` = FATAL, `6` = NONE) compile out to `((void)0)`.
  - `syn_log_init(NULL, SYN_LOG_INFO)` routes output to `syn_port_serial_write()`; pass a `SYN_LogOutputFn` callback (`syn_log_init(my_sink_fn, SYN_LOG_INFO)`) for RTT, SWO/ITM, USB CDC, or file backends.
- **Framed Ring-Buffer Data Logger (`log/syn_datalog.h`, `SYN_USE_DATALOG`)**:
  - Records binary telemetry packets prefixed with `SYN_DataLogHeader` (`uint16_t id`, `uint16_t len`) into a caller-owned `SYN_RingBuf` (`syn_datalog_init`, `syn_datalog_write`, `syn_datalog_read`) with dropped-frame accounting (`syn_datalog_get_dropped`).
- **High-Rate Flight Telemetry Blackbox (`log/syn_blackbox.h`, `SYN_USE_BLACKBOX`)**:
  - Compresses `SYN_Blackbox_Record` frames (3-axis gyro/accel, 4 setpoints, 4 motor outputs, microsecond timestamp) into full Intra (`'I'`, `syn_blackbox_encode_intra_ex`) and ZigZag-LEB128 delta Predictive (`'P'`, `syn_blackbox_encode_delta_ex`) frames for flash or SD storage.

