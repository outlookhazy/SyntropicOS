

# File syn\_pt.h

[**File List**](files.md) **>** [**pt**](dir_a1b594c7ca8b98b889036d2814e4f947.md) **>** [**syn\_pt.h**](syn__pt_8h.md)

[Go to the documentation of this file](syn__pt_8h.md)


```C++


#ifndef SYN_PT_H
#define SYN_PT_H

#include "../common/syn_barrier.h"
#include "../common/syn_compiler.h"
#include "../common/syn_defs.h"
#include "../port/syn_port_system.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
#define SYN_PT_STATE_BARRIER()     \
    do {                           \
        SYN_COMPILER_BARRIER();    \
        syn_port_memory_barrier(); \
    } while (0)
#else
#define SYN_PT_STATE_BARRIER() SYN_COMPILER_BARRIER()
#endif

/* ── Protothread continuation ───────────────────────────────────────────── */

typedef struct {
    uint16_t lc; 
} SYN_PT;

/* ── Return status ──────────────────────────────────────────────────────── */

typedef enum {
    PT_IDLE = 0,   
    PT_MORE = 1,   
    PT_EXITED = 2, 
    PT_ENDED = 3,  
} SYN_PT_Status;

/* ── Core macros ────────────────────────────────────────────────────────── */

#define PT_INIT(pt) ((pt)->lc = 0)

#define PT_BEGIN(pt)             \
    do {                         \
        char _pt_yield_flag = 1; \
        (void)_pt_yield_flag;    \
        switch ((pt)->lc) {      \
        case 0:

#define PT_END(pt)      \
    }                   \
    _pt_yield_flag = 0; \
    PT_INIT(pt);        \
    return PT_ENDED;    \
    }                   \
    while (0)

#define PT_WAIT_UNTIL(pt, cond)                          \
    do {                                                 \
        (pt)->lc = __LINE__;                             \
        SYN_FALLTHROUGH; /* intentional Duff's device */ \
    case __LINE__:                                       \
        if (!(cond)) {                                   \
            return PT_IDLE;                              \
        }                                                \
    } while (0)

#define PT_WAIT_WHILE(pt, cond) PT_WAIT_UNTIL(pt, !(cond))

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

#define PT_EXIT(pt)       \
    do {                  \
        PT_INIT(pt);      \
        return PT_EXITED; \
    } while (0)

#define PT_RESTART(pt)  \
    do {                \
        PT_INIT(pt);    \
        return PT_IDLE; \
    } while (0)

/* ── Child / sub-protothread ────────────────────────────────────────────── */

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

static inline bool syn_pt_arm_us_deadline(uint32_t *target, uint32_t now, uint32_t us)
{
    if (us == 0U) {
        return false;
    }
    uint32_t clamped = (us > 0x7FFFFFFFU) ? 0x7FFFFFFFU : us;
    *target = now + clamped;
    return true;
}

#define PT_DELAY_MS(pt, target, ms)                                                     \
    do {                                                                                \
        if (syn_pt_arm_ms_deadline((target), syn_port_get_tick_ms(), (uint32_t)(ms))) { \
            PT_WAIT_UNTIL(pt, (int32_t)(syn_port_get_tick_ms() - *(target)) >= 0);      \
            *(target) = 0U; /* consume — stale deadlines wrap to "future" */            \
        }                                                                               \
    } while (0)

#define PT_TASK_DELAY_MS(pt, task, ms) PT_DELAY_MS(pt, &(task)->delay_until, ms)

#define PT_DELAY_US(pt, target, us)                                                     \
    do {                                                                                \
        if (syn_pt_arm_us_deadline((target), syn_port_get_tick_us(), (uint32_t)(us))) { \
            PT_WAIT_UNTIL(pt, (int32_t)(syn_port_get_tick_us() - *(target)) >= 0);      \
        }                                                                               \
    } while (0)

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

#define PT_IS_RUNNING(pt) ((pt)->lc != 0)

#define PT_IS_IDLE(pt) ((pt)->lc == 0)

#ifdef __cplusplus
}
#endif

#endif /* SYN_PT_H */
```


