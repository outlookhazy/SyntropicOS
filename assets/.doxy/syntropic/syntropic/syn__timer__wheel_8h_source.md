

# File syn\_timer\_wheel.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_timer\_wheel.h**](syn__timer__wheel_8h.md)

[Go to the documentation of this file](syn__timer__wheel_8h.md)


```C++

#ifndef SYN_TIMER_WHEEL_H
#define SYN_TIMER_WHEEL_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_TIMER_WHEEL_BUCKETS 64U

typedef void (*SYN_TimerWheelCb)(void *arg);

typedef struct SYN_TimerWheelNode {
    struct SYN_TimerWheelNode *next; 
    struct SYN_TimerWheelNode *prev; 
    uint32_t         expires_tick;   
    uint32_t         rotation_count; 
    SYN_TimerWheelCb cb;             
    void            *arg;            
    bool             active;         
} SYN_TimerWheelNode;

typedef struct {
    SYN_TimerWheelNode *buckets[SYN_TIMER_WHEEL_BUCKETS]; 
    uint32_t current_tick;                                 
} SYN_TimerWheel;

SYN_Status syn_timer_wheel_init(SYN_TimerWheel *wheel);

SYN_Status syn_timer_wheel_add(SYN_TimerWheel *wheel, SYN_TimerWheelNode *node, uint32_t delay_ticks, SYN_TimerWheelCb cb, void *arg);

SYN_Status syn_timer_wheel_cancel(SYN_TimerWheel *wheel, SYN_TimerWheelNode *node);

size_t syn_timer_wheel_step(SYN_TimerWheel *wheel);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TIMER_WHEEL_H */
```


