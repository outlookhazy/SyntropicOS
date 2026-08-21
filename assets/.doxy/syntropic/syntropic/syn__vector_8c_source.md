

# File syn\_vector.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_vector.c**](syn__vector_8c.md)

[Go to the documentation of this file](syn__vector_8c.md)


```C++

#include "syn_vector.h"

#include "syn_assert.h"

void syn_vec_add(const q16_t *a, const q16_t *b, q16_t *out, uint16_t n)
{
    SYN_ASSERT(a != NULL && b != NULL && out != NULL);
    for (uint16_t i = 0; i < n; i++) {
        out[i] = a[i] + b[i];
    }
}

void syn_vec_sub(const q16_t *a, const q16_t *b, q16_t *out, uint16_t n)
{
    SYN_ASSERT(a != NULL && b != NULL && out != NULL);
    for (uint16_t i = 0; i < n; i++) {
        out[i] = a[i] - b[i];
    }
}

void syn_vec_scale(const q16_t *v, q16_t scale, q16_t *out, uint16_t n)
{
    SYN_ASSERT(v != NULL && out != NULL);
    for (uint16_t i = 0; i < n; i++) {
        out[i] = q16_mul(v[i], scale);
    }
}

void syn_vec_clamp(const q16_t *v, q16_t min_val, q16_t max_val, q16_t *out, uint16_t n)
{
    SYN_ASSERT(v != NULL && out != NULL);
    for (uint16_t i = 0; i < n; i++) {
        q16_t val = v[i];
        if (val < min_val)
            val = min_val;
        if (val > max_val)
            val = max_val;
        out[i] = val;
    }
}

q16_t syn_vec_min(const q16_t *v, uint16_t n)
{
    SYN_ASSERT(v != NULL && n > 0);
    q16_t min_v = v[0];
    for (uint16_t i = 1; i < n; i++) {
        if (v[i] < min_v)
            min_v = v[i];
    }
    return min_v;
}

q16_t syn_vec_max(const q16_t *v, uint16_t n)
{
    SYN_ASSERT(v != NULL && n > 0);
    q16_t max_v = v[0];
    for (uint16_t i = 1; i < n; i++) {
        if (v[i] > max_v)
            max_v = v[i];
    }
    return max_v;
}

q16_t syn_vec_mean(const q16_t *v, uint16_t n)
{
    SYN_ASSERT(v != NULL && n > 0);
    int64_t sum = 0;
    for (uint16_t i = 0; i < n; i++) {
        sum += v[i];
    }
    return (q16_t)(sum / n);
}

q16_t syn_vec_variance(const q16_t *v, uint16_t n)
{
    SYN_ASSERT(v != NULL && n > 0);
    q16_t mean = syn_vec_mean(v, n);
    int64_t sum_sq = 0;
    for (uint16_t i = 0; i < n; i++) {
        int64_t diff = v[i] - mean;
        sum_sq += (diff * diff) >> Q16_SHIFT;
    }
    return (q16_t)(sum_sq / n);
}

q16_t syn_vec_rms(const q16_t *v, uint16_t n)
{
    SYN_ASSERT(v != NULL && n > 0);
    int64_t sum_sq = 0;
    for (uint16_t i = 0; i < n; i++) {
        sum_sq += ((int64_t)v[i] * v[i]) >> Q16_SHIFT;
    }
    q16_t mean_sq = (q16_t)(sum_sq / n);
    return q16_sqrt(mean_sq);
}
```


