# Cross-Module System Integration

SyntropicOS modules compose directly with each other. When multiple modules are enabled in `syn_config.h`, cross-module integrations automatically hook together without requiring verbose glue code.


---

## Automatic Integration Matrix

| Source Module | Target Module | Automatic Integration Behavior |
|---|---|---|
| **Active Object** | **FSM** | Combines Table-Driven FSM state machine + `SYN_Mailbox` event queue + caller-owned `SYN_Task` runner into an event-driven actor. |
| **Button** | **FSM** | Uses `syn_fsm` state machine internally for debouncing, long-press hold transitions, and combo tracking. |
| **Watchdog** | **Error Log** | Task execution timeout and deadlock events are automatically recorded into `syn_errlog` when `.errlog` is configured. |
| **CLI** | **Scheduler** | Built-in CLI command `tasks` displays each task's name, priority, and state (`READY`, `SUSPENDED`, `DEAD`, `BLOCKED`) when `syn_cli_set_scheduler()` is invoked. |
| **CLI** | **Error Log** | Built-in CLI command `errors` dumps circular error log history over serial when `syn_cli_set_errlog()` is invoked. |
| **CLI** | **Version** | Built-in CLI command `version` automatically reports SyntropicOS kernel version, target architecture, and build timestamps. |
| **Param Store** | **CRC-16** | Uses `syn_crc16_ccitt()` for flash payload validation and corruption detection. |
| **Modbus** | **CRC-16** | Uses `syn_crc16_modbus()` for RTU frame validation. |

---

## 1. CLI + Scheduler & Error Log Integration

Attaching the scheduler and error logger to the Serial CLI exposes instant runtime diagnostics over UART without writing custom CLI commands:

```c
#include <syntropic/cli/syn_cli.h>
#include <syntropic/sched/syn_sched.h>

static SYN_CLI cli;
static SYN_Sched sched;

void app_setup_diagnostics(void) {
    // 1. Initialize CLI & Scheduler...
    
    // 2. Attach scheduler to CLI: automatically enables the 'tasks' command
    syn_cli_set_scheduler(&sched);
}
```

### CLI Terminal Output (`> tasks`)
```text
> tasks
Tasks:
  serial  prio=0  READY
  sensor  prio=1  BLOCKED
  blink  prio=2  READY
  logger  prio=3  SUSPENDED
```

---

## 2. Active Object Pattern (`syn_ao` + `syn_fsm`)

The **Active Object (AO)** integration combines three core primitives:
1. **FSM State Machine** (`syn_fsm`): Manages state transitions.
2. **Event Mailbox** (`SYN_Mailbox`): Buffers incoming `SYN_AO_Event` items (`mailbox_cap >= 2`; usable depth is `mailbox_cap - 1`).
3. **Caller-Owned Scheduler Task** (`SYN_Task`): Binds an entry in the application's `SYN_Task[]` array to the internal `syn_ao_pt_run` protothread at the specified `SYN_Prio`, parking in `SYN_TASK_BLOCKED` when empty and resuming automatically on `syn_ao_post()`.

### Active Object Flow

```mermaid
flowchart LR
    EventProducer["Async Interrupt / Driver"] -->|syn_ao_post| EventQueue["SYN_Mailbox"]
    EventQueue -->|Protothread Task Run| AOTask["syn_ao_pt_run (internal)"]
    AOTask -->|Dispatch Event| FSM["SYN_FSM State Machine"]
    FSM --> StateAction["Execute State Action Callback"]
```

### Usage Example

```c
#include <syntropic/sched/syn_ao.h>
#include <syntropic/sched/syn_sched.h>

enum { ST_IDLE = 0, ST_ACTIVE };
enum { EV_START = 1, EV_STOP };

static void on_start(void *ctx) {
    SYN_ActiveObject *ao = (SYN_ActiveObject *)ctx;
    (void)ao; // Access dispatched event payload via ao->last_event.data
}

static const SYN_FSM_Transition motor_transitions[] = {
    { ST_IDLE,   EV_START, ST_ACTIVE, NULL, on_start },
    { ST_ACTIVE, EV_STOP,  ST_IDLE,   NULL, NULL     },
    SYN_FSM_END
};

static SYN_Task tasks[2];
static SYN_Sched sched;
static SYN_ActiveObject motor_ao;
static SYN_AO_Event motor_mbox_buf[9]; // mailbox_cap = 9 (8 usable event slots)

void app_ao_setup(void) {
    syn_ao_init(&motor_ao, &tasks[0], "motor_ao", SYN_PRIO_HIGH,
                motor_transitions, ST_IDLE, motor_mbox_buf, 9);
    syn_task_create(&tasks[1], "housekeep", housekeep_task, SYN_PRIO_LOW, NULL);

    syn_sched_init(&sched, tasks, 2);
}

// ISR-safe and task-safe — queues event and wakes tasks[0] automatically:
void exti_start_irq(void) {
    syn_ao_post(&motor_ao, EV_START, NULL);
}
```
