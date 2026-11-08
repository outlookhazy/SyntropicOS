/**
 * @file syn_pt.h
 * @brief Protothreads — stackless cooperative coroutines for C.
 *
 * Protothreads provide lightweight, cooperative multitasking without
 * requiring a separate stack per thread. Each protothread is a normal C
 * function that uses macros to save and restore its execution position
 * via the switch/__LINE__ continuation trick (Duff's device).
 *
 * A protothread costs **2 bytes of RAM** (the continuation variable).
 *
 * @par Constraints
 * - You cannot use a bare `switch` statement inside a protothread body
 *   (use `if`/`else if` chains instead).
 * - Local variables are NOT preserved across yield/wait points. Store
 *   persistent state in `static` variables, globals, or a struct passed
 *   via the task's `user_data` pointer.
 * - A protothread must reside entirely within a single function.
 *
 * @par Standalone usage (no scheduler)
 * @code
 *   static SYN_PT pt;
 *   PT_INIT(&pt);
 *
 *   while (1) {
 *       my_protothread(&pt);
 *       // ... poll other protothreads ...
 *   }
 * @endcode
 *
 * @par With the scheduler
 * @code
 *   // See syn_sched.h for the full pattern.
 *   static SYN_PT_Status my_task(SYN_PT *pt, SYN_Task *task) {
 *       PT_BEGIN(pt);
 *       // ...
 *       PT_END(pt);
 *   }
 * @endcode
 *
 * @par Macro expansion (Duff's device continuation)
 * @code
 *   // High-level C code
 *   SYN_PT_Status my_task(SYN_PT *pt) {
 *       PT_BEGIN(pt);
 *       PT_WAIT_UNTIL(pt, flag == 1);
 *       PT_END(pt);
 *   }
 *
 *   // Expands preprocessor output to:
 *   SYN_PT_Status my_task(SYN_PT *pt) {
 *       char _pt_yield_flag = 1;
 *       switch (pt->lc) {
 *       case 0:
 *           // On resume, switch(pt->lc) jumps directly to case 12:
 *           pt->lc = 12; case 12:
 *           if (!(flag == 1)) return PT_IDLE;
 *       }
 *       _pt_yield_flag = 0;
 *       PT_INIT(pt);
 *       return PT_ENDED;
 *   }
 * @endcode
 * @ingroup syn_sched
 */

#ifndef SYN_PT_H
#define SYN_PT_H

#include "../common/syn_barrier.h"
#include "../common/syn_compiler.h"
#include "../common/syn_defs.h"
#include "../port/syn_port_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @cond INTERNAL */
#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
#define SYN_PT_STATE_BARRIER()     \
    do {                           \
        SYN_COMPILER_BARRIER();    \
        syn_port_memory_barrier(); \
    } while (0)
#else
#define SYN_PT_STATE_BARRIER() SYN_COMPILER_BARRIER()
#endif
/** @endcond */

/* ── Protothread continuation ───────────────────────────────────────────── */

/**
 * @brief Protothread control block.
 *
 * Stores the line-continuation used to resume the coroutine. Costs
 * exactly 2 bytes of RAM.
 */
typedef struct {
    uint16_t lc; /**< Line continuation — stores __LINE__ at last yield */
} SYN_PT;

/* ── Return status ──────────────────────────────────────────────────────── */

/**
 * @brief Return value from a protothread function.
 */
typedef enum {
    PT_IDLE = 0,   /**< Nothing to do right now — let others run this round */
    PT_MORE = 1,   /**< Completed a chunk of work and more work is waiting  */
    PT_EXITED = 2, /**< Thread aborted early via PT_EXIT                    */
    PT_ENDED = 3,  /**< Thread ran to PT_END — normal completion            */
} SYN_PT_Status;

/* ── Core macros ────────────────────────────────────────────────────────── */

/**
 * @brief Initialize (or reset) a protothread so it starts from the top.
 */
#define PT_INIT(pt) ((pt)->lc = 0)

/**
 * @brief Open a protothread body. Must be the first statement in the
 *        protothread function, before any PT_* operations.
 *
 * Expands to the head of a switch statement that jumps to the last
 * saved continuation point.
 */
#define PT_BEGIN(pt)             \
    do {                         \
        char _pt_yield_flag = 1; \
        (void)_pt_yield_flag;    \
        switch ((pt)->lc) {      \
        case 0:

/**
 * @brief Close a protothread body. Returns PT_ENDED and resets the
 *        continuation so the thread can be restarted if desired.
 */
#define PT_END(pt)      \
    }                   \
    _pt_yield_flag = 0; \
    PT_INIT(pt);        \
    return PT_ENDED;    \
    }                   \
    while (0)

/**
 * @brief Block until @p cond evaluates to true.
 *
 * Each time the protothread is polled and @p cond is false, it returns
 * PT_IDLE and resumes at this point on the next call.
 */
#define PT_WAIT_UNTIL(pt, cond)                          \
    do {                                                 \
        (pt)->lc = __LINE__;                             \
        SYN_FALLTHROUGH; /* intentional Duff's device */ \
    case __LINE__:                                       \
        if (!(cond)) {                                   \
            return PT_IDLE;                              \
        }                                                \
    } while (0)

/**
 * @brief Block while @p cond is true. Dual of PT_WAIT_UNTIL.
 */
#define PT_WAIT_WHILE(pt, cond) PT_WAIT_UNTIL(pt, !(cond))

/**
 * @brief Pass the remainder of this scheduler round with PT_IDLE.
 *
 * Signals that the protothread has no immediate work right now so other
 * ready tasks (including lower priorities) can run this round.
 */
#define PT_PASS(pt)                \
    do {                           \
        _pt_yield_flag = 0;        \
        (pt)->lc = __LINE__;       \
        SYN_FALLTHROUGH;           \
    case __LINE__:                 \
        if (_pt_yield_flag == 0) { \
            return PT_IDLE;        \
        }                          \
    } while (0)

/**
 * @brief Yield after completing a work chunk with PT_MORE.
 *
 * Signals that the protothread actively has more work remaining and should
 * remain in the scheduler's Work tier.
 */
#define PT_CONTINUE(pt)            \
    do {                           \
        _pt_yield_flag = 0;        \
        (pt)->lc = __LINE__;       \
        SYN_FALLTHROUGH;           \
    case __LINE__:                 \
        if (_pt_yield_flag == 0) { \
            return PT_MORE;        \
        }                          \
    } while (0)

/**
 * @brief Abort the protothread immediately and return PT_EXITED.
 */
#define PT_EXIT(pt)       \
    do {                  \
        PT_INIT(pt);      \
        return PT_EXITED; \
    } while (0)

/**
 * @brief Reset the protothread and restart from PT_BEGIN on next call.
 */
#define PT_RESTART(pt)  \
    do {                \
        PT_INIT(pt);    \
        return PT_IDLE; \
    } while (0)

/* ── Child / sub-protothread ────────────────────────────────────────────── */

/**
 * @brief Spawn a child protothread and block until it exits.
 *
 * Propagates the child's PT_IDLE or PT_MORE status verbatim while running.
 *
 * @param pt      Parent protothread.
 * @param child   Child protothread (SYN_PT struct).
 * @param func    Expression that calls the child's function, e.g.
 *                `child_func(&child_pt)`.
 *
 * @warning The @p func expression is evaluated on EVERY scheduler poll
 * until the child exits. Never pass expressions with side-effects
 * (e.g. counter++, function calls that modify state). Pre-evaluate
 * arguments before calling PT_SPAWN.
 *
 * Usage:
 * @code
 *   static SYN_PT child_pt;
 *   PT_SPAWN(pt, &child_pt, child_func(&child_pt));
 * @endcode
 */
#define PT_SPAWN(pt, child, func)            \
    do {                                     \
        PT_INIT(child);                      \
        (pt)->lc = __LINE__;                 \
        SYN_FALLTHROUGH;                     \
    case __LINE__: {                         \
        SYN_PT_Status _pt_child_st = (func); \
        if (_pt_child_st < PT_EXITED) {      \
            return _pt_child_st;             \
        }                                    \
    }                                        \
    } while (0)

/* ── Timer-aware delay (requires task descriptor) ───────────────────────── */

/*
 * PT_DELAY_MS needs access to both the tick source and a place to store
 * the delay target. When used with the scheduler (syn_task.h), the
 * SYN_Task struct provides `delay_until`. For standalone use, provide
 * your own uint32_t target variable.
 */

/**
 * @brief Internal helper: single-evaluation, INT32_MAX-clamped millisecond deadline arming.
 * @param target Pointer to deadline storage variable.
 * @param now    Current millisecond tick counter.
 * @param ms     Requested delay in milliseconds.
 * @return true if a non-zero deadline was armed, false if @p ms is 0.
 */
static inline bool syn_pt_arm_ms_deadline(uint32_t *target, uint32_t now, uint32_t ms)
{
    if (ms == 0U) {
        return false;
    }
    uint32_t clamped = (ms > 0x7FFFFFFFU) ? 0x7FFFFFFFU : ms;
    uint32_t dl = now + clamped;
    if (dl == 0U) {
        /* Avoid 0 sentinel; nudge backward at INT32_MAX so (dl - now) stays positive */
        dl = (clamped == 0x7FFFFFFFU) ? 0xFFFFFFFFU : 1U;
    }
    *target = dl;
    return true;
}

/**
 * @brief Internal helper: single-evaluation, INT32_MAX-clamped microsecond deadline arming.
 * @param target Pointer to deadline storage variable.
 * @param now    Current microsecond tick counter.
 * @param us     Requested delay in microseconds.
 * @return true if a non-zero deadline was armed, false if @p us is 0.
 */
static inline bool syn_pt_arm_us_deadline(uint32_t *target, uint32_t now, uint32_t us)
{
    if (us == 0U) {
        return false;
    }
    uint32_t clamped = (us > 0x7FFFFFFFU) ? 0x7FFFFFFFU : us;
    *target = now + clamped;
    return true;
}

/**
 * @brief Non-blocking delay for @p ms milliseconds.
 *
 * Requires a `uint32_t` variable to store the deadline. When used with
 * the scheduler, pass `&(task)->delay_until`. For standalone use, pass
 * any persistent `uint32_t *`.
 *
 * @note The target is reset to 0 (the scheduler's "no deadline pending"
 *       sentinel) once the delay completes. A deadline left in place
 *       would read as "in the future" again after the tick counter
 *       advances 2^31 ms (~24.8 days) past it, freezing the task. A
 *       computed deadline of exactly 0 (tick wrap) is nudged to 1 (or
 *       0xFFFFFFFF when clamped == INT32_MAX) so it cannot alias the
 *       sentinel while keeping `(int32_t)(dl - now) > 0`. Durations
 *       exceeding INT32_MAX ms are clamped to INT32_MAX ms, and @p ms
 *       is evaluated exactly once.
 *
 * @param pt       Protothread.
 * @param target   Pointer to a uint32_t that will hold the deadline tick.
 * @param ms       Delay duration in milliseconds.
 */
#define PT_DELAY_MS(pt, target, ms)                                                     \
    do {                                                                                \
        if (syn_pt_arm_ms_deadline((target), syn_port_get_tick_ms(), (uint32_t)(ms))) { \
            PT_WAIT_UNTIL(pt, (int32_t)(syn_port_get_tick_ms() - *(target)) >= 0);      \
            *(target) = 0U; /* consume — stale deadlines wrap to "future" */            \
        }                                                                               \
    } while (0)

/**
 * @brief Convenience form for use with the scheduler's SYN_Task (milliseconds).
 *
 * @param pt    Protothread.
 * @param task  Pointer to the SYN_Task struct.
 * @param ms    Delay duration in milliseconds.
 */
#define PT_TASK_DELAY_MS(pt, task, ms) PT_DELAY_MS(pt, &(task)->delay_until, ms)

/**
 * @brief Non-blocking sub-millisecond delay for @p us microseconds.
 *
 * Requires a user-supplied persistent `uint32_t` variable (e.g., `static uint32_t us_target;`).
 * Does not modify `SYN_Task.delay_until` to avoid corrupting the millisecond scheduler timebase.
 *
 * @param pt       Protothread.
 * @param target   Pointer to a uint32_t variable holding the microsecond deadline.
 * @param us       Delay duration in microseconds.
 */
#define PT_DELAY_US(pt, target, us)                                                     \
    do {                                                                                \
        if (syn_pt_arm_us_deadline((target), syn_port_get_tick_us(), (uint32_t)(us))) { \
            PT_WAIT_UNTIL(pt, (int32_t)(syn_port_get_tick_us() - *(target)) >= 0);      \
        }                                                                               \
    } while (0)

/**
 * @brief Block until ANY bit in @p mask is set in the event group.
 *
 * Unlike PT_WAIT_EVENT (which polls every scheduler pass), this macro
 * sets the task state to BLOCKED.  The scheduler skips the task
 * entirely until the event fires, saving CPU and enabling proper
 * tickless sleep.
 *
 * Matched flags are atomically captured and auto-cleared when the scheduler
 * dispatches the woken task.
 *
 * @param pt    Protothread.
 * @param task  Pointer to the SYN_Task struct.
 * @param grp   Pointer to SYN_EventGroup.
 * @param mask  Bitmask of event flags to wait for (any of them).
 */
#define PT_BLOCK_EVENT(pt, task, grp, mask)                                                      \
    do {                                                                                         \
        if (syn_task_arm_event_wait((task), (SYN_EventFlags *)(grp), (mask), false, 0U, NULL)) { \
            while ((task)->wait_event != NULL) {                                                 \
                syn_task_park_blocked((task));                                                   \
                PT_PASS(pt);                                                                     \
            }                                                                                    \
            (task)->wait_mask = 0U;                                                              \
        }                                                                                        \
    } while (0)

/**
 * @brief Block task execution (SYN_TASK_BLOCKED) until ANY bit in @p mask is set
 * OR @p timeout_ms milliseconds elapse.
 *
 * Combines event-flag blocking with millisecond deadline tracking. The task is
 * skipped by the scheduler until either the event fires or the timeout deadline passes.
 *
 * Matched event flags are captured into @p out_flags (if non-NULL) and auto-cleared ONLY
 * if the event actually fired before the timeout. Check `*out_flags != 0` to determine if
 * the event fired vs timed out.
 *
 * @param pt          Protothread.
 * @param task        Pointer to the SYN_Task struct.
 * @param grp         Pointer to SYN_EventFlags.
 * @param mask        Bitmask of event flags to wait for.
 * @param timeout_ms  Timeout duration in milliseconds.
 * @param out_flags   Pointer to uint32_t receiving matched flags (or 0 on timeout).
 */
#define PT_BLOCK_EVENT_WITH_TIMEOUT(pt, task, grp, mask, timeout_ms, out_flags)    \
    do {                                                                           \
        uint32_t *_syn_ef_out = (out_flags);                                       \
        if (syn_task_arm_event_wait((task), (SYN_EventFlags *)(grp), (mask), true, \
                                    (uint32_t)(timeout_ms), _syn_ef_out)) {        \
            while ((task)->wait_event != NULL) {                                   \
                syn_task_park_blocked((task));                                     \
                PT_PASS(pt);                                                       \
            }                                                                      \
            (task)->delay_until = 0U;                                              \
            {                                                                      \
                uint32_t *_syn_ef_post = (out_flags);                              \
                if (_syn_ef_post != NULL) {                                        \
                    *_syn_ef_post = (task)->wait_mask;                             \
                }                                                                  \
            }                                                                      \
            (task)->wait_mask = 0U;                                                \
        }                                                                          \
    } while (0)

/**
 * @brief Sleep task (SYN_TASK_BLOCKED) until explicitly resumed.
 *
 * @warning This macro does NOT poll @p cond on each scheduler tick.
 * Once the task yields as BLOCKED, the scheduler skips it entirely.
 * The condition is only re-evaluated when the task is explicitly
 * woken by an ISR or driver calling syn_task_resume(task).
 *
 * For polled condition blocking, use PT_WAIT_UNTIL(pt, cond) instead.
 * For event-flag-driven blocking, use PT_BLOCK_EVENT(pt, task, grp, mask).
 *
 * @param pt    Protothread.
 * @param task  Pointer to the SYN_Task struct.
 * @param cond  Boolean condition expression to evaluate.
 */
#define PT_BLOCK_CONDITION(pt, task, cond)                        \
    do {                                                          \
        while (1) {                                               \
            syn_port_enter_critical();                            \
            if ((task)->state == (uint8_t)SYN_TASK_READY) {       \
                (task)->state = (uint8_t)SYN_TASK_BLOCKED;        \
                SYN_PT_STATE_BARRIER();                           \
            }                                                     \
            syn_port_exit_critical();                             \
            if (cond) {                                           \
                syn_port_enter_critical();                        \
                if ((task)->state == (uint8_t)SYN_TASK_BLOCKED) { \
                    (task)->state = (uint8_t)SYN_TASK_READY;      \
                }                                                 \
                syn_port_exit_critical();                         \
                break;                                            \
            }                                                     \
            PT_PASS(pt);                                          \
        }                                                         \
    } while (0)

/* ── Query macros ───────────────────────────────────────────────────────── */

/** Check if a protothread is still running (has not exited). */
#define PT_IS_RUNNING(pt) ((pt)->lc != 0)

/** Check if a protothread has not yet started or has been reset. */
#define PT_IS_IDLE(pt) ((pt)->lc == 0)

#ifdef __cplusplus
}
#endif

#endif /* SYN_PT_H */
