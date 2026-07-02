

# File syn\_filter.h

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_filter.h**](syn__filter_8h.md)

[Go to the documentation of this file](syn__filter_8h.md)


```C++

#ifndef SYN_FILTER_H
#define SYN_FILTER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

#ifndef SYN_FILTER_MAX_WINDOW
  #define SYN_FILTER_MAX_WINDOW  32
#endif

/* ── Moving Average ─────────────────────────────────────────────────────── */

typedef struct {
    int16_t   buf[SYN_FILTER_MAX_WINDOW]; 
    int32_t   sum;      
    uint8_t   window;   
    uint8_t   count;    
    uint8_t   idx;      
} SYN_FilterMA;

void syn_filter_ma_init(SYN_FilterMA *f, uint8_t window);

int16_t syn_filter_ma_update(SYN_FilterMA *f, int16_t sample);

void syn_filter_ma_reset(SYN_FilterMA *f);

/* ── Exponential Moving Average ─────────────────────────────────────────── */

typedef struct {
    int32_t   value;    
    uint8_t   alpha;    
    bool      primed;   
} SYN_FilterEMA;

void syn_filter_ema_init(SYN_FilterEMA *f, uint8_t alpha);

int16_t syn_filter_ema_update(SYN_FilterEMA *f, int16_t sample);

void syn_filter_ema_reset(SYN_FilterEMA *f);

/* ── Median Filter ──────────────────────────────────────────────────────── */

typedef struct {
    int16_t   buf[SYN_FILTER_MAX_WINDOW]; 
    uint8_t   window;   
    uint8_t   count;    
    uint8_t   idx;      
} SYN_FilterMedian;

void syn_filter_median_init(SYN_FilterMedian *f, uint8_t window);

int16_t syn_filter_median_update(SYN_FilterMedian *f, int16_t sample);

void syn_filter_median_reset(SYN_FilterMedian *f);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FILTER_H */
```


