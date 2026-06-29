#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_FILTER) || SYN_USE_FILTER

/**
 * @file syn_filter.c
 * @brief Digital filter implementations.
 */

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

#endif /* SYN_USE_FILTER */
