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

### Writing Tasks: Rules and Gotchas

Protothreads use a `switch`/`__LINE__` continuation technique (sometimes
called Duff's device). This is invisible in normal use, but it creates two
constraints you must follow.

#### Rule 1: Local variables don't survive across yields or delays

When a task hits `PT_TASK_DELAY_MS()`, `PT_YIELD()`, or any `PT_WAIT_*`
macro, it **returns** from the function. When the scheduler calls it again,
execution resumes at the saved line — but local variables on the stack are
gone.

```c
// ❌ BAD: 'count' resets every time the task resumes
static SYN_PT_Status counter_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        int count = 0;                          // Allocated on the stack
        count++;
        PT_TASK_DELAY_MS(pt, task, 1000);       // Task returns here → stack is gone
        // 'count' is undefined after this point
    }
    PT_END(pt);
}
```

**Fix:** Use `static` locals, file-scope globals, or `task->user_data`:

```c
// ✅ Option A: static local
static SYN_PT_Status counter_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    static int count = 0;    // Lives in .bss, survives across yields
    for (;;) {
        count++;
        PT_TASK_DELAY_MS(pt, task, 1000);
    }
    PT_END(pt);
}

// ✅ Option B: user_data pointer (best for multiple task instances)
static SYN_PT_Status counter_task(SYN_PT *pt, SYN_Task *task)
{
    int *count = (int *)task->user_data;
    PT_BEGIN(pt);
    for (;;) {
        (*count)++;
        PT_TASK_DELAY_MS(pt, task, 1000);
    }
    PT_END(pt);
}
```

!!! warning "Local Variables & Resumption"
    Because protothreads are stackless and resume execution using a `switch` statement under the hood, **automatic local variables do not preserve their state across a yield**. 

    * **Across a Yield:** Any variable whose value must survive a yield (`PT_YIELD`, `PT_WAIT_UNTIL`, etc.) **must** be declared `static` or stored in a persistent structure (like `user_data`).
    * **Before `PT_BEGIN`:** Declaring a local variable before `PT_BEGIN` does **not** make it safe to use across a yield. Its declaration code will re-run on every invocation, resetting the variable to its initial value.
    * **Between Yields:** Local variables are only safe if their entire usage is self-contained **between** two consecutive yield points. To avoid compiler warnings about jumping over variable initializations, declare them inside a nested block `{ ... }` between the yields, or declare them before `PT_BEGIN` (if they do not need to persist across yields).

#### Rule 2: No `switch` statements inside a protothread body

Because `PT_BEGIN`/`PT_END` expand to a `switch`/`case` construct,
placing your own `switch` inside the protothread body produces nested
`switch` labels that confuse the compiler.

```c
// ❌ BAD: compiler error — nested switch conflicts with PT macros
static SYN_PT_Status my_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        switch (mode) {           // Conflicts with PT_BEGIN's switch
            case 0: /* ... */ break;
            case 1: /* ... */ break;
        }
        PT_TASK_DELAY_MS(pt, task, 100);
    }
    PT_END(pt);
}
```

**Fix:** Extract the `switch` into a helper function, or use `if`/`else if`:

```c
// ✅ Option A: helper function
static void handle_mode(int mode) {
    switch (mode) {
        case 0: /* ... */ break;
        case 1: /* ... */ break;
    }
}

static SYN_PT_Status my_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        handle_mode(mode);
        PT_TASK_DELAY_MS(pt, task, 100);
    }
    PT_END(pt);
}

// ✅ Option B: if/else if chain
static SYN_PT_Status my_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        if (mode == 0)      { /* ... */ }
        else if (mode == 1) { /* ... */ }
        PT_TASK_DELAY_MS(pt, task, 100);
    }
    PT_END(pt);
}
```

#### Summary

| Constraint | Workaround |
|---|---|
| Local variables lost across yield/delay | Use `static`, globals, or `task->user_data` |
| No `switch` inside task body | Extract to a helper function, or use `if`/`else if` |
| Task must be a single function | Use `PT_SPAWN()` for subtask decomposition |

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

The scheduler manages a caller-owned array of `SYN_Task` descriptors. Each call to `syn_sched_run()` selects and runs the **single highest-priority ready task** (0 = highest priority), with **round-robin** among equal-priority tasks.

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

## Tickless Idle

| Module | Header | Config |
|---|---|---|
| Tickless Idle | `sched/syn_sched.h` | `SYN_USE_TICKLESS` — Low-power sleep between task deadlines (requires: SCHED) |

Enabled via `SYN_USE_TICKLESS 1` in `syn_config.h`. Off by default — the scheduler is unchanged unless you opt in.

### How It Works

In a normal `syn_sched_run_forever()` loop, the CPU busy-loops when no tasks are ready. Tickless idle replaces that with low-power sleep:

```
┌────────────────────────────────────────────────────────┐
│                  syn_sched_run_tickless()               │
│                                                        │
│  ┌──► Run all ready tasks (syn_sched_run)              │
│  │                                                     │
│  │    Any tasks ready NOW?                             │
│  │    ├─ Yes → loop back, run them                     │
│  │    └─ No  → compute next wakeup deadline            │
│  │             │                                       │
│  │             ├─ Deadline exists → sleep until it      │
│  │             │   syn_port_sleep_until(wake_tick)      │
│  │             │                                       │
│  │             └─ No deadlines  → light sleep           │
│  │                 syn_sleep_enter(sleep)               │
│  │                                                     │
│  │    ◄── CPU wakes (timer alarm OR any interrupt) ──► │
│  └────────────────────────────────────────────────────  │
└────────────────────────────────────────────────────────┘
```

### Interrupts and Wakeup

The key insight: **`syn_port_sleep_until()` returns on *any* interrupt, not just the timer alarm.** This is how interrupts integrate naturally:

| Wakeup Source | What Happens |
|---|---|
| **Timer alarm fires** | CPU wakes at the scheduled tick. The scheduler runs delayed tasks that are now due. |
| **EXTI / GPIO interrupt** | CPU wakes early. The ISR runs (e.g., button press, sensor data-ready). If the ISR posts work via `syn_workqueue_post()` or sets an event via `syn_event_set()`, the scheduler picks it up on the next loop iteration. |
| **UART / SPI / DMA interrupt** | Same — ISR runs, fills a ring buffer or signals a semaphore. The scheduler runs the task that was waiting on that data. |
| **Any other IRQ** | CPU wakes, ISR runs, scheduler re-evaluates. If nothing is ready, it goes back to sleep. |

The scheduler **doesn't need to know about your interrupts.** It simply re-checks task readiness every time it wakes up. The idle loop is:

1. Run ready tasks
2. No tasks ready? → Sleep until the next deadline (or forever)
3. Wake up (timer *or* interrupt) → goto 1

### Wakelocks

If a peripheral needs the CPU to stay awake (e.g., mid-DMA transfer), use the sleep coordinator's wakelocks:

```c
syn_sleep_lock(&sleep);    // Prevent sleep
// ... do time-critical work ...
syn_sleep_unlock(&sleep);  // Allow sleep again
```

While any wakelock is held, `syn_sched_run_tickless()` busy-loops instead of sleeping — same as `syn_sched_run_forever()`.

### Example: Button + Periodic Task

```c
#define SYN_USE_TICKLESS 1

static SYN_Task tasks[2];
static SYN_Sched sched;
static SYN_Sleep sleep;

// Task A: blink LED every 1 second
static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task) {
    PT_BEGIN(pt);
    for (;;) {
        syn_gpio_toggle(LED_PIN);
        PT_TASK_DELAY_MS(pt, task, 1000);
    }
    PT_END(pt);
}

// Task B: respond to button press (EXTI wakes CPU)
static SYN_PT_Status button_task(SYN_PT *pt, SYN_Task *task) {
    PT_BEGIN(pt);
    for (;;) {
        PT_WAIT_UNTIL(pt, button_pressed);
        handle_button();
        button_pressed = false;
    }
    PT_END(pt);
}

int main(void) {
    syn_task_create(&tasks[0], "blink",  blink_task,  1, NULL);
    syn_task_create(&tasks[1], "button", button_task, 0, NULL);
    syn_sched_init(&sched, tasks, 2);
    syn_sleep_init(&sleep);

    // CPU sleeps between 1-second blinks.
    // Button EXTI wakes it early when pressed.
    syn_sched_run_tickless(&sched, &sleep);
}
```

Between blinks, the CPU enters low-power mode for ~1 second. If a button EXTI fires at 500 ms, the CPU wakes immediately, the ISR sets `button_pressed`, and the scheduler runs `button_task`. Then it goes back to sleep for the remaining ~500 ms until the next blink.

### Port Requirement

Implement `syn_port_sleep_until(uint32_t wake_tick_ms)` in your platform port. This function must:

1. Program a hardware wake timer (RTC alarm, LPTIM compare, etc.) for `wake_tick_ms`
2. Enter a low-power mode (e.g., `__WFI()` on Cortex-M)
3. Return when the alarm fires **or** any interrupt wakes the CPU

The default weak stub falls back to `syn_port_sleep(SYN_SLEEP_LIGHT)` (no timer programming — just WFI).


