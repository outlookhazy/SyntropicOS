#include "../common/syn_defs.h"

#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED

/**
 * @file syn_sched.c
 * @brief Cooperative scheduler implementation.
 */

#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "../util/syn_event.h"
#include "../util/syn_metrics.h"
#include "syn_sched.h"
#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX
#include "syn_mutex.h"
#endif

#include <limits.h>
#include <string.h>

#if defined(SYN_USE_METRICS) && SYN_USE_METRICS
SYN_METRIC_DECLARE(sched_ticks, "sched_ticks", "Total scheduler run calls",
                   SYN_METRIC_TYPE_COUNTER);
SYN_METRIC_DECLARE(sched_switches, "sched_switches", "Total task executions",
                   SYN_METRIC_TYPE_COUNTER);
#endif

/* ── Internal rank constants & active dispatch tracking ─────────────────── */

/** @cond INTERNAL */
#define SCHED_RANK_OVERDUE 0U
#define SCHED_RANK_WORK 1U
#define SCHED_RANK_POLL 2U
#define SCHED_RANK_PASSED 3U
#define SCHED_RANK_NONE 4U
/** @endcond */

/* ── Initialization ─────────────────────────────────────────────────────── */

/** @cond INTERNAL */
static uint16_t sched_clamp_wait_ms(uint16_t ms)
{
    return (ms < 1U) ? 1U : ms;
}
/** @endcond */

void syn_sched_init(SYN_Sched *sched, SYN_Task *tasks, size_t count)
{
    SYN_ASSERT(sched != NULL);
    SYN_ASSERT(tasks != NULL || count == 0);
    SYN_ASSERT(count <= UINT16_MAX);

    /* LCOV_EXCL_START: Defensive NULL check or invalid parameter fallback */
    if (sched == NULL || (tasks == NULL && count > 0) || count > UINT16_MAX) {
        return;
    }
    /* LCOV_EXCL_STOP */

    sched->tasks = tasks;
    sched->task_count = count;
    sched->share_deadline_ms = 0U;
    sched->running = NULL;
    sched->standalone_mutexes = NULL;
    sched->share_contended = false;
    sched->share_has_deadline = false;
    sched->share_watch_wakeups = false;

    for (size_t i = 0U; i < count; i++) {
        tasks[i].sched = sched;
    }

    for (size_t i = 0; i < SYN_SCHED_PRIO_LEVELS; i++) {
        sched->rr_per_prio[i] = 0U;
        sched->max_wait_ms[i] = sched_clamp_wait_ms((uint16_t)SYN_SCHED_MAX_WAIT_LOW_MS);
    }
#if SYN_SCHED_PRIO_LEVELS > 0
    sched->max_wait_ms[0] = sched_clamp_wait_ms((uint16_t)SYN_SCHED_MAX_WAIT_CRITICAL_MS);
#endif
#if SYN_SCHED_PRIO_LEVELS > 1
    sched->max_wait_ms[1] = sched_clamp_wait_ms((uint16_t)SYN_SCHED_MAX_WAIT_HIGH_MS);
#endif
#if SYN_SCHED_PRIO_LEVELS > 2
    sched->max_wait_ms[2] = sched_clamp_wait_ms((uint16_t)SYN_SCHED_MAX_WAIT_NORMAL_MS);
#endif

    SYN_METRIC_REGISTER(sched_ticks);
    SYN_METRIC_REGISTER(sched_switches);
}

void syn_sched_set_max_wait(SYN_Sched *sched, SYN_Prio prio, uint16_t ms)
{
    SYN_ASSERT(sched != NULL);
    SYN_ASSERT((uint32_t)prio < (uint32_t)SYN_SCHED_PRIO_LEVELS);

    /* LCOV_EXCL_START: Defensive NULL check or invalid parameter fallback */
    if (sched == NULL || (uint32_t)prio >= (uint32_t)SYN_SCHED_PRIO_LEVELS) {
        return;
    }
    /* LCOV_EXCL_STOP */

    sched->max_wait_ms[(uint32_t)prio] = sched_clamp_wait_ms(ms);
}

void syn_task_create(SYN_Task *task, const char *name, SYN_TaskFunc func, SYN_Prio priority,
                     void *user_data)
{
    SYN_ASSERT(task != NULL);
    SYN_ASSERT(func != NULL);
    SYN_ASSERT((uint32_t)priority < (uint32_t)SYN_SCHED_PRIO_LEVELS);

    if (task == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check */
    }
    syn_port_enter_critical();
    if (func == NULL || (uint32_t)priority >= (uint32_t)SYN_SCHED_PRIO_LEVELS) {
        /* LCOV_EXCL_START: Defensive invalid parameter fallback */
        memset(task, 0, sizeof(*task));
        task->state = (uint8_t)SYN_TASK_DEAD;
        syn_port_exit_critical();
        return;
        /* LCOV_EXCL_STOP */
    }

    PT_INIT(&task->pt);
    task->manual_priority = SYN_TASK_NO_MANUAL_PRIO;
    task->func = func;
    task->name = name;
    task->priority = (uint8_t)priority;
    task->base_priority = (uint8_t)priority;
    task->state = (uint8_t)SYN_TASK_READY;
    task->sched_flags = SYN_SCHED_FLAG_MORE;
    task->delay_until = 0U;
    task->due_ms = 0U;
    task->sched = NULL;
    task->held_mutexes = NULL;
    task->user_data = user_data;
    task->wait_event = NULL;
    task->wait_mask = 0U;
    SYN_PT_STATE_BARRIER();
    syn_port_exit_critical();
}

/** @cond INTERNAL */
static bool sched_is_event_ready(const SYN_Task *task, uint32_t now)
{
    if (task->wait_event != NULL &&
        (syn_event_flags_get(task->wait_event) & task->wait_mask) != 0U) {
        return true;
    }
    return (task->delay_until != 0U) && ((int32_t)(now - task->delay_until) >= 0);
}

static bool sched_is_async_wake(const SYN_Task *peer, uint32_t now)
{
    if ((peer->sched_flags & SYN_SCHED_FLAG_SLEEPING) == 0U) {
        return false;
    }
    if (peer->state == (uint8_t)SYN_TASK_SUSPENDED || peer->state == (uint8_t)SYN_TASK_DEAD) {
        return false;
    }
    if (peer->state == (uint8_t)SYN_TASK_BLOCKED || peer->wait_event != NULL) {
        return peer->wait_event != NULL && sched_is_event_ready(peer, now);
    }
    return (peer->sched_flags & SYN_SCHED_FLAG_ISR_WAKE) != 0U &&
           (peer->delay_until == 0U || (int32_t)(now - peer->delay_until) >= 0);
}

static uint32_t sched_wakeup_base_tick(const SYN_Task *task, uint32_t now, bool sleeping)
{
    if (sleeping && task->delay_until != 0U && (int32_t)(now - task->delay_until) >= 0 &&
        (task->wait_event == NULL ||
         (syn_event_flags_get(task->wait_event) & task->wait_mask) == 0U)) {
        return task->delay_until;
    }
    return now;
}

static bool sched_peer_woke(const SYN_Sched *sched, const SYN_Task *running, SYN_Task *peer,
                            uint32_t now)
{
    syn_port_enter_critical();
    if (peer->sched == NULL && peer->state == (uint8_t)SYN_TASK_READY) {
        peer->sched = (SYN_Sched *)sched;
        peer->sched_flags |= (uint8_t)(SYN_SCHED_FLAG_SLEEPING | SYN_SCHED_FLAG_ISR_WAKE);
    }
    if (!sched_is_async_wake(peer, now)) {
        syn_port_exit_critical();
        return false;
    }
    if (peer->priority <= running->priority) {
        syn_port_exit_critical();
        return true;
    }
    if ((peer->sched_flags & SYN_SCHED_FLAG_DUE_VALID) == 0U) {
        uint32_t base = sched_wakeup_base_tick(peer, now, true);
        peer->due_ms = base + (uint32_t)sched->max_wait_ms[peer->priority];
        peer->sched_flags |= SYN_SCHED_FLAG_DUE_VALID;
    }
    uint32_t due = peer->due_ms;
    syn_port_exit_critical();
    return (int32_t)(now - due) >= 0;
}
/** @endcond */

bool syn_task_should_share(const SYN_Task *task)
{
    if (task == NULL || task->sched == NULL || task->sched->running != task) {
        return false;
    }

    const SYN_Sched *sched = task->sched;
    if (sched->share_contended) {
        return true;
    }

    uint32_t now = syn_port_get_tick_ms();
    if (sched->share_has_deadline && (int32_t)(now - sched->share_deadline_ms) >= 0) {
        return true;
    }
    if (!sched->share_watch_wakeups) {
        return false;
    }

    for (size_t i = 0U; i < sched->task_count; i++) {
        SYN_Task *peer = &sched->tasks[i];
        if (peer != task && sched_peer_woke(sched, task, peer, now)) {
            return true;
        }
    }
    return false;
}

/* ── Ranked dispatch helpers ────────────────────────────────────────────── */

/** @cond INTERNAL */
static void sched_stamp_awake_if_needed(const SYN_Sched *sched, SYN_Task *task, uint32_t now)
{
    uint8_t flags = task->sched_flags;
    bool sleeping = (flags & SYN_SCHED_FLAG_SLEEPING) != 0U;
    bool had_due = (flags & SYN_SCHED_FLAG_DUE_VALID) != 0U;
    if (sleeping || !had_due) {
        uint8_t keep = (uint8_t)(flags & (SYN_SCHED_FLAG_ISR_WAKE | SYN_SCHED_FLAG_MUTEX_WAIT));
        task->sched_flags = (uint8_t)(keep | SYN_SCHED_FLAG_MORE | SYN_SCHED_FLAG_DUE_VALID);
        if (!had_due) {
            uint32_t base = sched_wakeup_base_tick(task, now, sleeping);
            task->due_ms = base + (uint32_t)sched->max_wait_ms[task->priority];
        }
    }
}

static bool sched_prepare_task(SYN_Sched *sched, SYN_Task *task, uint32_t now, bool *any_alive)
{
    task->sched = sched;
    if (task->state == (uint8_t)SYN_TASK_DEAD) {
        return false;
    }

    *any_alive = true;
    syn_port_enter_critical();

    if (task->state == (uint8_t)SYN_TASK_SUSPENDED) {
        if (task->delay_until != 0U && (int32_t)(now - task->delay_until) >= 0) {
            task->delay_until = (now == 0U) ? 0xFFFFFFFFU : now;
        }
        task->sched_flags = SYN_SCHED_FLAG_SLEEPING;
        syn_port_exit_critical();
        return false;
    }

    if (task->state == (uint8_t)SYN_TASK_BLOCKED || task->wait_event != NULL) {
        if (!sched_is_event_ready(task, now)) {
            task->state = (uint8_t)SYN_TASK_BLOCKED;
            task->sched_flags = SYN_SCHED_FLAG_SLEEPING;
            syn_port_exit_critical();
            return false;
        }
        sched_stamp_awake_if_needed(sched, task, now);
        syn_port_exit_critical();
        return true;
    }

    if (task->delay_until != 0U && (int32_t)(now - task->delay_until) < 0) {
        task->sched_flags = SYN_SCHED_FLAG_SLEEPING;
        syn_port_exit_critical();
        return false;
    }

    sched_stamp_awake_if_needed(sched, task, now);
    syn_port_exit_critical();
    return true;
}

static void sched_resleep_event_peers(SYN_Sched *sched, const SYN_Task *winner,
                                      const SYN_EventFlags *evt, uint32_t now)
{
    for (size_t i = 0U; i < sched->task_count; i++) {
        SYN_Task *peer = &sched->tasks[i];
        if (peer != winner && peer->wait_event == evt && peer->state != (uint8_t)SYN_TASK_DEAD &&
            peer->state != (uint8_t)SYN_TASK_SUSPENDED &&
            (peer->sched_flags & SYN_SCHED_FLAG_SLEEPING) == 0U &&
            !sched_is_event_ready(peer, now)) {
            peer->state = (uint8_t)SYN_TASK_BLOCKED;
            peer->sched_flags = SYN_SCHED_FLAG_SLEEPING;
        }
    }
}

static void sched_resolve_event_wait(SYN_Sched *sched, SYN_Task *task, uint32_t now)
{
    syn_port_enter_critical();
    SYN_EventFlags *evt = task->wait_event;
    uint32_t matched = 0U;
    (void)syn_event_flags_wait(
        evt, task->wait_mask,
        (uint32_t)SYN_EVENT_FLAGS_WAIT_ANY | (uint32_t)SYN_EVENT_FLAGS_AUTO_CLEAR, &matched);
    bool timed_out = (task->delay_until != 0U) && ((int32_t)(now - task->delay_until) >= 0);
    if (matched != 0U || timed_out) {
        task->wait_mask = matched;
        task->wait_event = NULL;
        task->delay_until = 0U;
        if (task->state == (uint8_t)SYN_TASK_BLOCKED) {
            task->state = (uint8_t)SYN_TASK_READY;
        }
    }
    sched_resleep_event_peers(sched, task, evt, now);
    syn_port_exit_critical();
}

static uint8_t sched_task_rank(const SYN_Task *task, uint32_t now)
{
    if ((int32_t)(now - task->due_ms) >= 0) {
        return SCHED_RANK_OVERDUE;
    }
    if ((task->sched_flags & SYN_SCHED_FLAG_MORE) != 0U) {
        return SCHED_RANK_WORK;
    }
    if ((task->sched_flags & SYN_SCHED_FLAG_PASSED) == 0U) {
        return SCHED_RANK_POLL;
    }
    return SCHED_RANK_PASSED;
}

static bool sched_is_better_candidate(uint8_t rank, uint8_t prio, size_t dist, uint8_t best_rank,
                                      uint8_t best_prio, size_t best_dist)
{
    if (rank != best_rank) {
        return rank < best_rank;
    }
    if (prio != best_prio) {
        return prio < best_prio;
    }
    return dist < best_dist;
}

static SYN_Task *sched_select_best(SYN_Sched *sched, uint32_t now, bool *any_alive,
                                   uint8_t *out_rank, size_t *out_idx)
{
    const size_t n = sched->task_count;
    SYN_Task *best_task = NULL;
    size_t best_idx = 0U;
    uint8_t best_rank = SCHED_RANK_NONE;
    uint8_t best_prio = 255U;
    size_t best_dist = n;

    for (size_t i = 0U; i < n; i++) {
        SYN_Task *task = &sched->tasks[i];
        if (!sched_prepare_task(sched, task, now, any_alive)) {
            continue;
        }

        const uint8_t rank = sched_task_rank(task, now);
        const uint8_t prio = task->priority;
        size_t rr_start = sched->rr_per_prio[prio];
        if (rr_start >= n) {
            rr_start = 0U;
        }
        const size_t dist = (i >= rr_start) ? (i - rr_start) : (n - rr_start + i);

        if (sched_is_better_candidate(rank, prio, dist, best_rank, best_prio, best_dist)) {
            best_rank = rank;
            best_prio = prio;
            best_dist = dist;
            best_task = task;
            best_idx = i;
        }
    }

    *out_rank = best_rank;
    *out_idx = best_idx;
    return best_task;
}

static void sched_fold_deadline(uint32_t candidate, uint32_t *earliest, bool *has_deadline)
{
    if (!(*has_deadline) || (int32_t)(candidate - *earliest) < 0) {
        *earliest = candidate;
        *has_deadline = true;
    }
}

static bool sched_is_peer_runnable_now(const SYN_Task *t, uint32_t now)
{
    if (t->state == (uint8_t)SYN_TASK_SUSPENDED) {
        return false;
    }
    if (t->state == (uint8_t)SYN_TASK_BLOCKED || t->wait_event != NULL) {
        return sched_is_event_ready(t, now);
    }
    return t->delay_until == 0U || (int32_t)(now - t->delay_until) >= 0;
}

static void sched_inspect_ready_peer(const SYN_Sched *sched, SYN_Task *t, uint8_t run_prio,
                                     uint32_t now, bool *contended, uint32_t *earliest_due,
                                     bool *has_deadline)
{
    uint8_t flags = t->sched_flags;
    bool sleeping = (flags & SYN_SCHED_FLAG_SLEEPING) != 0U;
    if ((sleeping || (flags & SYN_SCHED_FLAG_MORE) != 0U) && t->priority <= run_prio) {
        *contended = true;
    }
    uint32_t due;
    if ((flags & SYN_SCHED_FLAG_DUE_VALID) != 0U) {
        due = t->due_ms;
    } else {
        uint32_t base = sched_wakeup_base_tick(t, now, sleeping);
        due = base + (uint32_t)sched->max_wait_ms[t->priority];
        t->due_ms = due;
        t->sched_flags = (uint8_t)(flags | SYN_SCHED_FLAG_DUE_VALID);
    }

    if ((int32_t)(now - due) > 0) {
        due = now;
    }
    sched_fold_deadline(due, earliest_due, has_deadline);
}

static void sched_fold_sleeping_delay(const SYN_Sched *sched, const SYN_Task *t, uint8_t run_prio,
                                      uint32_t now, uint32_t *earliest_due, bool *has_deadline)
{
    uint32_t rem = ((int32_t)(now - t->delay_until) >= 0) ? 0U : (t->delay_until - now);
    uint32_t extra = (t->priority > run_prio) ? (uint32_t)sched->max_wait_ms[t->priority] : 0U;
    uint32_t max_rem = (uint32_t)INT32_MAX;
    uint32_t clamped = (rem > (max_rem - extra)) ? max_rem : (rem + extra);
    sched_fold_deadline(now + clamped, earliest_due, has_deadline);
}

static void sched_inspect_sleeping_peer(const SYN_Sched *sched, const SYN_Task *t, uint8_t run_prio,
                                        uint32_t now, uint32_t *earliest_due, bool *has_deadline,
                                        bool *watch_wakeups)
{
    if (t->state != (uint8_t)SYN_TASK_SUSPENDED && t->delay_until != 0U) {
        sched_fold_sleeping_delay(sched, t, run_prio, now, earliest_due, has_deadline);
    }
    if (t->state != (uint8_t)SYN_TASK_READY || t->wait_event != NULL) {
        *watch_wakeups = true;
    }
}

static void sched_update_share_state(SYN_Sched *sched, const SYN_Task *running, uint32_t now)
{
    const uint8_t run_prio = running->priority;
    bool contended = false;
    bool has_deadline = false;
    bool watch_wakeups = false;
    uint32_t earliest_due = 0U;

    for (size_t i = 0U; i < sched->task_count; i++) {
        SYN_Task *t = &sched->tasks[i];
        if (t == running) {
            continue;
        }
        if (t->state == (uint8_t)SYN_TASK_DEAD) {
            watch_wakeups = true;
            continue;
        }

        syn_port_enter_critical();
        if (sched_is_peer_runnable_now(t, now)) {
            sched_inspect_ready_peer(sched, t, run_prio, now, &contended, &earliest_due,
                                     &has_deadline);
        } else {
            sched_inspect_sleeping_peer(sched, t, run_prio, now, &earliest_due, &has_deadline,
                                        &watch_wakeups);
        }
        syn_port_exit_critical();
    }

    sched->share_contended = contended;
    sched->share_has_deadline = has_deadline;
    sched->share_watch_wakeups = watch_wakeups;
    sched->share_deadline_ms = earliest_due;
}

static void sched_refresh_share_locked(SYN_Sched *sched)
{
    if (sched != NULL && sched->running != NULL) {
        sched_update_share_state(sched, sched->running, syn_port_get_tick_ms());
    }
}

static bool sched_should_advance_rr(const SYN_Sched *sched, size_t idx, uint8_t dispatch_prio,
                                    uint8_t best_rank)
{
    if (best_rank != SCHED_RANK_OVERDUE) {
        return true;
    }
    const size_t n = sched->task_count;
    size_t pos = sched->rr_per_prio[dispatch_prio];
    if (pos >= n) {
        pos = 0U;
    }
    while (pos != idx) {
        const SYN_Task *p = &sched->tasks[pos];
        if (p->state == (uint8_t)SYN_TASK_READY && p->priority == dispatch_prio &&
            (p->sched_flags & SYN_SCHED_FLAG_MORE) != 0U &&
            (p->sched_flags & SYN_SCHED_FLAG_SLEEPING) == 0U) {
            return false;
        }
        pos = (pos + 1U >= n) ? 0U : (pos + 1U);
    }
    return true;
}

#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX
static bool sched_is_mutex_waiter_active(const SYN_Task *w)
{
    return (w != NULL) && ((w->sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U) &&
           (w->state == (uint8_t)SYN_TASK_READY) && (w->delay_until == 0U);
}

static void sched_prune_mutex_slots(SYN_Mutex *m)
{
    for (size_t i = 0U; i < SYN_MUTEX_MAX_WAITERS; i++) {
        if (!sched_is_mutex_waiter_active(m->waiters[i])) {
            m->waiters[i] = NULL;
        }
    }
}

static void sched_prune_all_mutex_waiters(SYN_Sched *sched)
{
    for (SYN_Mutex *m = sched->standalone_mutexes; m != NULL; m = m->next_contended) {
        sched_prune_mutex_slots(m);
    }
    for (size_t i = 0U; i < sched->task_count; i++) {
        for (SYN_Mutex *m = sched->tasks[i].held_mutexes; m != NULL; m = m->next_held) {
            sched_prune_mutex_slots(m);
        }
    }
}

static void sched_release_held_mutexes(SYN_Task *task)
{
    for (;;) {
        syn_port_enter_critical();
        SYN_Mutex *m = task->held_mutexes;
        if (m != NULL) {
            m->lock_count = 1U;
        }
        syn_port_exit_critical();
        if (m == NULL) {
            break;
        }
        (void)syn_mutex_unlock(m, task);
    }
}
#endif

static uint8_t sched_intrinsic_prio(const SYN_Task *task)
{
    return (task->manual_priority < task->base_priority) ? task->manual_priority
                                                         : task->base_priority;
}

static uint8_t sched_effective_prio(const SYN_Task *task)
{
    uint8_t target = sched_intrinsic_prio(task);
#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX
    for (const SYN_Mutex *m = task->held_mutexes; m != NULL; m = m->next_held) {
        for (size_t i = 0U; i < SYN_MUTEX_MAX_WAITERS; i++) {
            const SYN_Task *w = m->waiters[i];
            if (sched_is_mutex_waiter_active(w) && w->priority < target) {
                target = w->priority;
            }
        }
    }
#endif
    return target;
}

static void sched_apply_prio_target(SYN_Task *task, uint8_t target)
{
    if (target < task->priority) {
        uint8_t clr = (uint8_t)~SYN_SCHED_FLAG_PASSED;
        task->sched_flags = (uint8_t)((task->sched_flags & clr) | SYN_SCHED_FLAG_MORE);
        if (task->sched != NULL && (task->sched_flags & SYN_SCHED_FLAG_DUE_VALID) != 0U) {
            uint32_t boosted_due =
                syn_port_get_tick_ms() + (uint32_t)task->sched->max_wait_ms[target];
            if ((int32_t)(boosted_due - task->due_ms) < 0) {
                task->due_ms = boosted_due;
            }
        }
    }
    task->priority = target;
}

#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX
static void sched_boost_local_owners(SYN_Sched *sched)
{
    bool changed = true;
    for (size_t pass = 0U; pass < sched->task_count && changed; pass++) {
        changed = false;
        for (size_t i = 0U; i < sched->task_count; i++) {
            SYN_Task *t = &sched->tasks[i];
            if (t->held_mutexes != NULL) {
                uint8_t target = sched_effective_prio(t);
                if (target < t->priority) {
                    sched_apply_prio_target(t, target);
                    changed = true;
                }
            }
        }
    }
}

static void sched_boost_mutex_owners_locked(SYN_Sched *sched)
{
    sched_boost_local_owners(sched);
    for (SYN_Mutex *m = sched->standalone_mutexes; m != NULL; m = m->next_contended) {
        SYN_Task *owner = m->owner;
        if (owner != NULL) {
            uint8_t target = sched_effective_prio(owner);
            if (target < owner->priority) {
                sched_apply_prio_target(owner, target);
            }
        }
    }
}

static void sched_propagate_local_prios(SYN_Sched *sched)
{
    for (size_t i = 0U; i < sched->task_count; i++) {
        SYN_Task *t = &sched->tasks[i];
        if (t->held_mutexes != NULL) {
            t->priority = sched_intrinsic_prio(t);
        }
    }

    bool changed = true;
    for (size_t pass = 0U; pass < sched->task_count && changed; pass++) {
        changed = false;
        for (size_t i = 0U; i < sched->task_count; i++) {
            SYN_Task *t = &sched->tasks[i];
            if (t->held_mutexes != NULL) {
                uint8_t target = sched_effective_prio(t);
                if (target != t->priority) {
                    t->priority = target;
                    changed = true;
                }
            }
        }
    }
}

static void sched_propagate_ext_owners(const SYN_Sched *sched)
{
    for (SYN_Mutex *m = sched->standalone_mutexes; m != NULL; m = m->next_contended) {
        if (m->owner != NULL) {
            m->owner->priority = sched_effective_prio(m->owner);
        }
    }
}
#endif

static void sched_recompute_mutex_owners_locked(SYN_Sched *sched)
{
    if (sched == NULL) {
        return;
    }
#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX
    sched_prune_all_mutex_waiters(sched);
    sched_propagate_local_prios(sched);
    sched_propagate_ext_owners(sched);
#endif
    sched_refresh_share_locked(sched);
}

static void sched_advance_rr_if_needed(SYN_Sched *sched, size_t idx, uint8_t dispatch_prio,
                                       uint8_t best_rank)
{
    if (sched_should_advance_rr(sched, idx, dispatch_prio, best_rank)) {
        const size_t n = sched->task_count;
        const size_t next = idx + 1U;
        sched->rr_per_prio[dispatch_prio] = (next >= n) ? 0U : (uint16_t)next;
    }
}

static void sched_apply_post_status(const SYN_Sched *sched, SYN_Task *task, SYN_PT_Status status)
{
    if ((task->sched_flags & SYN_SCHED_FLAG_RESTARTED) != 0U) {
        bool stay_suspended = (task->state == (uint8_t)SYN_TASK_SUSPENDED);
        PT_INIT(&task->pt);
        task->manual_priority = SYN_TASK_NO_MANUAL_PRIO;
        task->priority = task->base_priority;
        task->delay_until = 0U;
        task->wait_event = NULL;
        task->wait_mask = 0U;
        if (stay_suspended) {
            task->due_ms = 0U;
            task->sched_flags = SYN_SCHED_FLAG_SLEEPING;
        } else {
            task->state = (uint8_t)SYN_TASK_READY;
            task->sched_flags = (uint8_t)(SYN_SCHED_FLAG_MORE | SYN_SCHED_FLAG_DUE_VALID);
            task->due_ms = syn_port_get_tick_ms() + (uint32_t)sched->max_wait_ms[task->priority];
        }
        return;
    }
    if (status >= PT_EXITED) {
        task->state = (uint8_t)SYN_TASK_DEAD;
        task->sched_flags = 0U;
        task->delay_until = 0U;
        task->wait_event = NULL;
        task->wait_mask = 0U;
        return;
    }
    if (task->state != (uint8_t)SYN_TASK_READY || task->delay_until != 0U) {
        task->sched_flags = SYN_SCHED_FLAG_SLEEPING;
        return;
    }
    uint8_t keep_wait = (uint8_t)(task->sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT);
    bool more = (status == PT_MORE) || ((task->sched_flags & SYN_SCHED_FLAG_ISR_WAKE) != 0U);
    uint8_t flag = more ? SYN_SCHED_FLAG_MORE : SYN_SCHED_FLAG_PASSED;
    task->sched_flags = (uint8_t)(flag | SYN_SCHED_FLAG_DUE_VALID | keep_wait);
    task->due_ms = syn_port_get_tick_ms() + (uint32_t)sched->max_wait_ms[task->priority];
}

static void sched_finish_dispatch(SYN_Sched *sched, SYN_Task *task, size_t idx,
                                  uint8_t dispatch_prio, uint8_t best_rank, bool had_mutex_wait,
                                  SYN_PT_Status status)
{
    sched_advance_rr_if_needed(sched, idx, dispatch_prio, best_rank);

    if (status >= PT_EXITED || (task->sched_flags & SYN_SCHED_FLAG_RESTARTED) != 0U) {
#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX
        sched_release_held_mutexes(task);
#endif
    }

    syn_port_enter_critical();
    bool mid_wait = (task->sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U;
    sched_apply_post_status(sched, task, status);
    if (had_mutex_wait || (mid_wait && (task->sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) == 0U)) {
        sched_recompute_mutex_owners_locked(sched);
    }
    syn_port_exit_critical();
}
/** @endcond */

/* ── Scheduler tick ─────────────────────────────────────────────────────── */

bool syn_sched_run(SYN_Sched *sched)
{
    SYN_ASSERT(sched != NULL);

    const size_t n = sched->task_count;
    if (n == 0U) {
        return false;
    }

    uint32_t now = syn_port_get_tick_ms();
    bool any_alive = false;
    uint8_t best_rank = SCHED_RANK_NONE;
    size_t best_idx = 0U;

    SYN_METRIC_INC(sched_ticks);

    SYN_Task *best_task = sched_select_best(sched, now, &any_alive, &best_rank, &best_idx);
    if (best_task == NULL) {
        return any_alive;
    }

    if (best_rank == SCHED_RANK_PASSED) {
        syn_port_enter_critical();
        for (size_t i = 0U; i < n; i++) {
            sched->tasks[i].sched_flags &= (uint8_t)~SYN_SCHED_FLAG_PASSED;
        }
        syn_port_exit_critical();
    }

    if (best_task->wait_event != NULL) {
        sched_resolve_event_wait(sched, best_task, now);
    }

    sched_update_share_state(sched, best_task, now);

    const uint8_t dispatch_prio = best_task->priority;
    SYN_Task *prev_running = sched->running;
    best_task->sched = sched;
    sched->running = best_task;

    syn_port_enter_critical();
    bool had_mutex_wait = (best_task->sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U;
    best_task->sched_flags &=
        (uint8_t)~(SYN_SCHED_FLAG_ISR_WAKE | SYN_SCHED_FLAG_MUTEX_WAIT | SYN_SCHED_FLAG_RESTARTED);
#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX
    if (had_mutex_wait) {
        sched_prune_all_mutex_waiters(sched);
    }
#endif
    syn_port_exit_critical();

    SYN_PT_Status status = best_task->func(&best_task->pt, best_task);

    SYN_METRIC_INC(sched_switches);
    if (sched->running != best_task) {
#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX
        if (status >= PT_EXITED || (best_task->sched_flags & SYN_SCHED_FLAG_RESTARTED) != 0U) {
            sched_release_held_mutexes(best_task);
        }
#endif
        return sched->task_count > 0U;
    }

    sched->running = prev_running;
    sched_finish_dispatch(sched, best_task, best_idx, dispatch_prio, best_rank, had_mutex_wait,
                          status);

    return any_alive;
}

SYN_NORETURN void syn_sched_run_forever(SYN_Sched *sched)
{
    for (;;) {
        syn_sched_run(sched);
    }
}

#if defined(SYN_USE_TICKLESS) && SYN_USE_TICKLESS

/** @cond INTERNAL */
static bool sched_next_wakeup_at(const SYN_Sched *sched, uint32_t now, uint32_t *out_earliest,
                                 bool *out_has_wake)
{
    uint32_t earliest = 0U;
    bool has_wake = false;
    bool any_ready_now = false;

    for (size_t i = 0U; i < sched->task_count; i++) {
        const SYN_Task *task = &sched->tasks[i];

        if (task->state == (uint8_t)SYN_TASK_DEAD || task->state == (uint8_t)SYN_TASK_SUSPENDED) {
            continue;
        }

        if (task->state == (uint8_t)SYN_TASK_BLOCKED || task->wait_event != NULL) {
            if (sched_is_event_ready(task, now)) {
                any_ready_now = true;
            } else if (task->delay_until != 0U) {
                sched_fold_deadline(task->delay_until, &earliest, &has_wake);
            }
            continue;
        }

        if (task->delay_until == 0U || (int32_t)(now - task->delay_until) >= 0) {
            any_ready_now = true;
            continue;
        }

        sched_fold_deadline(task->delay_until, &earliest, &has_wake);
    }

    *out_earliest = earliest;
    *out_has_wake = has_wake;
    return any_ready_now;
}

static void sched_try_tickless_sleep(SYN_Sleep *sleep, bool ready_now, bool has_wake, uint32_t wake)
{
    if (ready_now) {
        return;
    }
    if (!sleep->enabled || sleep->max_depth == SYN_SLEEP_NONE || syn_sleep_any_locked(sleep)) {
        sleep->veto_count++;
        return;
    }
    sleep->enter_count++;
    if (!has_wake) {
        syn_port_sleep(sleep->max_depth);
        return;
    }
    syn_port_sleep_until(wake);
}

#if !defined(SYN_USE_TIMER) || SYN_USE_TIMER
static bool sched_timer_wakeup_at(const SYN_Timer *timers, size_t count, uint32_t now,
                                  uint32_t *out_earliest, bool *out_has_wake)
{
    uint32_t earliest = 0U;
    bool has_wake = false;
    bool any_ready_now = false;

    if (timers == NULL) {
        *out_earliest = 0U;
        *out_has_wake = false;
        return false;
    }

    for (size_t i = 0U; i < count; i++) {
        if (!timers[i].active) {
            continue;
        }
        uint32_t target = timers[i].target_tick;
        if ((int32_t)(now - target) >= 0) {
            any_ready_now = true;
            continue;
        }
        sched_fold_deadline(target, &earliest, &has_wake);
    }

    *out_earliest = earliest;
    *out_has_wake = has_wake;
    return any_ready_now;
}
#endif
/** @endcond */

uint32_t syn_sched_next_wakeup(const SYN_Sched *sched)
{
    SYN_ASSERT(sched != NULL);

    uint32_t now = syn_port_get_tick_ms();
    uint32_t earliest = 0U;
    bool has_wake = false;
    if (sched_next_wakeup_at(sched, now, &earliest, &has_wake)) {
        return now;
    }
    if (!has_wake) {
        return UINT32_MAX;
    }
    return (earliest == UINT32_MAX) ? (UINT32_MAX - 1U) : earliest;
}

SYN_NORETURN void syn_sched_run_tickless(SYN_Sched *sched, SYN_Sleep *sleep)
{
    SYN_ASSERT(sched != NULL);
    SYN_ASSERT(sleep != NULL);

    for (;;) {
        syn_sched_run(sched);

        uint32_t hint_wake = 0U;
        bool hint_has_wake = false;
        if (sched_next_wakeup_at(sched, syn_port_get_tick_ms(), &hint_wake, &hint_has_wake)) {
            continue;
        }

        /* Wakeup evaluation and sleep entry must be atomic with a single now sample. */
        syn_port_enter_critical();
        uint32_t now = syn_port_get_tick_ms();
        uint32_t wake = 0U;
        bool has_wake = false;
        bool ready_now = sched_next_wakeup_at(sched, now, &wake, &has_wake);
        sched_try_tickless_sleep(sleep, ready_now, has_wake, wake);
        syn_port_exit_critical();
    }
}

#if !defined(SYN_USE_TIMER) || SYN_USE_TIMER

SYN_NORETURN void syn_sched_run_tickless_ex(SYN_Sched *sched, SYN_Sleep *sleep, SYN_Timer *timers,
                                            size_t timer_count)
{
    SYN_ASSERT(sched != NULL);
    SYN_ASSERT(sleep != NULL);

    for (;;) {
        syn_sched_run(sched);
        syn_timer_service(timers, timer_count);

        uint32_t hint_wake = 0U;
        bool hint_has_wake = false;
        if (sched_next_wakeup_at(sched, syn_port_get_tick_ms(), &hint_wake, &hint_has_wake)) {
            continue;
        }

        syn_port_enter_critical();
        uint32_t now = syn_port_get_tick_ms();
        uint32_t wake = 0U;
        bool has_wake = false;
        uint32_t timer_wake = 0U;
        bool has_timer_wake = false;
        bool ready_now =
            sched_next_wakeup_at(sched, now, &wake, &has_wake) ||
            sched_timer_wakeup_at(timers, timer_count, now, &timer_wake, &has_timer_wake);
        if (has_timer_wake) {
            sched_fold_deadline(timer_wake, &wake, &has_wake);
        }
        sched_try_tickless_sleep(sleep, ready_now, has_wake, wake);
        syn_port_exit_critical();
    }
}

#endif /* SYN_USE_TIMER */

#endif /* SYN_USE_TICKLESS */

/* ── Task control ───────────────────────────────────────────────────────── */

void syn_task_suspend(SYN_Task *task)
{
    SYN_ASSERT(task != NULL);
    if (task == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check */
    }
    syn_port_enter_critical();
    bool had_mutex_wait = (task->sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U;
    if (task->state != (uint8_t)SYN_TASK_DEAD) {
        uint8_t keep = (uint8_t)(task->sched_flags & SYN_SCHED_FLAG_RESTARTED);
        task->state = (uint8_t)SYN_TASK_SUSPENDED;
        task->sched_flags = (uint8_t)(SYN_SCHED_FLAG_SLEEPING | keep);
        SYN_PT_STATE_BARRIER();
    }
    if (had_mutex_wait) {
        sched_recompute_mutex_owners_locked(task->sched);
    } else {
        sched_refresh_share_locked(task->sched);
    }
    syn_port_exit_critical();
}

void syn_task_resume(SYN_Task *task)
{
    SYN_ASSERT(task != NULL);
    if (task == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check */
    }
    syn_port_enter_critical();
    if (task->state == (uint8_t)SYN_TASK_SUSPENDED ||
        (task->state == (uint8_t)SYN_TASK_BLOCKED && task->wait_event == NULL)) {
        uint8_t flags = (uint8_t)((task->sched_flags & (uint8_t)~SYN_SCHED_FLAG_PASSED) |
                                  SYN_SCHED_FLAG_MORE | SYN_SCHED_FLAG_ISR_WAKE);
        uint32_t now = syn_port_get_tick_ms();
        bool ready_now = (task->wait_event != NULL)
                             ? sched_is_event_ready(task, now)
                             : (task->delay_until == 0U || (int32_t)(now - task->delay_until) >= 0);
        if (task->sched != NULL && ready_now) {
            task->due_ms = now + (uint32_t)task->sched->max_wait_ms[task->priority];
            flags |= SYN_SCHED_FLAG_DUE_VALID;
        }
        task->sched_flags = flags;
        task->state = (uint8_t)SYN_TASK_READY;
        SYN_PT_STATE_BARRIER();
    }
    syn_port_exit_critical();
}

void syn_task_restart(SYN_Task *task)
{
    SYN_ASSERT(task != NULL);
    SYN_ASSERT(task->func != NULL);
    if (task == NULL || task->func == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check */
    }
#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX
    sched_release_held_mutexes(task);
#endif
    syn_port_enter_critical();
    bool had_mutex_wait = (task->sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U;
    uint8_t flags = SYN_SCHED_FLAG_MORE;
    if (task->sched != NULL && task->sched->running == task) {
        flags |= SYN_SCHED_FLAG_RESTARTED;
    }
    PT_INIT(&task->pt);
    task->manual_priority = SYN_TASK_NO_MANUAL_PRIO;
    task->priority = task->base_priority;
    task->delay_until = 0U;
    task->due_ms = 0U;
    task->wait_event = NULL;
    task->wait_mask = 0U;
    task->sched_flags = flags;
    task->state = (uint8_t)SYN_TASK_READY;
    SYN_PT_STATE_BARRIER();
    if (had_mutex_wait) {
        sched_recompute_mutex_owners_locked(task->sched);
    } else {
        sched_refresh_share_locked(task->sched);
    }
    syn_port_exit_critical();
}

size_t syn_sched_alive_count(const SYN_Sched *sched)
{
    SYN_ASSERT(sched != NULL);

    size_t count = 0;
    for (size_t i = 0; i < sched->task_count; i++) {
        if (sched->tasks[i].state != (uint8_t)SYN_TASK_DEAD) {
            count++;
        }
    }
    return count;
}

/** @cond INTERNAL */
static void sched_boost_from_task_locked(SYN_Task *task)
{
    uint8_t prev = task->priority;
    sched_apply_prio_target(task, sched_effective_prio(task));
#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX
    if (task->sched != NULL && task->priority < prev &&
        (task->sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U) {
        sched_boost_mutex_owners_locked(task->sched);
    }
#else
    (void)prev;
#endif
    sched_refresh_share_locked(task->sched);
}

static void sched_demote_from_task_locked(SYN_Task *task)
{
    if (task->sched != NULL) {
        if (task->held_mutexes == NULL) {
            task->priority = sched_intrinsic_prio(task);
        }
        sched_recompute_mutex_owners_locked(task->sched);
    } else {
        task->priority = sched_effective_prio(task);
    }
}

void syn_sched_boost_task_prio(SYN_Task *task)
{
    sched_boost_from_task_locked(task);
}

void syn_sched_recompute_task_prio(SYN_Task *task)
{
    sched_demote_from_task_locked(task);
}

void syn_sched_clear_mutex_wait(SYN_Task *task)
{
    if ((task->sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U) {
        task->sched_flags &= (uint8_t)~SYN_SCHED_FLAG_MUTEX_WAIT;
        sched_recompute_mutex_owners_locked(task->sched);
    }
}

void syn_sched_refresh_share(SYN_Sched *sched)
{
    sched_refresh_share_locked(sched);
}
/** @endcond */

void syn_task_boost_priority(SYN_Task *task, SYN_Prio temp_prio)
{
    SYN_ASSERT(task != NULL);
    if (task == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }
    syn_port_enter_critical();
    if ((uint32_t)temp_prio < (uint32_t)SYN_SCHED_PRIO_LEVELS &&
        (uint8_t)temp_prio < task->priority) {
        task->manual_priority = (uint8_t)temp_prio;
        sched_boost_from_task_locked(task);
    }
    syn_port_exit_critical();
}

void syn_task_restore_priority(SYN_Task *task)
{
    SYN_ASSERT(task != NULL);
    if (task == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }
    syn_port_enter_critical();
    task->manual_priority = SYN_TASK_NO_MANUAL_PRIO;
    sched_demote_from_task_locked(task);
    syn_port_exit_critical();
}

void syn_task_set_base_priority(SYN_Task *task, SYN_Prio new_prio)
{
    SYN_ASSERT(task != NULL);
    SYN_ASSERT((uint32_t)new_prio < (uint32_t)SYN_SCHED_PRIO_LEVELS);

    if (task == NULL || (uint32_t)new_prio >= (uint32_t)SYN_SCHED_PRIO_LEVELS) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }

    syn_port_enter_critical();
    uint8_t old_base = task->base_priority;
    task->base_priority = (uint8_t)new_prio;
    if ((uint8_t)new_prio < old_base) {
        sched_boost_from_task_locked(task);
    } else {
        sched_demote_from_task_locked(task);
    }
    syn_port_exit_critical();
}

#endif /* SYN_USE_SCHED */
