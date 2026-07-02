

# File syn\_rate\_limit.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_rate\_limit.h**](syn__rate__limit_8h.md)

[Go to the documentation of this file](syn__rate__limit_8h.md)


```C++

#ifndef SYN_RATE_LIMIT_H
#define SYN_RATE_LIMIT_H

#include "../port/syn_port_system.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t  tokens;       
    uint32_t  max_tokens;   
    uint32_t  interval_ms;  
    uint32_t  last_refill;  
} SYN_RateLimit;

static inline void syn_rate_limit_init(SYN_RateLimit *rl,
                                        uint32_t max_tokens,
                                        uint32_t interval_ms)
{
    rl->tokens      = max_tokens;
    rl->max_tokens  = max_tokens;
    rl->interval_ms = interval_ms;
    rl->last_refill = syn_port_get_tick_ms();
}

static inline bool syn_rate_limit_allow(SYN_RateLimit *rl)
{
    uint32_t now = syn_port_get_tick_ms();
    uint32_t elapsed = now - rl->last_refill;

    /* Refill tokens based on elapsed time */
    if (elapsed >= rl->interval_ms) {
        uint32_t refills = elapsed / rl->interval_ms;
        rl->tokens = rl->max_tokens; /* full refill */
        rl->last_refill += refills * rl->interval_ms;
    } else if (rl->interval_ms > 0) {
        /* Partial refill: proportional token recovery */
        uint32_t new_tokens = (rl->max_tokens * elapsed) / rl->interval_ms;
        if (new_tokens > 0) {
            rl->tokens += new_tokens;
            if (rl->tokens > rl->max_tokens) rl->tokens = rl->max_tokens;
            rl->last_refill = now;
        }
    }

    if (rl->tokens > 0) {
        rl->tokens--;
        return true;
    }
    return false;
}

static inline uint32_t syn_rate_limit_remaining(const SYN_RateLimit *rl)
{
    return rl->tokens;
}

static inline void syn_rate_limit_reset(SYN_RateLimit *rl)
{
    rl->tokens      = rl->max_tokens;
    rl->last_refill = syn_port_get_tick_ms();
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_RATE_LIMIT_H */
```


