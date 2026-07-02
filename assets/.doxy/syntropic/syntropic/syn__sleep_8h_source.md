

# File syn\_sleep.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_sleep.h**](syn__sleep_8h.md)

[Go to the documentation of this file](syn__sleep_8h.md)


```C++

#ifndef SYN_SLEEP_H
#define SYN_SLEEP_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Sleep depth ────────────────────────────────────────────────────────── */

typedef enum {
    SYN_SLEEP_NONE   = 0,  
    SYN_SLEEP_LIGHT  = 1,  
    SYN_SLEEP_DEEP   = 2,  
} SYN_SleepMode;

#define SYN_SLEEP_LOCK_UART     (1u << 0)  
#define SYN_SLEEP_LOCK_SPI      (1u << 1)  
#define SYN_SLEEP_LOCK_I2C      (1u << 2)  
#define SYN_SLEEP_LOCK_DMA      (1u << 3)  
#define SYN_SLEEP_LOCK_ADC      (1u << 4)  
#define SYN_SLEEP_LOCK_TIMER    (1u << 5)  
#define SYN_SLEEP_LOCK_APP0     (1u << 8)  
#define SYN_SLEEP_LOCK_APP1     (1u << 9)  
#define SYN_SLEEP_LOCK_APP2     (1u << 10) 
#define SYN_SLEEP_LOCK_APP3     (1u << 11) 
/* ── Sleep coordinator ──────────────────────────────────────────────────── */

typedef struct {
    SYN_SleepMode   max_depth;       
    volatile uint32_t lock_mask;      
    uint32_t          enter_count;    
    uint32_t          veto_count;     
    bool              enabled;        
} SYN_Sleep;

/* ── Port function (user implements) ────────────────────────────────────── */

extern void syn_port_sleep(SYN_SleepMode mode);

/* ── API ────────────────────────────────────────────────────────────────── */

static inline void syn_sleep_init(SYN_Sleep *s, SYN_SleepMode max_depth)
{
    s->max_depth   = max_depth;
    s->lock_mask   = 0;
    s->enter_count = 0;
    s->veto_count  = 0;
    s->enabled     = true;
}

static inline void syn_sleep_lock(SYN_Sleep *s, uint32_t lock_id)
{
    s->lock_mask |= lock_id;
}

static inline void syn_sleep_unlock(SYN_Sleep *s, uint32_t lock_id)
{
    s->lock_mask &= ~lock_id;
}

static inline bool syn_sleep_is_locked(const SYN_Sleep *s, uint32_t lock_id)
{
    return (s->lock_mask & lock_id) != 0;
}

static inline bool syn_sleep_any_locked(const SYN_Sleep *s)
{
    return s->lock_mask != 0;
}

static inline bool syn_sleep_enter(SYN_Sleep *s)
{
    if (!s->enabled || s->lock_mask != 0) {
        s->veto_count++;
        return false;
    }

    s->enter_count++;
    syn_port_sleep(s->max_depth);
    return true;
}

static inline void syn_sleep_enable(SYN_Sleep *s, bool en)
{
    s->enabled = en;
}

static inline uint32_t syn_sleep_locks(const SYN_Sleep *s)
{
    return s->lock_mask;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_SLEEP_H */
```


