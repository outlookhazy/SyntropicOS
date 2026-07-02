

# File syn\_hysteresis.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_hysteresis.h**](syn__hysteresis_8h.md)

[Go to the documentation of this file](syn__hysteresis_8h.md)


```C++

#ifndef SYN_HYSTERESIS_H
#define SYN_HYSTERESIS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t  threshold;     
    int32_t  band;          
    bool     state;         
} SYN_Hysteresis;

static inline void syn_hyst_init(SYN_Hysteresis *h,
                                  int32_t threshold,
                                  int32_t band,
                                  bool initial)
{
    h->threshold = threshold;
    h->band      = band;
    h->state     = initial;
}

static inline bool syn_hyst_update(SYN_Hysteresis *h, int32_t value)
{
    if (h->state) {
        /* Currently high — switch low only if value drops below (threshold - band) */
        if (value < h->threshold - h->band) {
            h->state = false;
        }
    } else {
        /* Currently low — switch high only if value rises above (threshold + band) */
        if (value > h->threshold + h->band) {
            h->state = true;
        }
    }
    return h->state;
}

static inline bool syn_hyst_state(const SYN_Hysteresis *h)
{
    return h->state;
}

static inline void syn_hyst_set(SYN_Hysteresis *h,
                                 int32_t threshold, int32_t band)
{
    h->threshold = threshold;
    h->band      = band;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_HYSTERESIS_H */
```


