

# File syn\_filter.c

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_filter.c**](syn__filter_8c.md)

[Go to the documentation of this file](syn__filter_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_FILTER) || SYN_USE_FILTER

#include "syn_filter.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ════════════════════════════════════════════════════════════════════════ */
/*  Moving Average                                                         */
/* ════════════════════════════════════════════════════════════════════════ */

void syn_filter_ma_init(SYN_FilterMA *f, uint8_t window)
{
    SYN_ASSERT(f != NULL);
    SYN_ASSERT(window > 0 && window <= SYN_FILTER_MAX_WINDOW);

    memset(f, 0, sizeof(*f));
    f->window = window;
}

int16_t syn_filter_ma_update(SYN_FilterMA *f, int16_t sample)
{
    SYN_ASSERT(f != NULL);

    if (f->count >= f->window) {
        /* Remove oldest sample from running sum */
        f->sum -= f->buf[f->idx];
    } else {
        f->count++;
    }

    /* Insert new sample */
    f->buf[f->idx] = sample;
    f->sum += sample;
    f->idx = (uint8_t)((f->idx + 1) % f->window);

    return (int16_t)(f->sum / f->count);
}

void syn_filter_ma_reset(SYN_FilterMA *f)
{
    SYN_ASSERT(f != NULL);
    uint8_t w = f->window;
    memset(f, 0, sizeof(*f));
    f->window = w;
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Exponential Moving Average                                             */
/* ════════════════════════════════════════════════════════════════════════ */

void syn_filter_ema_init(SYN_FilterEMA *f, uint8_t alpha)
{
    SYN_ASSERT(f != NULL);

    f->value  = 0;
    f->alpha  = alpha;
    f->primed = false;
}

int16_t syn_filter_ema_update(SYN_FilterEMA *f, int16_t sample)
{
    SYN_ASSERT(f != NULL);

    if (!f->primed) {
        /* First sample — initialize directly */
        f->value  = (int32_t)sample << 8;
        f->primed = true;
        return sample;
    }

    /* EMA: out = alpha * sample + (256 - alpha) * prev, all in Q8 */
    int32_t sample_q8 = (int32_t)sample << 8;
    f->value += ((int32_t)f->alpha * (sample_q8 - f->value)) >> 8;

    return (int16_t)(f->value >> 8);
}

void syn_filter_ema_reset(SYN_FilterEMA *f)
{
    SYN_ASSERT(f != NULL);
    uint8_t a = f->alpha;
    f->value  = 0;
    f->primed = false;
    f->alpha  = a;
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Median Filter                                                          */
/* ════════════════════════════════════════════════════════════════════════ */

void syn_filter_median_init(SYN_FilterMedian *f, uint8_t window)
{
    SYN_ASSERT(f != NULL);
    SYN_ASSERT(window > 0 && window <= SYN_FILTER_MAX_WINDOW);

    memset(f, 0, sizeof(*f));
    f->window = window;
}

int16_t syn_filter_median_update(SYN_FilterMedian *f, int16_t sample)
{
    SYN_ASSERT(f != NULL);

    /* Insert into circular buffer */
    f->buf[f->idx] = sample;
    f->idx = (uint8_t)((f->idx + 1) % f->window);
    if (f->count < f->window) {
        f->count++;
    }

    /* Sort a copy to find median (insertion sort — fine for small windows) */
    int16_t sorted[SYN_FILTER_MAX_WINDOW];
    uint8_t n = f->count;
    memcpy(sorted, f->buf, (size_t)n * sizeof(int16_t));

    uint8_t i;
    for (i = 1; i < n; i++) {
        int16_t key = sorted[i];
        uint8_t j = i;
        while (j > 0 && sorted[j - 1] > key) {
            sorted[j] = sorted[j - 1];
            j--;
        }
        sorted[j] = key;
    }

    return sorted[n / 2];
}

void syn_filter_median_reset(SYN_FilterMedian *f)
{
    SYN_ASSERT(f != NULL);
    uint8_t w = f->window;
    memset(f, 0, sizeof(*f));
    f->window = w;
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  FIR Filter                                                             */
/* ════════════════════════════════════════════════════════════════════════ */

void syn_filter_fir_init(SYN_FilterFIR *f, const q16_t *taps, q16_t *history, uint16_t num_taps)
{
    SYN_ASSERT(f != NULL && taps != NULL && history != NULL && num_taps > 0);

    f->taps     = taps;
    f->history  = history;
    f->num_taps = num_taps;
    f->head     = 0;
    memset(history, 0, (size_t)num_taps * sizeof(q16_t));
}

q16_t syn_filter_fir_update(SYN_FilterFIR *f, q16_t sample)
{
    SYN_ASSERT(f != NULL && f->taps != NULL && f->history != NULL);

    /* Insert new sample into circular history */
    f->history[f->head] = sample;

    int64_t acc = 0;
    uint16_t idx = f->head;
    for (uint16_t i = 0; i < f->num_taps; i++) {
        acc += (int64_t)f->taps[i] * f->history[idx];
        if (idx == 0) {
            idx = f->num_taps - 1;
        } else {
            idx--;
        }
    }

    f->head = (uint16_t)((f->head + 1) % f->num_taps);
    return (q16_t)(acc >> Q16_SHIFT);
}

void syn_filter_fir_reset(SYN_FilterFIR *f)
{
    SYN_ASSERT(f != NULL && f->history != NULL);
    memset(f->history, 0, (size_t)f->num_taps * sizeof(q16_t));
    f->head = 0;
}

#endif /* SYN_USE_FILTER */
```


