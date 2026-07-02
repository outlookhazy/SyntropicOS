

# File syn\_pt.h

[**File List**](files.md) **>** [**pt**](dir_a1b594c7ca8b98b889036d2814e4f947.md) **>** [**syn\_pt.h**](syn__pt_8h.md)

[Go to the documentation of this file](syn__pt_8h.md)


```C++

#ifndef SYN_PT_H
#define SYN_PT_H

#include "../common/syn_defs.h"
#include "../common/syn_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Protothread continuation ───────────────────────────────────────────── */

typedef struct {
    uint16_t lc;   
} SYN_PT;

/* ── Return status ──────────────────────────────────────────────────────── */

typedef enum {
    PT_WAITING = 0,  
    PT_YIELDED = 1,  
    PT_EXITED  = 2,  
    PT_ENDED   = 3,  
} SYN_PT_Status;

/* ── Core macros ────────────────────────────────────────────────────────── */

#define PT_INIT(pt)   ((pt)->lc = 0)

#define PT_BEGIN(pt)                                          \
    do {                                                       \
        char _pt_yield_flag = 1;                               \
        (void)_pt_yield_flag;                                  \
        switch ((pt)->lc) {                                    \
        case 0:

#define PT_END(pt)                                            \
        }                                                      \
        _pt_yield_flag = 0;                                    \
        PT_INIT(pt);                                           \
        return PT_EXITED;                                      \
    } while (0)

#define PT_WAIT_UNTIL(pt, cond)                               \
    do {                                                       \
        (pt)->lc = __LINE__;                                   \
        SYN_FALLTHROUGH; /* intentional Duff's device */      \
        case __LINE__:                                         \
        if (!(cond)) {                                         \
            return PT_WAITING;                                 \
        }                                                      \
    } while (0)

#define PT_WAIT_WHILE(pt, cond)   PT_WAIT_UNTIL(pt, !(cond))

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

#define PT_EXIT(pt)                                           \
    do {                                                       \
        PT_INIT(pt);                                           \
        return PT_ENDED;                                       \
    } while (0)

#define PT_RESTART(pt)                                        \
    do {                                                       \
        PT_INIT(pt);                                           \
        return PT_WAITING;                                     \
    } while (0)

/* ── Child / sub-protothread ────────────────────────────────────────────── */

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

#define PT_DELAY_MS(pt, target, ms)                           \
    do {                                                       \
        *(target) = syn_port_get_tick_ms() + (uint32_t)(ms);  \
        PT_WAIT_UNTIL(pt, syn_port_get_tick_ms() >= *(target)); \
    } while (0)

#define PT_TASK_DELAY_MS(pt, task, ms) \
    PT_DELAY_MS(pt, &(task)->delay_until, ms)

/* ── Query macros ───────────────────────────────────────────────────────── */

#define PT_IS_RUNNING(pt)     ((pt)->lc != 0)

#define PT_IS_IDLE(pt)        ((pt)->lc == 0)

#ifdef __cplusplus
}
#endif

#endif /* SYN_PT_H */
```


