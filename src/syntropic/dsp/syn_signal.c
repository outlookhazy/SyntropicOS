#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_SIGNAL) || SYN_USE_SIGNAL

/**
 * @file syn_signal.c
 * @brief Signal statistics implementation.
 */

#include "../util/syn_assert.h"
#include "../util/syn_qmath.h"
#include "syn_signal.h"

#include <limits.h>
#include <string.h>

/* INT32_MIN / INT32_MAX fallback for C99 */
#ifndef INT32_MIN
/** @brief INT32_MIN fallback for pre-C99 compilers. */
#define INT32_MIN (-2147483647 - 1)
#endif
#ifndef INT32_MAX
/** @brief INT32_MAX fallback for pre-C99 compilers. */
#define INT32_MAX 2147483647
#endif

void syn_signal_init(SYN_Signal *sig, int32_t *buf, size_t capacity)
{
    SYN_ASSERT(sig != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(capacity > 0);

    memset(sig, 0, sizeof(*sig));
    sig->buf = buf;
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
    if (sig->head >= sig->capacity)
        sig->head = 0;

    sig->cache_valid = false;
}

void syn_signal_clear(SYN_Signal *sig)
{
    sig->head = 0;
    sig->count = 0;
    sig->sum = 0;
    sig->cache_valid = false;
}

/* ── Min/Max with lazy cache ────────────────────────────────────────────── */

/**
 * @brief Recompute cached min/max values over the signal window.
 * @param sig  Signal instance.
 */
static void recompute_minmax(SYN_Signal *sig)
{
    if (sig->cache_valid || sig->count == 0)
        return;

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
        if (v < lo)
            lo = v;
        if (v > hi)
            hi = v;
    }

    sig->cached_min = lo;
    sig->cached_max = hi;
    sig->cache_valid = true;
}

int32_t syn_signal_min(SYN_Signal *sig)
{
    if (sig->count == 0)
        return 0;
    recompute_minmax(sig);
    return sig->cached_min;
}

int32_t syn_signal_max(SYN_Signal *sig)
{
    if (sig->count == 0)
        return 0;
    recompute_minmax(sig);
    return sig->cached_max;
}

/* ── Variance ───────────────────────────────────────────────────────────── */

/**
 * @brief Accumulate one unsigned squared magnitude into running (quotient, remainder) mod count.
 * @param sq    Squared sample or deviation magnitude.
 * @param n     Total sample count (must be > 0).
 * @param q_io  [in,out] Running integer quotient of sum(sq) / n.
 * @param r_io  [in,out] Running remainder of sum(sq) % n (always < n).
 */
static void signal_accum_mean_sq(uint64_t sq, uint64_t n, uint64_t *q_io, uint64_t *r_io)
{
    *q_io += sq / n;
    uint64_t sq_rem = sq % n;
    if ((n - *r_io) <= sq_rem) {
        (*q_io)++;
        *r_io = sq_rem - (n - *r_io);
    } else {
        *r_io += sq_rem;
    }
}

/**
 * @brief Convert exact (whole, rem) of sum_sq / count into clamped Q16.16 mean square value.
 * @param whole Integer part of sum_sq / count.
 * @param rem   Remainder part of sum_sq % count.
 * @param n     Sample count (must be > 0).
 * @return Mean square value in Q16.16 format, clamped to INT32_MAX.
 */
static int32_t signal_mean_sq_parts_to_q16(uint64_t whole, uint64_t rem, uint64_t n)
{
    if (whole >= 32768ULL) {
        return INT32_MAX;
    }
    return (int32_t)((whole << 16U) + ((rem << 16U) / n));
}

/**
 * @brief Compute exact (whole, rem) of mean squared samples across active window without overflow.
 * @param sig       Signal instance (count > 0).
 * @param out_whole Output integer part of mean square.
 * @param out_rem   Output remainder part of mean square mod count.
 */
static void signal_mean_sq_parts(const SYN_Signal *sig, uint64_t *out_whole, uint64_t *out_rem)
{
    uint64_t q = 0ULL;
    uint64_t r = 0ULL;
    uint64_t n = (uint64_t)sig->count;
    size_t start = (sig->count < sig->capacity) ? 0U : sig->head;

    for (size_t i = 0U; i < sig->count; i++) {
        size_t idx = (start + i) % sig->capacity;
        int64_t v = (int64_t)sig->buf[idx];
        uint64_t mag = (v < 0) ? (uint64_t)(-v) : (uint64_t)v;
        signal_accum_mean_sq(mag * mag, n, &q, &r);
    }
    *out_whole = q;
    *out_rem = r;
}

/**
 * @brief Compute unsaturated RMS value of active window in unsigned Q16.16 format.
 * @param sig Signal instance.
 * @return Unsigned 64-bit Q16.16 RMS value, or 0 if window is empty.
 */
static uint64_t signal_rms_q16_u64(const SYN_Signal *sig)
{
    if (sig->count == 0U) {
        return 0ULL;
    }

    uint64_t whole = 0ULL;
    uint64_t rem = 0ULL;
    uint64_t n = (uint64_t)sig->count;
    signal_mean_sq_parts(sig, &whole, &rem);
    if (whole >= (1ULL << 30U)) {
        return syn_isqrt64(whole) << 16U;
    }
    uint64_t mean_sq_q32 = (whole << 32U) + ((rem << 32U) / n);
    return syn_isqrt64(mean_sq_q32);
}

/**
 * @brief Compute population variance as 64-bit integer and Q0.32 fractional parts without overflow.
 *
 * Uses sum = q_m * N + r_m (|r_m| < N) and the identity
 * E[(X - mu)^2] = E[(X - q_m)^2] - (r_m / N)^2 so fractional means are exact.
 *
 * @param sig          Signal instance (count >= 2).
 * @param out_whole    Output integer part of variance.
 * @param out_frac_q32 Output Q0.32 fractional part of variance in [0, 2^32 - 1].
 */
static void signal_variance_parts(const SYN_Signal *sig, uint64_t *out_whole,
                                  uint64_t *out_frac_q32)
{
    uint64_t n = (uint64_t)sig->count;
    int64_t q_m = sig->sum / (int64_t)sig->count;
    int64_t r_m = sig->sum % (int64_t)sig->count;
    uint64_t u = (r_m < 0) ? (uint64_t)(-r_m) : (uint64_t)r_m;
    uint64_t q = 0ULL;
    uint64_t r = 0ULL;
    size_t start = (sig->count < sig->capacity) ? 0U : sig->head;

    for (size_t i = 0U; i < sig->count; i++) {
        size_t idx = (start + i) % sig->capacity;
        int64_t diff = (int64_t)sig->buf[idx] - q_m;
        uint64_t mag = (diff < 0) ? (uint64_t)(-diff) : (uint64_t)diff;
        signal_accum_mean_sq(mag * mag, n, &q, &r);
    }

    uint64_t r_q32 = (r << 32U) / n;
    uint64_t u_q32 = (u << 32U) / n;
    uint64_t corr_q32 = (u_q32 * u_q32) >> 32U;

    if (r_q32 < corr_q32) {
        q--;
        *out_frac_q32 = (1ULL << 32U) + r_q32 - corr_q32;
    } else {
        *out_frac_q32 = r_q32 - corr_q32;
    }
    *out_whole = q;
}

int32_t syn_signal_variance_q16(const SYN_Signal *sig)
{
    if (sig->count < 2U) {
        return 0;
    }

    uint64_t whole = 0ULL;
    uint64_t frac_q32 = 0ULL;
    signal_variance_parts(sig, &whole, &frac_q32);
    if (whole >= 32768ULL) {
        return INT32_MAX;
    }
    return (int32_t)((whole << 16U) + (frac_q32 >> 16U));
}

/* ── Access helpers ─────────────────────────────────────────────────────── */

int32_t syn_signal_latest(const SYN_Signal *sig)
{
    if (sig->count == 0) {
        return 0;
    }
    size_t idx = (sig->head == 0) ? sig->capacity - 1 : sig->head - 1;
    return sig->buf[idx];
}

int32_t syn_signal_at(const SYN_Signal *sig, size_t index)
{
    if (index >= sig->count) {
        return 0;
    }

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
    if (sig->count < 2U) {
        return 0;
    }

    size_t latest_idx = (sig->head == 0U) ? sig->capacity - 1U : sig->head - 1U;
    size_t prev_idx = (latest_idx == 0U) ? sig->capacity - 1U : latest_idx - 1U;
    int64_t diff = (int64_t)sig->buf[latest_idx] - (int64_t)sig->buf[prev_idx];

    if (diff > (int64_t)INT32_MAX) {
        return INT32_MAX;
    }
    if (diff < (int64_t)INT32_MIN) {
        return INT32_MIN;
    }
    return (int32_t)diff;
}

/* ── RMS & Standard Deviation ──────────────────────────────────────────── */

int32_t syn_signal_rms_q16(const SYN_Signal *sig)
{
    uint64_t rms = signal_rms_q16_u64(sig);
    if (rms > (uint64_t)INT32_MAX) {
        return INT32_MAX;
    }
    return (int32_t)rms;
}

int32_t syn_signal_std_dev_q16(const SYN_Signal *sig)
{
    if (sig->count < 2U) {
        return 0;
    }

    uint64_t whole = 0ULL;
    uint64_t frac_q32 = 0ULL;
    signal_variance_parts(sig, &whole, &frac_q32);
    if (whole >= (1ULL << 30U)) {
        return INT32_MAX;
    }
    return (int32_t)syn_isqrt64((whole << 32U) + frac_q32);
}

int32_t syn_signal_power_q16(const SYN_Signal *sig)
{
    if (sig->count == 0U) {
        return 0;
    }

    uint64_t whole = 0ULL;
    uint64_t rem = 0ULL;
    signal_mean_sq_parts(sig, &whole, &rem);
    return signal_mean_sq_parts_to_q16(whole, rem, (uint64_t)sig->count);
}

int32_t syn_signal_crest_factor_q16(SYN_Signal *sig)
{
    uint64_t rms = signal_rms_q16_u64(sig);
    if (rms == 0ULL) {
        return 0;
    }

    int64_t max_v = (int64_t)syn_signal_max(sig);
    int64_t min_v = (int64_t)syn_signal_min(sig);
    uint64_t peak = (max_v > 0) ? (uint64_t)max_v : 0ULL;
    uint64_t min_mag = (min_v < 0) ? (uint64_t)(-min_v) : 0ULL;
    if (min_mag > peak) {
        peak = min_mag;
    }

    uint64_t cf_q16 = (peak << 32U) / rms;
    return (cf_q16 > (uint64_t)INT32_MAX) ? INT32_MAX : (int32_t)cf_q16;
}

#endif /* SYN_USE_SIGNAL */
