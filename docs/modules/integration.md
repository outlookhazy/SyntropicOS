# Cross-Module Integration

SyntropicOS modules compose with each other where it makes sense. These integrations are automatic when both modules are enabled — no extra wiring code required.

## Integration Map

| Source Module | Target Module | What Happens |
|---|---|---|
| **Sensor** | **Signal Stats** | Attach a `SYN_Signal` window to any sensor for automatic sliding-window min/max/mean/variance |
| **Motor Ctrl** | **ErrLog** | Stall detection and limit events auto-record if the motor controller's `.errlog` field is set |
| **Watchdog** | **ErrLog** | Task timeout events auto-record if `.errlog` is configured |
| **Boot Manager** | **ErrLog** | Crash-loop and safe-mode events logged via `syn_boot_log_events()` |
| **CLI** | **Version** | Built-in `version` command prints `syn_version()` info |
| **CLI** | **ErrLog** | Built-in `errors` command dumps errlog entries (requires `syn_cli_set_errlog()`) |
| **CLI** | **Scheduler** | Built-in `tasks` command shows task states (requires `syn_cli_set_scheduler()`) |
| **Modbus** | **CRC** | Uses `syn_crc16_modbus()` for frame integrity |
| **Param Store** | **CRC** | Uses `syn_crc16_ccitt()` for data integrity |
| **Profiler** | **Fmt** | Uses `syn_fmt` instead of `snprintf` (no stdio.h dependency) |
| **Trace** | **Fmt** | Uses `syn_fmt` for formatted output |
| **CLI** | **Fmt** | Uses `syn_fmt` for `syn_cli_printf()` |
| **Log** | **Fmt** | Uses `syn_fmt` for log message formatting |
| **Active Object** | **FSM** | Combines FSM state machine + event queue + scheduler task into a single execution context |
| **Button** | **FSM** | Uses `syn_fsm` internally for debounce state tracking |
