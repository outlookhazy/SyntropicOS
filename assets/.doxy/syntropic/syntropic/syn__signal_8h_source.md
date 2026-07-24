

# File syn\_signal.h

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_signal.h**](syn__signal_8h.md)

[Go to the documentation of this file](syn__signal_8h.md)


```C++

#ifndef SYN_SIGNAL_H
#define SYN_SIGNAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Instance ───────────────────────────────────────────────────────────── */

typedef struct {
    int32_t  *buf;           
    size_t    capacity;      
    size_t    head;          
    size_t    count;         
    /* Running accumulators */
    int64_t   sum;           
    int32_t   cached_min;    
    int32_t   cached_max;    
    bool      cache_valid;   
} SYN_Signal;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_signal_init(SYN_Signal *sig, int32_t *buf, size_t capacity);

void syn_signal_push(SYN_Signal *sig, int32_t sample);

void syn_signal_clear(SYN_Signal *sig);

static inline size_t syn_signal_count(const SYN_Signal *sig)
{
    return sig->count;
}

static inline bool syn_signal_full(const SYN_Signal *sig)
{
    return sig->count == sig->capacity;
}

int32_t syn_signal_min(SYN_Signal *sig);

int32_t syn_signal_max(SYN_Signal *sig);

static inline int32_t syn_signal_peak_to_peak(SYN_Signal *sig)
{
    if (sig->count == 0) return 0;
    return syn_signal_max(sig) - syn_signal_min(sig);
}

static inline int32_t syn_signal_mean(const SYN_Signal *sig)
{
    if (sig->count == 0) return 0;
    return (int32_t)(sig->sum / (int64_t)sig->count);
}

static inline int64_t syn_signal_sum(const SYN_Signal *sig)
{
    return sig->sum;
}

int32_t syn_signal_variance_q16(const SYN_Signal *sig);

int32_t syn_signal_latest(const SYN_Signal *sig);

int32_t syn_signal_at(const SYN_Signal *sig, size_t index);

int32_t syn_signal_delta(const SYN_Signal *sig);

int32_t syn_signal_rms_q16(const SYN_Signal *sig);

int32_t syn_signal_std_dev_q16(const SYN_Signal *sig);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SIGNAL_H */
```


