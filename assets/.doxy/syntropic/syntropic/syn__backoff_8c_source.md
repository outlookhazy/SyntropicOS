

# File syn\_backoff.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_backoff.c**](syn__backoff_8c.md)

[Go to the documentation of this file](syn__backoff_8c.md)


```C++


#include "syn_backoff.h"

#include "syn_random.h"

#include <string.h>

void syn_backoff_init(SYN_Backoff *b, uint32_t min_ms, uint32_t max_ms, uint8_t factor,
                      uint8_t max_attempts)
{
    if (b == NULL) {
        return;
    }
    if (min_ms > max_ms) {
        min_ms = max_ms;
    }
    (void)memset(b, 0, sizeof(*b));
    b->min_ms = min_ms;
    b->max_ms = max_ms;
    b->max_attempts = max_attempts;
    b->factor = (factor < 1U) ? 1U : factor;
    syn_backoff_reset(b);
}

void syn_backoff_reset(SYN_Backoff *b)
{
    if (b == NULL) {
        return;
    }
    b->current_ms = b->min_ms;
    b->attempts = 0U;
}

uint32_t syn_backoff_next_ms(SYN_Backoff *b)
{
    if (b == NULL) {
        return 0U;
    }

    /* 1. Calculate delay for THIS attempt (including jitter) */
    uint32_t base = b->current_ms;
    uint32_t jitter_max = base / 2U;
    uint64_t delay64 = (uint64_t)base;
    if (jitter_max > 0U) {
        delay64 += (uint64_t)syn_random_range(0U, jitter_max);
    }
    uint32_t delay = (delay64 > (uint64_t)UINT32_MAX) ? UINT32_MAX : (uint32_t)delay64;

    /* 2. Update state for NEXT attempt */
    uint64_t next = (uint64_t)b->current_ms * b->factor;
    if (next > b->max_ms) {
        b->current_ms = b->max_ms;
    } else {
        b->current_ms = (uint32_t)next;
    }
    if (b->attempts < UINT8_MAX) {
        b->attempts++;
    }

    return delay;
}

bool syn_backoff_exhausted(const SYN_Backoff *b)
{
    if (b == NULL) {
        return true;
    }
    return b->attempts >= b->max_attempts;
}
```


