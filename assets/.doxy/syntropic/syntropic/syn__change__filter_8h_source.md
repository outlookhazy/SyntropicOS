

# File syn\_change\_filter.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_change\_filter.h**](syn__change__filter_8h.md)

[Go to the documentation of this file](syn__change__filter_8h.md)


```C++

#ifndef SYN_CHANGE_FILTER_H
#define SYN_CHANGE_FILTER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float    last_value;      
    uint32_t last_time_ms;    
    float    dead_band;       
    uint32_t min_interval_ms; 
    bool     initialized;     
} SYN_ChangeFilter;

static inline void syn_change_filter_init(SYN_ChangeFilter *cf,
                                           float dead_band,
                                           uint32_t min_interval_ms)
{
    cf->last_value      = 0.0f;
    cf->last_time_ms    = 0;
    cf->dead_band       = dead_band;
    cf->min_interval_ms = min_interval_ms;
    cf->initialized     = false;
}

static inline bool syn_change_filter_update(SYN_ChangeFilter *cf,
                                             float value,
                                             uint32_t now_ms)
{
    /* First value is always reported */
    if (!cf->initialized) {
        cf->last_value   = value;
        cf->last_time_ms = now_ms;
        cf->initialized  = true;
        return true;
    }

    /* Rate limit check */
    uint32_t elapsed = now_ms - cf->last_time_ms;
    if (cf->min_interval_ms > 0 && elapsed < cf->min_interval_ms) {
        return false;
    }

    /* Dead-band check */
    float delta = value - cf->last_value;
    if (delta < 0.0f) delta = -delta;

    if (delta <= cf->dead_band) {
        return false;
    }

    /* Changed enough and enough time passed */
    cf->last_value   = value;
    cf->last_time_ms = now_ms;
    return true;
}

static inline void syn_change_filter_reset(SYN_ChangeFilter *cf)
{
    cf->initialized = false;
}

static inline float syn_change_filter_last(const SYN_ChangeFilter *cf)
{
    return cf->last_value;
}

static inline void syn_change_filter_set(SYN_ChangeFilter *cf,
                                          float dead_band,
                                          uint32_t min_interval_ms)
{
    cf->dead_band       = dead_band;
    cf->min_interval_ms = min_interval_ms;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_CHANGE_FILTER_H */
```


