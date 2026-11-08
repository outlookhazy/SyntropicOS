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
