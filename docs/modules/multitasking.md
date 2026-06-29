# Cooperative Multitasking

SyntropicOS provides a cooperative multitasking kernel built on protothreads — stackless coroutines that cost **2 bytes of RAM per thread**.

## Protothreads

| Module | Header | Config |
|---|---|---|
| Protothreads | `pt/syn_pt.h` | `SYN_USE_PT` |
| Semaphores | `pt/syn_pt_sem.h` | `SYN_USE_PT` |

Protothreads are stackless cooperative coroutines implemented via the Duff's device trick (`switch`/`__LINE__` continuation). Each `SYN_PT` struct is a single `uint16_t`.

**Core macros:**

| Macro | Description |
|---|---|
| `PT_BEGIN(pt)` | Open a protothread body (must be first) |
| `PT_END(pt)` | Close and return `PT_EXITED` |
| `PT_WAIT_UNTIL(pt, cond)` | Block until condition is true |
| `PT_WAIT_WHILE(pt, cond)` | Block while condition is true |
| `PT_YIELD(pt)` | Yield control unconditionally |
| `PT_EXIT(pt)` | Terminate immediately |
| `PT_RESTART(pt)` | Reset and restart from the top |
| `PT_SPAWN(pt, child, func)` | Run a child protothread and block until it exits |
| `PT_DELAY_MS(pt, target, ms)` | Non-blocking delay (needs a `uint32_t*` for deadline storage) |
| `PT_TASK_DELAY_MS(pt, task, ms)` | Convenience form using `task->delay_until` |

!!! warning "Protothread constraints"
    - You **cannot** use a bare `switch` statement inside a protothread body (use `if`/`else if` chains).
    - Local variables are **not preserved** across yield/wait points. Use `static` variables or `task->user_data`.
    - A protothread must reside entirely within a single function.

**Return values** (`SYN_PT_Status`):

- `PT_WAITING` — blocked, condition not met
- `PT_YIELDED` — voluntarily yielded
- `PT_EXITED` — ran to `PT_END`
- `PT_ENDED` — explicitly ended via `PT_EXIT`

## Scheduler

| Module | Header | Config |
|---|---|---|
| Task | `sched/syn_task.h` | `SYN_USE_SCHED` |
| Scheduler | `sched/syn_sched.h` | `SYN_USE_SCHED` |

The scheduler manages a caller-owned array of `SYN_Task` descriptors. On each tick it runs every ready task in **priority order** (0 = highest), with **round-robin** among equal-priority tasks.

```c
static SYN_Task tasks[3];
static SYN_Sched sched;

syn_task_create(&tasks[0], "blink",   blink_fn,   1, NULL);
syn_task_create(&tasks[1], "serial",  serial_fn,  0, NULL);
syn_task_create(&tasks[2], "monitor", monitor_fn, 2, NULL);

syn_sched_init(&sched, tasks, 3);
syn_sched_run_forever(&sched);  // never returns
```

**Task control:**

- `syn_task_suspend(task)` — skip task until resumed
- `syn_task_resume(task)` — make task eligible again
- `syn_task_restart(task)` — reset protothread and set to READY
- `syn_task_is_alive(task)` — check if task is not DEAD
- `syn_sched_alive_count(sched)` — count living tasks

## Timers & Scheduling Extensions

| Module | Header | Config | Description |
|---|---|---|---|
| Timers | `sched/syn_timer.h` | `SYN_USE_TIMER` | Software timers — one-shot and periodic callbacks driven by `syn_port_get_tick_ms()` |
| Events | `util/syn_event.h` | `SYN_USE_EVENT` | 32-bit event flag groups for inter-task signaling |
| Watchdog | `sched/syn_watchdog.h` | `SYN_USE_WATCHDOG` | Per-task software check-in deadlines. Timeout events auto-record to `syn_errlog` if configured. |
| Sequencer | `sched/syn_sequencer.h` | `SYN_USE_SEQUENCER` | Step-based async sequence runner for timed action chains |
| Work Queue | `sched/syn_workqueue.h` | `SYN_USE_WORKQUEUE` | Deferred work queue — safely post work from ISR to main thread |
| Mailbox | `sched/syn_mailbox.h` | Always available | Typed single-producer/single-consumer (SPSC) message queue (header-only) |
| Active Object | `sched/syn_ao.h` | `SYN_USE_AO` | Active Object execution context — combines FSM + event queue + scheduler task |
