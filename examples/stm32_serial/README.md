# SyntropicOS — STM32 Blue Pill Example

A complete example demonstrating SyntropicOS on the **STM32F103C8T6 "Blue Pill"** board  
using the STM32 HAL (stm32cube) framework.  
Connect a USB-to-serial adapter to USART1 at **115200 baud** to interact with the CLI.

## Features Demonstrated

| Module | What it shows |
|--------|--------------|
| **Scheduler** | Two cooperative protothread tasks with distinct priorities (`SYN_PRIO_HIGH` and `SYN_PRIO_LOW`) |
| **CLI** | Interactive command-line interface over USART1 |
| **LED** | Blink, flash, SOS Morse pattern via the `syn_led` driver |
| **FSM** | Three-state machine (IDLE → RUNNING → FAULT) |
| **Logging** | Structured `SYN_LOG` output with severity levels |
| **Port layer** | Using `syn_port_uart_*` functions with STM32 HAL underneath |

## Hardware Setup

```
STM32F103C8 "Blue Pill"
├── PC13 ── On-board LED (active-low, heartbeat)
├── PA9 ─── USART1 TX --> USB-serial adapter RX
└── PA10 ── USART1 RX <-- USB-serial adapter TX
```

Connect a USB-to-serial adapter (e.g. CP2102, FTDI, CH340) to PA9/PA10.  
Don't forget to share a common GND between the adapter and the Blue Pill.

## Build & Flash

```bash
pio run -d examples/stm32_serial                        # Build
pio run -d examples/stm32_serial -t upload              # Flash via ST-Link
pio device monitor -b 115200                            # Open serial monitor
```

> **Tip:** If you're using a USB-serial adapter instead of ST-Link for monitoring,  
> specify the port: `pio device monitor -b 115200 -p /dev/ttyUSB0`

## CLI Commands

```
> help              List available commands
> led on            Turn LED on (PC13 goes LOW — active-low)
> led off           Turn LED off
> led blink         Fast blink (200ms)
> led sos           Morse code SOS pattern
> fsm start         Transition FSM: IDLE → RUNNING
> fsm fault         Transition FSM: RUNNING → FAULT
> fsm stop          Transition FSM: → IDLE
> tasks             Show scheduler task list
```

## Code Structure

The example is a single file ([`src/main.c`](src/main.c)) organized into two halves:

### Application (portable SyntropicOS code)
1. **Application objects** — CLI, scheduler, tasks, LED
2. **FSM** — Transition table, state callbacks, and CLI command
3. **CLI commands** — `led` and `fsm` handlers
4. **Callbacks** — Serial output for CLI and logging
5. **Tasks** — `blink_task` (heartbeat + tick log), `cli_task` (serial polling)
6. **`setup()`** — Initializes all SyntropicOS modules in order

### HAL Boilerplate (board-specific)
7. **`main()`** — HAL init → clock config → GPIO/UART init → `setup()` → scheduler loop
8. **`SystemClock_Config()`** — HSE 8 MHz × PLL 9 = 72 MHz SYSCLK
9. **MSP functions** — GPIO alternate-function mapping for USART1

The split is intentional: the application code above the `HAL Boilerplate` comment
is portable and would look nearly identical on any SyntropicOS-supported platform.

## Task Layout

| Task | Priority | Rate | Purpose |
|------|----------|------|---------|
| `blink` | `SYN_PRIO_LOW` (`3`) | 50 ms | LED animation + periodic tick log (`PT_TASK_DELAY_MS`) |
| `cli` | `SYN_PRIO_HIGH` (`1`) | every cycle | Poll USART1 (`PT_CONTINUE_IF_NEEDED` while RX bytes arrive, `PT_PASS` when empty) |

`cli` runs at `SYN_PRIO_HIGH` (`1`) for responsive serial input while yielding via `PT_CONTINUE_IF_NEEDED` when `blink` (`SYN_PRIO_LOW`, `3`) reaches its deadline, and `PT_PASS` once the RX buffer is empty.
