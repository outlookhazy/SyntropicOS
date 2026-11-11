# Cooperative Kernel & Async Dispatch Invariants (`src/syntropic/sched/`)

Applies to `syn_sched`, `syn_task`, `syn_mutex`, `syn_event_flags`, `syn_ao`, `syn_timer`, `syn_timer_wheel`, `syn_watchdog`, `syn_sequencer`, `syn_workqueue`, and `syn_mailbox`.

## Cross-Directory Touchpoints

- **Protothreads**: `src/syntropic/pt/syn_pt.h` (`PT_IDLE`, `PT_MORE`, `PT_YIELDED`, `PT_EXITED`, `PT_ENDED`) and `syn_pt_sem.h`; scheduler-aware cooperative yield macros `PT_CONTINUE_IF_NEEDED` and `PT_SPAWN_IF_NEEDED` in `src/syntropic/sched/syn_sched.h`.
- **System & Port**: `src/syntropic/system/syn_sleep.h` (tickless sleep coordinator), `src/syntropic/port/syn_port_system.h` (`syn_port_enter_critical`, `syn_port_get_tick_ms`, `syn_port_sleep_until`), `src/syntropic/common/syn_barrier.h`, and `src/syntropic/util/syn_event.h`.
- **Host Mock**: `tests/unit/mocks/mock_port.c` simulates ticks, critical-section nesting, and sleep hooks in unit tests.

## Kernel & Concurrency Invariants

- **Ranked Cooperative Dispatch & Fairness (`syn_sched`)**: `PT_MORE` / `PT_PASS` tasks have remaining burst work while `PT_IDLE` tasks are polling/waiting. Maintain accurate `due_ms` starvation deadlines and `syn_task_should_share()` state across async ISR wakeups (`syn_task_resume`), peer wakeups, and mid-slice task creation/restart on `SYN_TASK_DEAD` slots.
- **Task Lifecycle & Stale State Cleanup**: Task transitions (`syn_task_create` on uninitialized or `SYN_TASK_DEAD` slots, `syn_task_restart` on active slots, `syn_task_suspend`, exit/end) must execute atomically with respect to ISR wakeups, release or prune held/waited mutex state, clear stale `wait_event` pointers, and preserve scheduler back-pointers when re-initializing registered slots.
- **Transitive Priority Inheritance & Single-Scheduler AMP Scope (`syn_mutex`, `syn_sched`)**: `SYN_Mutex` is strictly intra-scheduler — all scheduled tasks (`task->sched != NULL`) acquiring or contending on a mutex must belong to the same `SYN_Sched` instance (`SYN_INVALID_PARAM` otherwise; cross-core AMP sync uses `syn_port_spinlock.h` or SPSC `syn_mailbox`). Within a scheduler, priority boosts and cycle-safe demotions must propagate transitively across chains, recompute effective priority from remaining held mutexes and manual boosts on unlock/eviction, and preserve wait flags across recursive relocks.
- **32-Bit Tick Wraparound (`uint32_t`)**: Always compare ticks via signed modular subtraction `(int32_t)(now - deadline) >= 0`. Guard periodic `delay_until` anchors when tasks remain `SYN_TASK_SUSPENDED` across half-range (`>24.85` days) so stale anchors do not appear in the future.
- **Callback & Self-Post Reentrancy (`syn_timer_wheel`, `syn_sequencer`, `syn_ao`, `syn_workqueue`)**: Copy node/step state to stack and unlink/advance before invoking user callbacks so reentrant `re-init`, `start`, `stop`, or loop toggles cannot corrupt intrusive links. Bound single-dispatch event bursts in `syn_ao` so self-posting handlers cannot starve peer tasks.
