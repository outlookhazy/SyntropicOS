

# File syn\_backoff.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_backoff.c**](syn__backoff_8c.md)

[Go to the documentation of this file](syn__backoff_8c.md)


```C++

#include "syn_backoff.h"
#include "syn_random.h"
#include "syn_assert.h"

void syn_backoff_init(SYN_Backoff *b, uint32_t min_ms, uint32_t max_ms, uint8_t factor, uint8_t max_attempts)
{
    SYN_ASSERT(b != NULL);
    b->min_ms       = min_ms;
    b->max_ms       = max_ms;
    b->max_attempts = max_attempts;
    b->factor       = (factor < 1) ? 1 : factor;
    syn_backoff_reset(b);
}

void syn_backoff_reset(SYN_Backoff *b)
{
    SYN_ASSERT(b != NULL);
    b->current_ms = b->min_ms;
    b->attempts   = 0;
}

uint32_t syn_backoff_next_ms(SYN_Backoff *b)
{
    SYN_ASSERT(b != NULL);

    /* 1. Calculate delay for THIS attempt (including jitter) */
    uint32_t base = b->current_ms;
    uint32_t jitter_max = base / 2;
    uint32_t delay = base;
    if (jitter_max > 0) {
        delay += syn_random_range(0, jitter_max);
    }

    /* 2. Update state for NEXT attempt */
    uint64_t next = (uint64_t)b->current_ms * b->factor;
    if (next > b->max_ms) {
        b->current_ms = b->max_ms;
    } else {
        b->current_ms = (uint32_t)next;
    }
    b->attempts++;

    return delay;
}

bool syn_backoff_exhausted(const SYN_Backoff *b)
{
    SYN_ASSERT(b != NULL);
    return b->attempts >= b->max_attempts;
}
```


