# Services

## Logging

| Module | Header | Config |
|---|---|---|
| Logging | `log/syn_log.h` | `SYN_USE_LOG` |
| Data Logger | `log/syn_datalog.h` | `SYN_USE_DATALOG` |

Severity-filtered logging with per-module tags, optional timestamps, and ANSI color output. Logging calls below the compile-time minimum level (`SYN_LOG_LEVEL`) are stripped entirely — zero overhead.

**Log levels** (0–6):

| Level | Macro | Value |
|---|---|---|
| Trace | `SYN_LOG_T(tag, fmt, ...)` | 0 |
| Debug | `SYN_LOG_D(tag, fmt, ...)` | 1 |
| Info | `SYN_LOG_I(tag, fmt, ...)` | 2 |
| Warn | `SYN_LOG_W(tag, fmt, ...)` | 3 |
| Error | `SYN_LOG_E(tag, fmt, ...)` | 4 |
| Fatal | `SYN_LOG_F(tag, fmt, ...)` | 5 |
| None (disable all) | — | 6 |

**Setup:**

```c
static void my_output(const char *str, size_t len) {
    syn_port_uart_transmit(0, (const uint8_t *)str, len, 0);
}

syn_log_init(my_output, SYN_LOG_DEBUG);

#define TAG "main"
SYN_LOG_I(TAG, "System started, version %d.%d", 1, 0);
SYN_LOG_E(TAG, "Sensor read failed: %d", status);
```

**Output:**
```
[   1234] I/main: System started, version 1.0
[   1240] E/main: Sensor read failed: -1
```

Additional utilities: `syn_log_raw()` for pre-formatted output, `syn_log_hexdump()` for hex + ASCII buffer dumps.

The **Data Logger** (`syn_datalog`) provides time-series binary data logging to a ring buffer, suitable for writing to flash storage.

---

## Command-Line Interface

| Module | Header | Config |
|---|---|---|
| CLI | `cli/syn_cli.h` | `SYN_USE_CLI` |

A lightweight interactive shell over UART or any byte stream. Commands are registered as a static table — no dynamic allocation. Input is processed one character at a time, making it easy to feed from a UART ISR or polled loop.

**Features:**

- Static command table (name + help string + handler function)
- Automatic `argc`/`argv` parsing (whitespace-delimited, with quoted strings)
- Built-in `help` command
- Backspace and line editing
- Optional command history (`SYN_CLI_HISTORY_DEPTH`)
- Echo control

**Setup:**

```c
static int cmd_led(int argc, char *argv[]);
static int cmd_reset(int argc, char *argv[]);

static const SYN_CLI_Command commands[] = {
    { "led",   "led <on|off>  — Control the LED",  cmd_led   },
    { "reset", "reset         — System reset",     cmd_reset },
};

static SYN_CLI cli;
syn_cli_init(&cli, commands, 2, my_putchar, "> ");

// In main loop or UART RX handler:
syn_cli_process_char(&cli, received_byte);
```

**Built-in diagnostic commands** (enabled by default, individually toggleable):

| Command | Config Define | Description |
|---|---|---|
| `version` | `SYN_CLI_CMD_VERSION` | Print `syn_version()` info |
| `uptime` | `SYN_CLI_CMD_UPTIME` | Print tick_ms uptime |
| `errors` | `SYN_CLI_CMD_ERRORS` | Dump errlog entries (requires `syn_cli_set_errlog()`) |
| `tasks` | `SYN_CLI_CMD_TASKS` | Show scheduler task states (requires `syn_cli_set_scheduler()`) |
