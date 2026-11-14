

# File syn\_goertzel.c

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_goertzel.c**](syn__goertzel_8c.md)

[Go to the documentation of this file](syn__goertzel_8c.md)


```C++


#include "syn_goertzel.h"

#include "../util/syn_qmath.h"

#include <string.h>

#define Q14_ONE 16384L 

SYN_Status syn_goertzel_init(SYN_Goertzel *g, uint32_t sample_rate_hz, uint32_t target_freq_hz,
                             size_t block_size)
{
    if ((g == NULL) || (sample_rate_hz == 0U) || (target_freq_hz == 0U) || (block_size == 0U) ||
        (target_freq_hz >= sample_rate_hz / 2U)) {
        return SYN_INVALID_PARAM;
    }

    memset(g, 0, sizeof(SYN_Goertzel));
    g->n_total = block_size;

    /* Calculate bin index k = round(N * f_target / f_s) */
    uint32_t k = (uint32_t)(((uint64_t)block_size * target_freq_hz + (sample_rate_hz / 2U)) /
                            sample_rate_hz);

    /* Angle omega = 2 * pi * k / N in Q16 format (2 * pi = 411775 Q16) */
    q16_t omega = (q16_t)(((uint64_t)411775U * k) / block_size);

    /* Cosine in Q16 format */
    q16_t cos_val = q16_cos(omega);

    /* Goertzel coeff = 2 * cos(omega) in Q14 format (16384 = 1.0) */
    g->coeff_q15 = (int32_t)(cos_val >> 1);

    return SYN_OK;
}

SYN_Status syn_goertzel_reset(SYN_Goertzel *g)
{
    if (g == NULL) {
        return SYN_INVALID_PARAM;
    }

    g->s1 = 0;
    g->s2 = 0;
    g->n_count = 0U;

    return SYN_OK;
}

SYN_Status syn_goertzel_process_sample(SYN_Goertzel *g, int16_t sample)
{
    if (g == NULL) {
        return SYN_INVALID_PARAM;
    }

    int64_t fb = ((int64_t)g->coeff_q15 * (int64_t)g->s1) >> 14;
    int64_t s0 = (int64_t)sample + fb - (int64_t)g->s2;
    if (s0 > 2147483647LL) {
        s0 = 2147483647LL;
    } else if (s0 < -2147483647LL) {
        s0 = -2147483647LL;
    }
    g->s2 = g->s1;
    g->s1 = (int32_t)s0;
    g->n_count++;

    return SYN_OK;
}

size_t syn_goertzel_process_block(SYN_Goertzel *g, const int16_t *samples, size_t count)
{
    if ((g == NULL) || (samples == NULL) || (count == 0U)) {
        return 0U;
    }

    size_t processed = 0U;
    for (size_t i = 0U; i < count; i++) {
        if (syn_goertzel_process_sample(g, samples[i]) == SYN_OK) {
            processed++;
        }
    }

    return processed;
}

uint64_t syn_goertzel_get_magnitude_sq(const SYN_Goertzel *g)
{
    if (g == NULL) {
        return 0ULL;
    }

    int64_t s1 = (g->s1 == INT32_MIN) ? -2147483647LL : (int64_t)g->s1;
    int64_t s2 = (g->s2 == INT32_MIN) ? -2147483647LL : (int64_t)g->s2;
    int64_t scaled_s1 = ((int64_t)g->coeff_q15 * s1) >> 14;
    int64_t prod = scaled_s1 * s2;
    uint64_t sq_sum = (uint64_t)(s1 * s1) + (uint64_t)(s2 * s2);

    if (prod < 0) {
        return sq_sum + (uint64_t)(-prod);
    }
    if ((uint64_t)prod >= sq_sum) {
        return 0ULL;
    }
    return sq_sum - (uint64_t)prod;
}
```


