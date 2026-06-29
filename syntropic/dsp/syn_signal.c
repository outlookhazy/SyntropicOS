#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SIGNAL) || SYN_USE_SIGNAL

/**
 * @file syn_signal.c
 * @brief Signal statistics implementation.
 */

#include "syn_signal.h"
#include "../util/syn_assert.h"

#include <string.h>
#include <limits.h>

/* INT32_MIN / INT32_MAX fallback for C99 */
#ifndef INT32_MIN
/** @brief INT32_MIN fallback for pre-C99 compilers. */
#define INT32_MIN  (-2147483647 - 1)
#endif
#ifndef INT32_MAX
/** @brief INT32_MAX fallback for pre-C99 compilers. */
#define INT32_MAX  2147483647
#endif

void syn_signal_init(SYN_Signal *sig, int32_t *buf, size_t capacity)
{
    SYN_ASSERT(sig != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(capacity > 0);

    memset(sig, 0, sizeof(*sig));
    sig->buf      = buf;
    sig->capacity = capacity;
    memset(buf, 0, sizeof(int32_t) * capacity);
}

void syn_signal_push(SYN_Signal *sig, int32_t sample)
{
    /* If window is full, evict oldest */
    if (sig->count == sig->capacity) {
        sig->sum -= sig->buf[sig->head];
    } else {
        sig->count++;
    }

    sig->buf[sig->head] = sample;
    sig->sum += sample;

    sig->head++;
    if (sig->head >= sig->capacity) sig->head = 0;

    sig->cache_valid = false;
}

void syn_signal_clear(SYN_Signal *sig)
{
    sig->head  = 0;
    sig->count = 0;
    sig->sum   = 0;
    sig->cache_valid = false;
}

/* ── Min/Max with lazy cache ────────────────────────────────────────────── */

/**
 * @brief Recompute cached min/max values over the signal window.
 * @param sig  Signal instance.
 */
static void recompute_minmax(SYN_Signal *sig)
{
    if (sig->cache_valid || sig->count == 0) return;

    int32_t lo = INT32_MAX;
    int32_t hi = INT32_MIN;
    size_t i;

    /* Walk the valid entries */
    size_t start;
    if (sig->count < sig->capacity) {
        start = 0;
    } else {
        start = sig->head; /* oldest */
    }

    for (i = 0; i < sig->count; i++) {
        size_t idx = (start + i) % sig->capacity;
        int32_t v = sig->buf[idx];
        if (v < lo) lo = v;
        if (v > hi) hi = v;
    }

    sig->cached_min  = lo;
    sig->cached_max  = hi;
    sig->cache_valid = true;
}

int32_t syn_signal_min(SYN_Signal *sig)
{
    if (sig->count == 0) return 0;
    recompute_minmax(sig);
    return sig->cached_min;
}

int32_t syn_signal_max(SYN_Signal *sig)
{
    if (sig->count == 0) return 0;
    recompute_minmax(sig);
    return sig->cached_max;
}

/* ── Variance ───────────────────────────────────────────────────────────── */

int32_t syn_signal_variance_q16(const SYN_Signal *sig)
{
    if (sig->count < 2) return 0;

    int32_t mean = (int32_t)(sig->sum / (int64_t)sig->count);

    int64_t sum_sq = 0;
    size_t start;
    if (sig->count < sig->capacity) {
        start = 0;
    } else {
        start = sig->head;
    }

    size_t i;
    for (i = 0; i < sig->count; i++) {
        size_t idx = (start + i) % sig->capacity;
        int64_t diff = (int64_t)sig->buf[idx] - mean;
        sum_sq += diff * diff;
    }

    /* Variance in Q16.16: (sum_sq << 16) / N */
    int64_t var_q16 = (sum_sq << 16) / (int64_t)sig->count;

    /* Clamp to int32_t range */
    if (var_q16 > INT32_MAX) return INT32_MAX;
    if (var_q16 < INT32_MIN) return INT32_MIN;

    return (int32_t)var_q16;
}

/* ── Access helpers ─────────────────────────────────────────────────────── */

int32_t syn_signal_latest(const SYN_Signal *sig)
{
    if (sig->count == 0) return 0;
    size_t idx = (sig->head == 0) ? sig->capacity - 1 : sig->head - 1;
    return sig->buf[idx];
}

int32_t syn_signal_at(const SYN_Signal *sig, size_t index)
{
    if (index >= sig->count) return 0;

    size_t start;
    if (sig->count < sig->capacity) {
        start = 0;
    } else {
        start = sig->head;
    }

    size_t actual = (start + index) % sig->capacity;
    return sig->buf[actual];
}

int32_t syn_signal_delta(const SYN_Signal *sig)
{
    if (sig->count < 2) return 0;

    size_t latest_idx = (sig->head == 0) ? sig->capacity - 1 : sig->head - 1;
    size_t prev_idx   = (latest_idx == 0) ? sig->capacity - 1 : latest_idx - 1;

    return sig->buf[latest_idx] - sig->buf[prev_idx];
}

#endif /* SYN_USE_SIGNAL */
