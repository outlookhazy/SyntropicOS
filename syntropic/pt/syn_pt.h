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
 * @ingroup syn_sched
 */

#ifndef SYN_PT_H
#define SYN_PT_H

#include "../common/syn_defs.h"
#include "../common/syn_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Protothread continuation ───────────────────────────────────────────── */

/**
 * @brief Protothread control block.
 *
 * Stores the line-continuation used to resume the coroutine. Costs
 * exactly 2 bytes of RAM.
 */
typedef struct {
    uint16_t lc;   /**< Line continuation — stores __LINE__ at last yield */
} SYN_PT;

/* ── Return status ──────────────────────────────────────────────────────── */

/**
 * @brief Return value from a protothread function.
 */
typedef enum {
    PT_WAITING = 0,  /**< Thread is blocked (condition not yet met)  */
    PT_YIELDED = 1,  /**< Thread voluntarily yielded — call again    */
    PT_EXITED  = 2,  /**< Thread ran to PT_END — normal completion   */
    PT_ENDED   = 3,  /**< Thread was explicitly ended via PT_EXIT    */
} SYN_PT_Status;

/* ── Core macros ────────────────────────────────────────────────────────── */

/**
 * @brief Initialize (or reset) a protothread so it starts from the top.
 */
#define PT_INIT(pt)   ((pt)->lc = 0)

/**
 * @brief Open a protothread body. Must be the first statement in the
 *        protothread function, before any PT_* operations.
 *
 * Expands to the head of a switch statement that jumps to the last
 * saved continuation point.
 */
#define PT_BEGIN(pt)                                          \
    do {                                                       \
        char _pt_yield_flag = 1;                               \
        (void)_pt_yield_flag;                                  \
        switch ((pt)->lc) {                                    \
        case 0:

/**
 * @brief Close a protothread body. Returns PT_EXITED and resets the
 *        continuation so the thread can be restarted if desired.
 */
#define PT_END(pt)                                            \
        }                                                      \
        _pt_yield_flag = 0;                                    \
        PT_INIT(pt);                                           \
        return PT_EXITED;                                      \
    } while (0)

/**
 * @brief Block until @p cond evaluates to true.
 *
 * Each time the protothread is polled and @p cond is false, it returns
 * PT_WAITING and resumes at this point on the next call.
 */
#define PT_WAIT_UNTIL(pt, cond)                               \
    do {                                                       \
        (pt)->lc = __LINE__;                                   \
        SYN_FALLTHROUGH; /* intentional Duff's device */      \
        case __LINE__:                                         \
        if (!(cond)) {                                         \
            return PT_WAITING;                                 \
        }                                                      \
    } while (0)

/**
 * @brief Block while @p cond is true. Dual of PT_WAIT_UNTIL.
 */
#define PT_WAIT_WHILE(pt, cond)   PT_WAIT_UNTIL(pt, !(cond))

/**
 * @brief Yield control unconditionally. The protothread will resume at
 *        this point on the next call.
 */
#define PT_YIELD(pt)                                          \
    do {                                                       \
        _pt_yield_flag = 0;                                    \
        (pt)->lc = __LINE__;                                   \
        SYN_FALLTHROUGH;                                      \
        case __LINE__:                                         \
        if (_pt_yield_flag == 0) {                             \
            return PT_YIELDED;                                 \
        }                                                      \
    } while (0)

/**
 * @brief Yield while @p cond is true.
 *
 * Like PT_WAIT_WHILE, but returns PT_YIELDED instead of PT_WAITING,
 * signaling the scheduler that this thread actively wants to run.
 */
#define PT_YIELD_UNTIL(pt, cond)                              \
    do {                                                       \
        _pt_yield_flag = 0;                                    \
        (pt)->lc = __LINE__;                                   \
        SYN_FALLTHROUGH;                                      \
        case __LINE__:                                         \
        if ((_pt_yield_flag == 0) || !(cond)) {                \
            return PT_YIELDED;                                 \
        }                                                      \
    } while (0)

/**
 * @brief Terminate the protothread immediately.
 */
#define PT_EXIT(pt)                                           \
    do {                                                       \
        PT_INIT(pt);                                           \
        return PT_ENDED;                                       \
    } while (0)

/**
 * @brief Reset the protothread and restart from PT_BEGIN on next call.
 */
#define PT_RESTART(pt)                                        \
    do {                                                       \
        PT_INIT(pt);                                           \
        return PT_WAITING;                                     \
    } while (0)

/* ── Child / sub-protothread ────────────────────────────────────────────── */

/**
 * @brief Spawn a child protothread and block until it exits.
 *
 * @param pt      Parent protothread.
 * @param child   Child protothread (SYN_PT struct).
 * @param func    Expression that calls the child's function, e.g.
 *                `child_func(&child_pt)`.
 *
 * Usage:
 * @code
 *   static SYN_PT child_pt;
 *   PT_SPAWN(pt, &child_pt, child_func(&child_pt));
 * @endcode
 */
#define PT_SPAWN(pt, child, func)                             \
    do {                                                       \
        PT_INIT(child);                                        \
        (pt)->lc = __LINE__;                                   \
        SYN_FALLTHROUGH;                                      \
        case __LINE__:                                         \
        if ((func) < PT_EXITED) {                              \
            return PT_WAITING;                                 \
        }                                                      \
    } while (0)

/* ── Timer-aware delay (requires task descriptor) ───────────────────────── */

/*
 * PT_DELAY_MS needs access to both the tick source and a place to store
 * the delay target. When used with the scheduler (syn_task.h), the
 * SYN_Task struct provides `delay_until`. For standalone use, provide
 * your own uint32_t target variable.
 */

/**
 * @brief Non-blocking delay for @p ms milliseconds.
 *
 * Requires a `uint32_t` variable to store the deadline. When used with
 * the scheduler, pass `&(task)->delay_until`. For standalone use, pass
 * any persistent `uint32_t *`.
 *
 * @param pt       Protothread.
 * @param target   Pointer to a uint32_t that will hold the deadline tick.
 * @param ms       Delay duration in milliseconds.
 */
#define PT_DELAY_MS(pt, target, ms)                           \
    do {                                                       \
        *(target) = syn_port_get_tick_ms() + (uint32_t)(ms);  \
        PT_WAIT_UNTIL(pt, syn_port_get_tick_ms() >= *(target)); \
    } while (0)

/**
 * @brief Convenience form for use with the scheduler's SYN_Task.
 *
 * @param pt    Protothread.
 * @param task  Pointer to the SYN_Task struct.
 * @param ms    Delay duration in milliseconds.
 */
#define PT_TASK_DELAY_MS(pt, task, ms) \
    PT_DELAY_MS(pt, &(task)->delay_until, ms)

/* ── Query macros ───────────────────────────────────────────────────────── */

/** Check if a protothread is still running (has not exited). */
#define PT_IS_RUNNING(pt)     ((pt)->lc != 0)

/** Check if a protothread has not yet started or has been reset. */
#define PT_IS_IDLE(pt)        ((pt)->lc == 0)

#ifdef __cplusplus
}
#endif

#endif /* SYN_PT_H */
