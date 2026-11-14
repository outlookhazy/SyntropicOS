# Cooperative Multitasking

SyntropicOS provides a cooperative multitasking kernel built on protothreads — stackless coroutines where standalone `SYN_PT` continuation state costs **2 bytes of RAM** (`uint16_t lc`), and full scheduled `SYN_Task` descriptors with priority, delays, mutex tracking, and event-blocking cost **44 bytes per task descriptor on 32-bit targets** (80 bytes on 64-bit LP64 targets, with zero per-task stack allocation).

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
| `PT_END(pt)` | Close body and return `PT_ENDED` (normal completion) |
| `PT_WAIT_UNTIL(pt, cond)` | Yield `PT_IDLE` until condition is true |
| `PT_WAIT_WHILE(pt, cond)` | Yield `PT_IDLE` while condition is true |
| `PT_PASS(pt)` | Yield `PT_IDLE` — no immediate work; let other ready tasks run this round |
| `PT_CONTINUE(pt)` | Yield `PT_MORE` — completed a work chunk and more work is waiting |
| `PT_CONTINUE_IF_NEEDED(pt, task)` | Yield `PT_MORE` only if `syn_task_should_share(task)` is true (`sched/syn_sched.h`) |
| `PT_EXIT(pt)` | Abort immediately and return `PT_EXITED` |
| `PT_RESTART(pt)` | Reset continuation and return `PT_IDLE` |
| `PT_SPAWN(pt, child, func)` | Run child protothread until exit, propagating child `PT_IDLE` / `PT_MORE` verbatim |
| `PT_DELAY_MS(pt, target, ms)` | Non-blocking delay (needs a `uint32_t*` for deadline storage) |
| `PT_TASK_DELAY_MS(pt, task, ms)` | Convenience form using `task->delay_until` |
| `PT_BLOCK_CONDITION(pt, task, cond)` | Park task in `SYN_TASK_BLOCKED` without polling while `cond` is false; requires explicit `syn_task_resume(task)` from an ISR/driver/producer to re-evaluate `cond` |
| `PT_BLOCK_EVENT(pt, task, grp, mask)` | Block task execution (`SYN_TASK_BLOCKED`) until event bit fires |
| `PT_BLOCK_EVENT_WITH_TIMEOUT(pt, task, grp, mask, ms, out)` | Block task execution (`SYN_TASK_BLOCKED`) until event bit fires OR timeout elapses |

### Macro Expansion Under the Hood (Duff's Device)

Protothreads save execution position using a 2-byte line continuation variable (`uint16_t lc`) and C `switch`/`case` jump tables:

=== "High-Level C Code"
    ```c
    SYN_PT_Status blink_task(SYN_PT *pt)
    {
        PT_BEGIN(pt);
        while (1) {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            PT_WAIT_UNTIL(pt, (syn_port_get_tick_ms() - last_tick) >= 500);
            last_tick = syn_port_get_tick_ms();
        }
        PT_END(pt);
    }
    ```

=== "Preprocessor Macro Expansion"
    ```c
    SYN_PT_Status blink_task(SYN_PT *pt)
    {
        char _pt_yield_flag = 1;
        switch (pt->lc) {
            case 0:
                while (1) {
                    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

                    /* PT_WAIT_UNTIL expansion: saves __LINE__ (10) into pt->lc before returning.
                     * On next function invocation, switch(pt->lc) jumps directly to 'case 10:' 
                     * inside the while loop, bypassing function re-entry and resuming execution! */
                    pt->lc = 10; case 10:
                    if (!((syn_port_get_tick_ms() - last_tick) >= 500)) {
                        return PT_IDLE;
                    }

                    last_tick = syn_port_get_tick_ms();
                }
        }
        _pt_yield_flag = 0;
        pt->lc = 0;
        return PT_ENDED;
    }
    ```

### Writing Tasks: Rules and Gotchas

Protothreads use a `switch`/`__LINE__` continuation technique (sometimes
called Duff's device). This is invisible in normal use, but it creates two
constraints you must follow.

#### Rule 1: Local variables don't survive across yields or delays

When a task hits `PT_TASK_DELAY_MS()`, `PT_PASS()`, `PT_CONTINUE()`, or any `PT_WAIT_*`
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

    * **Across a Yield:** Any variable whose value must survive a yield (`PT_PASS`, `PT_CONTINUE`, `PT_WAIT_UNTIL`, etc.) **must** be declared `static` or stored in a persistent structure (like `user_data`).
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

- `PT_IDLE` (`0`) — nothing to do right now (`PT_PASS`, false `PT_WAIT_UNTIL`); scheduler marks the task `PASSED` for the current round so other ready tasks run first
- `PT_MORE` (`1`) — completed a chunk of work and more work is waiting (`PT_CONTINUE`); task stays in the scheduler's `Work` tier
- `PT_EXITED` (`2`) — aborted early via `PT_EXIT`
- `PT_ENDED` (`3`) — ran to `PT_END` (normal completion)

## Scheduler

| Module | Header | Config |
|---|---|---|
| Task | `sched/syn_task.h` | `SYN_USE_SCHED` |
| Scheduler | `sched/syn_sched.h` | `SYN_USE_SCHED` |
| Mutex | `sched/syn_mutex.h` | `SYN_USE_MUTEX` |

The scheduler manages a caller-owned array of `SYN_Task` descriptors. Each call to `syn_sched_run()` dispatches **one** eligible task using ranked selection with per-priority starvation bounds and round-robin rotation within each priority level.

```c
static SYN_Task tasks[3];
static SYN_Sched sched;

syn_task_create(&tasks[0], "blink",   blink_fn,   SYN_PRIO_HIGH,     NULL);
syn_task_create(&tasks[1], "serial",  serial_fn,  SYN_PRIO_CRITICAL, NULL);
syn_task_create(&tasks[2], "monitor", monitor_fn, SYN_PRIO_NORMAL,   NULL);

syn_sched_init(&sched, tasks, 3);
syn_sched_run_forever(&sched);  // never returns
```

### Priorities & Starvation Windows

SyntropicOS defines four standard priority levels (`SYN_Prio` in `syn_task.h`, `0` = highest):

| Priority | Value | Default Max Wait (`max_wait_ms`) | Config Macro | Typical Workload |
|---|---|---|---|---|
| `SYN_PRIO_CRITICAL` | `0` | `1 ms` | `SYN_SCHED_MAX_WAIT_CRITICAL_MS` | Control loops, safety interlocks, low-latency bus polling |
| `SYN_PRIO_HIGH` | `1` | `5 ms` | `SYN_SCHED_MAX_WAIT_HIGH_MS` | Protocol I/O, serial CLI, active frame/event handlers |
| `SYN_PRIO_NORMAL` | `2` | `15 ms` | `SYN_SCHED_MAX_WAIT_NORMAL_MS` | Application state machines, UI refresh, periodic sampling |
| `SYN_PRIO_LOW` | `3` | `50 ms` | `SYN_SCHED_MAX_WAIT_LOW_MS` | Background batch compute (NN inference, flash GC, logging) |

```c
// Define SYN_USE_* and struct-sizing macros in syn_config.h (or project-wide -D flags)
// so syn_sched.c and application translation units compile with identical layouts:
#define SYN_SCHED_PRIO_LEVELS 4  // default

// Or configure per-instance at runtime (values < 1 clamp to 1 ms):
syn_sched_set_max_wait(&sched, SYN_PRIO_LOW, 25);
```

#### Cooperative Chunk Sizing Rule

Because protothreads are non-preemptive, the scheduler only regains control at yield points. Keep every uninterrupted execution chunk across **all** tasks (including `SYN_PRIO_LOW`) shorter than the tightest active `max_wait_ms` (`1 ms` default when `SYN_PRIO_CRITICAL` is used). Split multi-millisecond loops across `PT_CONTINUE(pt)` or `PT_CONTINUE_IF_NEEDED(pt, task)`.

Attach a `SYN_TaskProfileManager` (`syntropic/debug/syn_task_profile.h`) via `syn_sched_set_profiler(&sched, &prof)` during development, HIL, or QEMU CI runs and call `syn_task_profile_check_budgets(&prof, &sched, &violations, &worst)` to verify that no scheduled task's `max_yield_us` exceeds its participating peers' `max_wait_ms` budget.

### Ranked Dispatch & Yield Intent (`PT_PASS` vs `PT_CONTINUE`)

On each `syn_sched_run()` call, eligible (ready, non-delayed, non-blocked) tasks are grouped into ranks. The highest rank with any member wins, and **priority followed by round-robin** determines the winner inside every rank:

| Rank | Membership | Order Within Rank |
|---|---|---|
| **1. Overdue** | `(int32_t)(now - due_ms) >= 0` | Priority, then round-robin |
| **2. Work** | `MORE` set (`PT_CONTINUE` or woke from sleep/block) | Priority, then round-robin |
| **3. Poll** | Not `MORE` and not `PASSED` | Priority, then round-robin |
| **Round Reset** | Only `PASSED` tasks remain | Clear all `PASSED` flags and select by priority, then round-robin |

After each dispatch, `due_ms = now + max_wait_ms[prio]` and the priority level's round-robin cursor advances.

!!! important "Starvation Bound Guarantee"
    Every eligible task runs within `max_wait_ms[prio]` **plus** the execution time of the cooperative chunk running at that moment **plus** any higher-priority overdue tasks ahead of it. This is a soft cooperative bound; hard real-time latency belongs in an ISR. If higher priorities consume more than 100% of CPU (`sum(run_time / max_wait) > 1`), lower priorities shed first by design.

#### Choosing the Right Yield Macro

| Macro | Status Returned | Scheduler Effect | When to Use |
|---|---|---|---|
| `PT_PASS(pt)` | `PT_IDLE` | Marks task `PASSED` for the current round | Polling loops that checked for work and found none, or finished draining |
| `PT_CONTINUE(pt)` | `PT_MORE` | Keeps task in `Work` rank | Chunked processing (e.g. NN layer, HTTP stream, motion step) with remaining backlog |
| `PT_CONTINUE_IF_NEEDED(pt, task)` | `PT_MORE` (conditional) | Yields only if `syn_task_should_share(task)` is true | Drain loops that can process multiple items back-to-back until an equal/higher-priority `MORE` peer is waiting or any peer reaches its `due_ms` |
| `PT_WAIT_UNTIL(pt, cond)` | `PT_IDLE` (if false) | Marks task `PASSED` for the current round | Polled condition waiting (keeps task `READY`; use `PT_BLOCK_*` for tickless sleep) |
| `PT_BLOCK_EVENT(pt, ...)` | `PT_IDLE` + `BLOCKED` | Skipped completely until event flag fires | Event-driven tasks and tickless low-power designs |

#### Four Standard Task Archetypes

```c
// 1. Periodic task: sleeps until deadline (tickless-compatible, wakes in Work rank)
static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_gpio_toggle(LED_PIN);
        PT_TASK_DELAY_MS(pt, task, 500);
    }
    PT_END(pt);
}

// 2. Non-blocking poller: PT_CONTINUE while active, PT_PASS when idle
static SYN_PT_Status cli_poll_task(SYN_PT *pt, SYN_Task *task)
{
    uint8_t byte;
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        if (uart_try_read_byte(&byte)) {
            syn_cli_process_char(&cli, (char)byte);
            PT_CONTINUE(pt); // More RX bytes likely queued — stay in Work rank
        } else {
            PT_PASS(pt);     // Nothing waiting — pass remainder of round
        }
    }
    PT_END(pt);
}

// 3. Chunked compute worker: bounded latency per step with PT_CONTINUE
static SYN_PT_Status nn_worker_task(SYN_PT *pt, SYN_Task *task)
{
    static size_t layer;
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        PT_WAIT_UNTIL(pt, inference_requested);
        for (layer = 0; layer < NUM_LAYERS; layer++) {
            run_nn_layer(layer); // Keep each chunk < 1 ms
            PT_CONTINUE(pt);     // More layers remain — share CPU, stay in Work rank
        }
        inference_requested = false;
        PT_PASS(pt);
    }
    PT_END(pt);
}

// 4. Adaptive burst drain: zero yield overhead until a peer actually needs CPU
static SYN_PT_Status rx_drain_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        while (rx_ring_available() > 0) {
            process_one_frame();
            PT_CONTINUE_IF_NEEDED(pt, task); // Yields only on peer contention/due_ms
        }
        PT_PASS(pt); // Queue empty — let lower priorities run this round
    }
    PT_END(pt);
}
```

!!! warning "Anti-Pattern: Unconditional `PT_CONTINUE` in an Idle Poll Loop"
    Never end an unconditional poll loop with `PT_CONTINUE(pt)` when no work was performed:
    ```c
    // ❌ BAD: stays in Work rank forever even when sensor has no data
    for (;;) {
        if (sensor_has_sample()) { read_sample(); }
        PT_CONTINUE(pt); // Forces lower-priority tasks onto their max_wait_ms timer!
    }

    // ✅ GOOD: only report PT_MORE when work remains; otherwise PT_PASS
    for (;;) {
        if (sensor_has_sample()) {
            read_sample();
            PT_CONTINUE(pt);
        } else {
            PT_PASS(pt);
        }
    }
    ```
    Returning `PT_MORE` when idle defeats `Poll` rounds (`Rank 3`) and prevents lower-priority tasks from running until their `due_ms` starvation deadline expires.


### True Task Blocking Primitives (`SYN_TASK_BLOCKED`)

Standard protothread `PT_WAIT_UNTIL` and `PT_WAIT_EVENT` use cooperative polling — the task stays `SYN_TASK_READY`, executes once per polling round, checks its condition, and returns `PT_IDLE`. While functional, this wastes CPU cycles and prevents tickless sleep hardware low-power modes.

SyntropicOS provides **First-Class Native Task Blocking**. When a task blocks via `PT_BLOCK_EVENT` or `PT_BLOCK_CONDITION` (and its subsystem wrappers), its state transitions to `SYN_TASK_BLOCKED` and the scheduler **skips the task completely** on subsequent dispatches (zero CPU polling overhead, tickless-sleep compatible):

```c
#define EVT_DATA_READY  SYN_BIT(0)

static SYN_EventGroup uart_events;

// ISR:
void UART_IRQHandler(void) {
    syn_event_set(&uart_events, EVT_DATA_READY);  // ISR-safe; scheduler auto-wakes blocked waiter
}

// Task:
static SYN_PT_Status uart_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        PT_BLOCK_EVENT(pt, task, &uart_events, EVT_DATA_READY);
        // Wakes here when EVT_DATA_READY is set (flag auto-cleared)
        process_uart_data();
    }
    PT_END(pt);
}
```

#### Native Task Blocking Matrix Across OS Services

Native blocking primitives fall into two distinct wake contracts:

**1. Auto-Waking Native Primitives (no manual `syn_task_resume(task)` required)**

| Subsystem / Layer | Polled Primitive | Native Task-Blocking Primitive | Automatic Wake Mechanism |
|---|---|---|---|
| **Event Flags** | `PT_WAIT_EVENT(pt, grp, mask)` | `PT_BLOCK_EVENT(pt, task, grp, mask)` / `PT_BLOCK_EVENT_WITH_TIMEOUT(pt, task, grp, mask, ms, out)` | Woken automatically when `syn_event_flags_set()` / `syn_event_set()` sets a matching bit (or `ms` timeout expires) |
| **Active Objects** | — | `SYN_ActiveObject` (`syn_ao_init`, internal `syn_ao_pt_run`) | Woken automatically by `syn_ao_post(ao, sig, data)` |
| **TCP / UDP Sockets** | — | `PT_TCP_BLOCK_READ(...)` / `PT_UDP_BLOCK_READ(...)` | Woken automatically by `syn_tcp_process_packet()`, `syn_udp_process_packet()`, or `syn_udp_unbind()` |

**2. Zero-RAM `PT_BLOCK_CONDITION` Convenience Wrappers (producer / ISR MUST call `syn_task_resume(task)`)**

!!! warning "`PT_BLOCK_CONDITION` Does Not Poll"
    `PT_BLOCK_CONDITION(pt, task, cond)` and the zero-RAM convenience wrappers below do **not** store a waiter pointer inside the target struct and are **not** polled by the scheduler. After updating the underlying state, the producer, driver, or ISR **must** call `syn_task_resume(task)` so the scheduler re-evaluates `cond`; omitting `syn_task_resume(task)` leaves the task parked in `SYN_TASK_BLOCKED` indefinitely.

| Subsystem / Layer | Polled Primitive | Native Task-Blocking Wrapper | Required Producer / ISR Wake Sequence |
|---|---|---|---|
| **Core Expression** | `PT_WAIT_UNTIL(pt, cond)` | `PT_BLOCK_CONDITION(pt, task, cond)` | Update state satisfying `cond` + `syn_task_resume(task)` |
| **Semaphores** | `PT_SEM_WAIT(pt, sem)` | `PT_SEM_BLOCK(pt, task, sem)` | `PT_SEM_SIGNAL(sem)` + `syn_task_resume(task)` |
| **Stream I/O** | `PT_STREAM_WAIT(pt, stream)` | `PT_BLOCK_STREAM(pt, task, stream)` | `syn_stream_put()` / `syn_stream_write()` + `syn_task_resume(task)` |
| **SPSC Queue** | `PT_QUEUE_WAIT_POP(pt, q)` | `PT_BLOCK_QUEUE_POP(pt, task, q)` | `syn_spsc_queue_push()` + `syn_task_resume(task)` |
| **DHCP Engine** | `PT_DHCP_WAIT_BOUND(pt, dhcp)` | `PT_DHCP_BLOCK_BOUND(pt, task, dhcp)` | `syn_dhcp_process_packet()` state update + `syn_task_resume(task)` |
| **Network Config** | `PT_NETCFG_WAIT_BOUND(pt, netcfg)` | `PT_NETCFG_BLOCK_BOUND(pt, task, netcfg)` / `PT_NETCFG_BLOCK_LINK_UP(...)` | `syn_netcfg_*()` state update + `syn_task_resume(task)` |
| **AutoIP Service** | `PT_AUTOIP_WAIT_BOUND(pt, autoip)` | `PT_AUTOIP_BLOCK_BOUND(pt, task, autoip)` | `syn_autoip_process_arp()` state update + `syn_task_resume(task)` |
| **Button Input** | `PT_WAIT_BUTTON_PRESS(pt, btn)` / `RELEASE` | `PT_BUTTON_BLOCK_PRESS(pt, task, btn)` / `PT_BUTTON_BLOCK_RELEASE(pt, task, btn)` | `syn_button_update()` state update + `syn_task_resume(task)` |

- **Ethernet RX (`PT_ETH_WAIT_FRAME`)**: `PT_ETH_WAIT_FRAME(pt, rx_buf, max_len, rx_len)` is a polled primitive (`PT_WAIT_UNTIL` over `syn_port_eth_rx()`). For native task blocking on Ethernet RX, signal an event flag (`PT_BLOCK_EVENT`) or pair `PT_BLOCK_CONDITION` with `syn_task_resume(task)` in the MAC/PHY RX ISR.
- **Mutex Contention (`PT_MUTEX_LOCK`)**: `PT_MUTEX_LOCK(pt, task, mutex)` uses polled `PT_WAIT_UNTIL` with priority inheritance and promotes the contended waiter into `Work` rank on `syn_mutex_unlock()` (keeps contended tasks `SYN_TASK_READY` and prevents tickless sleep while contended).

| Approach | Scheduling cost | Tickless-safe | Use case |
|---|---|---|---|
| `PT_WAIT_*` (Polled) | Polled every pass | No — prevents sleep | Simple standalone / legacy coroutines |
| `PT_BLOCK_*` (Native) | Skipped entirely while blocked | Yes | Low-power event-driven services |

### Yield-Safe Priority Boosting (`syn_task_boost_priority`)

Tasks can temporarily elevate their execution priority during critical hardware transactions (e.g. RS-485 frame bursts, SPI flash writes) without risk of stack variable corruption across yields:

```c
static SYN_PT_Status sensor_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        PT_TASK_DELAY_MS(pt, task, 1000);

        // Elevate task to SYN_PRIO_CRITICAL (highest)
        syn_task_boost_priority(task, SYN_PRIO_CRITICAL);

        send_sensor_request();
        PT_WAIT_UNTIL(pt, sensor_data_ready()); // Yield safe across ticks!
        read_sensor_response();

        // Restore back to configured base priority
        syn_task_restore_priority(task);
    }
    PT_END(pt);
}
```

- **`syn_task_boost_priority(task, temp_prio)`**: Monotonic upward priority boost — elevates `task->priority` to `temp_prio` only when `temp_prio < task->priority` (numerically lower `SYN_Prio` is higher priority; never demotes an already higher manual boost or inherited mutex priority), marks `MORE`, and tightens `due_ms` to the boosted priority's wait window.
- **`syn_task_restore_priority(task)`**: Restores `task->priority` to `task->base_priority`, or retains the highest contended waiter priority across any held `SYN_Mutex` locks while priority inheritance is active.
- **`syn_task_set_base_priority(task, new_prio)`**: Updates `task->base_priority` permanently and recomputes `task->priority` while preserving any higher active `SYN_Mutex` priority inheritance.

### Priority-Inheriting Mutex (`sched/syn_mutex.h`)

When tasks at different priorities share a peripheral or multi-step transaction (e.g. shared SPI bus or I2C EEPROM), `SYN_Mutex` provides recursive cooperative mutual exclusion with automatic priority inheritance:

```c
static SYN_Mutex spi_bus_mtx;

// Initialize once during setup:
syn_mutex_init(&spi_bus_mtx);

// Inside any task (e.g. SYN_PRIO_LOW flash logger or SYN_PRIO_CRITICAL sensor):
static SYN_PT_Status flash_log_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        PT_WAIT_UNTIL(pt, log_page_full());

        PT_MUTEX_LOCK(pt, task, &spi_bus_mtx);
        spi_select_flash();
        PT_SPAWN(pt, &flash_pt, flash_write_page_pt(&flash_pt));
        spi_deselect_flash();
        PT_MUTEX_UNLOCK(&spi_bus_mtx, task);

        PT_PASS(pt);
    }
    PT_END(pt);
}
```

- **Priority Inheritance**: If a higher-priority task calls `PT_MUTEX_LOCK` while a lower-priority task holds `spi_bus_mtx`, the holder's priority is immediately boosted to the waiter's priority and promoted into `Work` rank (`MORE` set) until `PT_MUTEX_UNLOCK` releases the outer lock, preventing medium-priority tasks from preempting the holder.
- **Recursive Locking**: The owning task may re-acquire the same `SYN_Mutex` up to `UINT16_MAX` times; ownership releases when `lock_count` returns to `0`.
- **Non-Blocking Try-Lock**: `PT_MUTEX_TRYLOCK(&spi_bus_mtx, task)` attempts immediate acquisition and returns `true` on success or `false` if held by another task.
- **Single-Scheduler (AMP) Scope**: All scheduled tasks acquiring or waiting on a `SYN_Mutex` must belong to the same `SYN_Sched` instance (`SYN_INVALID_PARAM` on cross-scheduler contention). For cross-core synchronization in AMP builds, use `SYN_SPINLOCK_GUARD()` or SPSC `syn_mailbox`.

### Task Delay Semantics: "At Least" Minimum Wait Guarantees

In a cooperative operating system like SyntropicOS, all delay macros (`PT_TASK_DELAY_MS`, `PT_DELAY_MS`, `PT_DELAY_US`) guarantee a **minimum wait duration** ("at least N ms / N us").

!!! important "Non-Preemptive Delay Guarantee"
    Delays do **not** guarantee an exact microsecond or millisecond wakeup time. Because execution is strictly cooperative, a task whose delay has expired will resume on the **next scheduler pass after the currently executing task yields**. Actual resumption latency equals `deadline + execution_time_of_running_tasks`.

#### Millisecond vs Microsecond Delays

1. **Millisecond Delays (`PT_TASK_DELAY_MS`)**:
   - Managed directly by the scheduler top-level loop (`task->delay_until` in ms).
   - Maximum single delay capacity: **24.8 days** ($2,147,483,647$ ms).
   - Ideal for periodic tasks, heartbeat timers, and protocol timeouts.

2. **Microsecond Delays (`PT_DELAY_US`)**:
   - Uses a dedicated, user-supplied persistent target variable (`static uint32_t us_target;`).
   - Does **not** modify `task->delay_until`, preventing top-level scheduler deadline conflicts.
   - Evaluated inside the protothread via `PT_WAIT_UNTIL` against `syn_port_get_tick_us()`. Returns `PT_IDLE` while the deadline is pending, allowing other ready tasks to run during the round.

```c
static SYN_PT_Status pulse_task(SYN_PT *pt, SYN_Task *task)
{
    static uint32_t us_target; // Persistent target for microsecond delay

    PT_BEGIN(pt);
    for (;;) {
        syn_gpio_write(LED_PIN, SYN_GPIO_HIGH);

        // 1. Microsecond delay: minimum 50 µs wait (uses us_target, task->delay_until stays 0)
        PT_DELAY_US(pt, &us_target, 50);

        syn_gpio_write(LED_PIN, SYN_GPIO_LOW);

        // 2. Millisecond delay: minimum 100 ms wait (managed by scheduler in ms)
        PT_TASK_DELAY_MS(pt, task, 100);
    }
    PT_END(pt);
}
```


**Task states:**


| State | Value | Description |
|---|---|---|
| `SYN_TASK_READY` | 0 | Eligible to run when not delayed |
| `SYN_TASK_SUSPENDED` | 1 | Skipped until resumed |
| `SYN_TASK_DEAD` | 2 | Exited, will not run again |
| `SYN_TASK_BLOCKED` | 3 | Blocked on event/condition — skipped until woken |

**Task & scheduler control:**

- `syn_sched_set_max_wait(sched, prio, ms)` — configure per-priority starvation bound (clamped to `>= 1` ms)
- `syn_task_should_share(task)` — true during `task`'s dispatch if an equal/higher-priority `MORE` peer is waiting or a peer's `due_ms` has arrived
- `syn_task_suspend(task)` — skip task until resumed
- `syn_task_resume(task)` — make task eligible again
- `syn_task_restart(task)` — reset protothread and set to READY
- `syn_task_is_alive(task)` — check if task is not DEAD
- `syn_sched_alive_count(sched)` — count living tasks

### ISR-Safety Reference

| Execution Context | Functions / Macros |
|---|---|
| **ISR-safe** | `syn_task_resume()`, `syn_event_flags_set()` (`syn_event_set()`), `syn_event_flags_clear()` (`syn_event_clear()`), `syn_event_flags_get()` (`syn_event_get()`), `syn_ao_post()`, `syn_workqueue_post()`, `syn_spsc_queue_push()` (single-ISR producer), `syn_sleep_lock()`, `syn_sleep_unlock()` |
| **Task / main-loop context only** | `syn_sched_init()`, `syn_sched_run*()`, `syn_sched_set_max_wait()`, `syn_task_create()`, `syn_task_suspend()`, `syn_task_restart()`, `syn_task_boost_priority()`, `syn_task_restore_priority()`, `syn_task_set_base_priority()`, `syn_mutex_try_lock()`, `syn_mutex_unlock()` |

## Timers & Scheduling Extensions

| Module | Header | Config | Description |
|---|---|---|---|
| Timers | `sched/syn_timer.h` | `SYN_USE_TIMER` | Software timers — one-shot and periodic callbacks driven by `syn_port_get_tick_ms()` |
| Timer Wheel | `sched/syn_timer_wheel.h` | Always available | O(1) 64-bucket hashed timing wheel (`syn_timer_wheel_add`, `syn_timer_wheel_cancel`, `syn_timer_wheel_step`) for high-count one-shot timeouts |
| Events | `sched/syn_event_flags.h` / `util/syn_event.h` | `SYN_USE_EVENT` | 32-bit atomic event flag groups (`SYN_EventFlags` / `SYN_EventGroup`) for ISR & inter-task signaling |
| Watchdog | `sched/syn_watchdog.h` | `SYN_USE_WATCHDOG` | Per-task software check-in deadlines. Timeout events auto-record to `syn_errlog` if configured. |
| Sequencer | `sched/syn_sequencer.h` | `SYN_USE_SEQUENCER` | Step-based async sequence runner for timed action chains |
| Work Queue | `sched/syn_workqueue.h` | `SYN_USE_WORKQUEUE` | Deferred work queue — safely post work from ISR to main thread |
| Mailbox | `sched/syn_mailbox.h` | Always available | Typed single-producer/single-consumer (SPSC) message queue (header-only) |
| Active Object | `sched/syn_ao.h` | `SYN_USE_AO` | Active Object execution context — combines FSM + `SYN_Mailbox` + caller-owned scheduler task |

## Tickless Idle

| Module | Header | Config |
|---|---|---|
| Tickless Idle | `sched/syn_sched.h` | `SYN_USE_TICKLESS` — Low-power sleep between task deadlines (requires: SCHED) |

Enabled via `SYN_USE_TICKLESS 1` in `syn_config.h`. Off by default — the scheduler is unchanged unless you opt in.

### How It Works

In a normal `syn_sched_run_forever()` loop, the CPU busy-loops when no tasks are ready. Tickless idle replaces that with low-power sleep (`syn_sched_run_tickless()` for task deadlines, or `syn_sched_run_tickless_ex()` when `SYN_Timer` deadlines must also wake the CPU):

```
┌────────────────────────────────────────────────────────┐
│                syn_sched_run_tickless()                │
│                                                        │
│  +---> Run all ready tasks (syn_sched_run)             │
│  |                                                     │
│  |    Any tasks ready NOW?                             │
│  |    |- Yes -> loop back, run them                    │
│  |    \- No  -> compute next wakeup deadline           │
│  |             |                                       │
│  |             |- Deadline exists -> sleep until it    │
│  |             |   syn_port_sleep_until(wake_tick)     │
│  |             |                                       │
│  |             \- No deadlines  -> light sleep         │
│  |                 syn_sleep_enter(sleep)              │
│  |                                                     │
│  |    <--- CPU wakes (timer alarm OR interrupt) --->   │
│  +───────────────────────────────────────────────────  │
└────────────────────────────────────────────────────────┘
```

### Interrupts and Wakeup

The key insight: **`syn_port_sleep_until()` returns on *any* interrupt, not just the timer alarm.** This is how interrupts integrate naturally:

| Wakeup Source | What Happens |
|---|---|
| **Timer alarm fires** | CPU wakes at the scheduled tick. The scheduler runs delayed tasks that are now due. |
| **EXTI / GPIO interrupt** | CPU wakes early. The ISR runs (e.g., button press, sensor data-ready). If the ISR posts work via `syn_workqueue_post()` or sets an event via `syn_event_set()`, the scheduler picks it up on the next loop iteration. |
| **UART / SPI / DMA interrupt** | Same — ISR runs, fills a ring buffer/stream or signals a semaphore, and calls `syn_task_resume(task)` (or `syn_event_set()`). The scheduler runs the woken task. |
| **Any other IRQ** | CPU wakes, ISR runs, scheduler re-evaluates. If nothing is ready, it goes back to sleep. |

The scheduler **doesn't need to know about your interrupts.** It simply re-checks task readiness every time it wakes up. The idle loop is:

1. Run ready tasks
2. No tasks ready? → Sleep until the next deadline (or forever)
3. Wake up (timer *or* interrupt) → goto 1

!!! important "Polling Tasks Prevent Tickless Sleep"
    `syn_sched_next_wakeup()` returns `now` whenever any `SYN_TASK_READY` task has `delay_until == 0` — including any task yielded via `PT_PASS(pt)` or polling on `PT_WAIT_UNTIL(pt, cond)` / `PT_DELAY_US(...)`. Whenever `wake == now`, sleep entry is skipped immediately. In a tickless system, **every** task must park in `PT_TASK_DELAY_MS`, `PT_BLOCK_*` (`SYN_TASK_BLOCKED`), `SYN_TASK_SUSPENDED`, or `SYN_TASK_DEAD` when idle.

### Wakelocks & Sleep Control

If a peripheral needs the CPU to stay awake (e.g., mid-UART or DMA transfer), acquire a named lock bit on the `SYN_Sleep` coordinator:

```c
#define LOCK_UART_TX SYN_SLEEP_LOCK_UART

syn_sleep_lock(&sleep, LOCK_UART_TX);    // Prevent sleep (ISR-safe)
// ... do time-critical work ...
syn_sleep_unlock(&sleep, LOCK_UART_TX);  // Allow sleep again
```

While any wakelock bit is held, when `syn_sleep_enable(&sleep, false)` has disabled sleep globally, or when `sleep` is configured with `SYN_SLEEP_NONE`, `syn_sched_run_tickless()` and `syn_sched_run_tickless_ex()` skip hardware sleep and busy-loop like `syn_sched_run_forever()`.

### Example: Button + Periodic Task

```c
// syn_config.h (must be visible when compiling syn_sched.c as well as app TUs)
#define SYN_USE_TICKLESS 1
```

```c
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

// Task B: respond to button press (EXTI sets event flag and wakes CPU)
static SYN_PT_Status button_task(SYN_PT *pt, SYN_Task *task) {
    PT_BEGIN(pt);
    for (;;) {
        PT_BLOCK_EVENT(pt, task, &btn_events, EVT_BUTTON_PRESSED);
        handle_button();
    }
    PT_END(pt);
}

int main(void) {
    syn_event_init(&btn_events);
    syn_task_create(&tasks[0], "blink",  blink_task,  SYN_PRIO_NORMAL,   NULL);
    syn_task_create(&tasks[1], "button", button_task, SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&sched, tasks, 2);
    syn_sleep_init(&sleep, SYN_SLEEP_DEEP);

    // CPU sleeps between 1-second blinks.
    // Button EXTI wakes it early when pressed.
    syn_sched_run_tickless(&sched, &sleep);
}
```

Between blinks, `blink_task` is delayed and `button_task` is `SYN_TASK_BLOCKED`, so the CPU enters low-power mode for ~1 second. If a button EXTI fires at 500 ms, the CPU wakes immediately, the ISR calls `syn_event_set(&btn_events, EVT_BUTTON_PRESSED)`, and the scheduler runs `button_task` before returning to sleep for the remaining ~500 ms until the next blink.

### Port Requirement

Implement `syn_port_sleep_until(uint32_t wake_tick_ms)` in your platform port. This function must:

1. Program a hardware wake timer (RTC alarm, LPTIM compare, etc.) for `wake_tick_ms`
2. Enter a low-power mode (e.g., `__WFI()` on Cortex-M)
3. Return when the alarm fires **or** any interrupt wakes the CPU

The default weak stub falls back to `syn_port_sleep(SYN_SLEEP_LIGHT)` (no timer programming — just WFI).

### Timer-Aware Tickless (`run_tickless_ex`)

When using software timers alongside tickless idle, the basic `syn_sched_run_tickless()` only considers task `delay_until` deadlines. Software timer expirations won't wake the CPU — they'll fire late, after the next task wakes up.

`syn_sched_run_tickless_ex()` solves this by combining both:

```
                             syn_sched_run_tickless_ex()
┌────────────────────────────────────────────────────────────┐
│  1. Run scheduler (syn_sched_run)                          │
│  2. Service software timers (syn_timer_service)            │
│  3. sleep_until = min(next_task_wakeup, next_timer_expiry) │
│  4. Enter low-power sleep until sleep_until                │
│  5. goto 1                                                 │
└────────────────────────────────────────────────────────────┘
```

Requires both `SYN_USE_TICKLESS` and `SYN_USE_TIMER` to be enabled.

```c
// syn_config.h
#define SYN_USE_TICKLESS 1
#define SYN_USE_TIMER    1
```

```c
static SYN_Timer timers[2];
syn_timer_init(&timers[0], 100, true, sensor_poll_cb, NULL);
syn_timer_init(&timers[1], 5000, true, heartbeat_cb, NULL);
syn_timer_start(&timers[0]);
syn_timer_start(&timers[1]);

// CPU wakes for both task delays AND timer expirations
syn_sched_run_tickless_ex(&sched, &sleep, timers, 2);
```

`syn_timer_next_expiry()` is also available standalone if you need to query the earliest timer deadline:

```c
uint32_t next = syn_timer_next_expiry(timers, timer_count);
// UINT32_MAX if no active timers
```

---

## Multicore (AMP)

Enable `SYN_USE_MULTICORE` to add Asymmetric Multiprocessing support. Each core runs its own independent cooperative scheduler; cores communicate via the existing mailbox (upgraded with memory barriers) and protect shared peripherals with spinlocks.

**Key files:**

| File | Purpose |
|------|---------|
| `syn_barrier.h` | Acquire/release memory ordering primitives |
| `syn_port_spinlock.h` | Spinlock, core ID, and IPC notify port functions |
| `syn_spinlock.h` | Scoped `SYN_SPINLOCK_GUARD()` helper |
| `syn_mailbox.h` | SPSC mailbox (barrier-upgraded for cross-core safety) |

### Architecture

```
         Core 0                         Core 1
   ┌──────────────────┐          ┌──────────────────┐
   │ SYN_Sched sched0 │          │ SYN_Sched sched1 │
   │ tasks0[N]        │          │ tasks1[M]        │
   │                  │          │                  │
   │ run_forever()    │<-------->│ run_forever()    │
   │                  │ Mailbox  │                  │
   └──────────────────┘          └──────────────────┘
```

Each core owns its own scheduler, task array, and timers. No changes to the cooperative protothread model. Cross-core coordination uses only two primitives:
- **Mailbox** — typed message passing (SPSC, lock-free)
- **Spinlock** — mutual exclusion for shared hardware (UART, flash, etc.)

### Configuration

```c
// syn_config.h
#define SYN_USE_MULTICORE   1
#define SYN_SPINLOCK_COUNT  4  // number of spinlock IDs (default 4)
```

### Cross-Core Mailbox

The existing `syn_mailbox` is automatically upgraded with acquire/release barriers when `SYN_USE_MULTICORE=1`. On single-core builds, these compile to zero-cost plain volatile access.

```c
typedef struct { uint8_t id; int32_t value; } SensorMsg;

// Place in shared SRAM accessible to both cores
SYN_MAILBOX_DEFINE(ipc_mbox, SensorMsg, 16);

// Optional: wake consumer core on post
syn_mailbox_set_notify(&ipc_mbox, true);

// Core 0 (producer):
SensorMsg msg = { .id = 1, .value = 42 };
syn_mailbox_post(&ipc_mbox, &msg);

// Core 1 (consumer):
SensorMsg rx;
if (syn_mailbox_receive(&ipc_mbox, &rx)) {
    handle(rx.id, rx.value);
}
```

> **Warning:** The mailbox is strictly **single-producer, single-consumer**. If you need multiple producers, serialize access with a spinlock.

### Spinlocks

Spinlocks protect shared resources (peripherals, log buffers) across cores. They disable interrupts on the acquiring core to prevent priority inversion.

```c
#include "syntropic/util/syn_spinlock.h"

// Scoped lock — guaranteed release on scope exit
SYN_SPINLOCK_GUARD(SYN_SPINLOCK_UART) {
    syn_port_uart_transmit(0, data, len, 10);
}

// Manual lock (for more control)
syn_port_spinlock_acquire(SYN_SPINLOCK_FLASH);
syn_port_flash_write(addr, buf, len);
syn_port_spinlock_release(SYN_SPINLOCK_FLASH);
```

Well-known lock IDs:

| ID | Macro | Purpose |
|----|-------|---------|
| 0 | `SYN_SPINLOCK_UART` | Shared UART |
| 1 | `SYN_SPINLOCK_FLASH` | Shared flash |
| 2 | `SYN_SPINLOCK_USER0` | Application use |
| 3 | `SYN_SPINLOCK_USER1` | Application use |

### Porting Guide

Implement these functions for your platform:

```c
// Required:
void     syn_port_spinlock_acquire(uint8_t id);   // disable IRQ + spin
void     syn_port_spinlock_release(uint8_t id);   // release + restore IRQ
bool     syn_port_spinlock_try_acquire(uint8_t id);
uint8_t  syn_port_core_id(void);                  // return 0 or 1
void     syn_port_memory_barrier(void);           // DMB on ARM, __sync_synchronize fallback

// Optional (no-op stub provided):
void     syn_port_ipc_notify(void);               // SEV on ARM, triggers WFE wakeup
```

**RP2040 example** — hardware spinlocks:

```c
#include "hardware/sync.h"

static spin_lock_t *locks[SYN_SPINLOCK_COUNT];
static uint32_t saved_irq[SYN_SPINLOCK_COUNT];

void syn_port_spinlock_acquire(uint8_t id) {
    saved_irq[id] = spin_lock_blocking(locks[id]);
}

void syn_port_spinlock_release(uint8_t id) {
    spin_unlock(locks[id], saved_irq[id]);
}

void syn_port_memory_barrier(void) {
    __dmb();  // Data Memory Barrier
}

void syn_port_ipc_notify(void) {
    __sev();  // Send Event — wakes other core from WFE
}
```
