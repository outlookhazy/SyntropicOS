

# File syn\_adpcm.c

[**File List**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_adpcm.c**](syn__adpcm_8c.md)

[Go to the documentation of this file](syn__adpcm_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_ADPCM) || SYN_USE_ADPCM

#include "../util/syn_assert.h"
#include "syn_adpcm.h"

#include <string.h>

static const int16_t g_adpcm_step_table[89] = {
    7,     8,     9,     10,    11,    12,    13,    14,    16,    17,    19,   21,    23,
    25,    28,    31,    34,    37,    41,    45,    50,    55,    60,    66,   73,    80,
    88,    97,    107,   118,   130,   143,   157,   173,   190,   209,   230,  253,   279,
    307,   337,   371,   408,   449,   494,   544,   598,   658,   724,   796,  876,   963,
    1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,  2272,  2499,  2749, 3024,  3327,
    3660,  4026,  4428,  4871,  5358,  5894,  6484,  7132,  7845,  8630,  9493, 10442, 11487,
    12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767};

static const int8_t g_adpcm_index_table[16] = {-1, -1, -1, -1, 2, 4, 6, 8,
                                               -1, -1, -1, -1, 2, 4, 6, 8};

static inline int16_t clamp_s16(int32_t val)
{
    if (val > 32767) {
        return 32767;
    }
    if (val < -32768) {
        return -32768;
    }
    return (int16_t)val;
}

static inline int8_t clamp_step_idx(int32_t idx)
{
    if (idx < 0) {
        return 0;
    }
    if (idx > 88) {
        return 88;
    }
    return (int8_t)idx;
}
void syn_adpcm_init(SYN_ADPCM_State *state)
{
    if (state == NULL) {
        return;
    }
    state->predicted_sample = 0;
    state->step_index = 0;
}

int16_t syn_adpcm_decode_sample(SYN_ADPCM_State *state, uint8_t nibble)
{
    if (state == NULL) {
        return 0;
    }

    uint8_t delta_code = nibble & 0x0FU;
    int16_t step = g_adpcm_step_table[state->step_index];

    /* Calculate difference = step * (delta + 0.5) / 4 */
    int32_t diff = step >> 3;
    if ((delta_code & 0x04U) != 0U) {
        diff += step;
    }
    if ((delta_code & 0x02U) != 0U) {
        diff += step >> 1;
    }
    if ((delta_code & 0x01U) != 0U) {
        diff += step >> 2;
    }

    int32_t pred = state->predicted_sample;
    if ((delta_code & 0x08U) != 0U) {
        pred -= diff;
    } else {
        pred += diff;
    }

    state->predicted_sample = clamp_s16(pred);
    state->step_index =
        clamp_step_idx((int32_t)state->step_index + g_adpcm_index_table[delta_code]);

    return state->predicted_sample;
}

uint8_t syn_adpcm_encode_sample(SYN_ADPCM_State *state, int16_t sample)
{
    if (state == NULL) {
        return 0;
    }

    int16_t step = g_adpcm_step_table[state->step_index];
    int32_t diff = (int32_t)sample - (int32_t)state->predicted_sample;
    uint8_t delta_code = 0;

    if (diff < 0) {
        delta_code = 0x08U;
        diff = -diff;
    }

    int32_t mask = 0x04;
    int32_t temp_step = step;
    int32_t diff_q = step >> 3;

    for (int i = 0; i < 3; i++) {
        if (diff >= temp_step) {
            delta_code |= (uint8_t)mask;
            diff -= temp_step;
            diff_q += temp_step;
        }
        temp_step >>= 1;
        mask >>= 1;
    }

    int32_t pred = state->predicted_sample;
    if ((delta_code & 0x08U) != 0U) {
        pred -= diff_q;
    } else {
        pred += diff_q;
    }

    state->predicted_sample = clamp_s16(pred);
    state->step_index =
        clamp_step_idx((int32_t)state->step_index + g_adpcm_index_table[delta_code]);

    return delta_code;
}

size_t syn_adpcm_decode_block(SYN_ADPCM_State *state, const uint8_t *in, int16_t *out,
                              size_t sample_count)
{
    if (state == NULL || in == NULL || out == NULL || sample_count == 0U) {
        return 0U;
    }

    size_t samples_done = 0U;
    size_t byte_idx = 0U;

    while (samples_done < sample_count) {
        uint8_t byte = in[byte_idx++];
        out[samples_done++] = syn_adpcm_decode_sample(state, byte & 0x0FU);
        if (samples_done < sample_count) {
            out[samples_done++] = syn_adpcm_decode_sample(state, (byte >> 4) & 0x0FU);
        }
    }

    return samples_done;
}

size_t syn_adpcm_encode_block(SYN_ADPCM_State *state, const int16_t *in, uint8_t *out,
                              size_t sample_count)
{
    if (state == NULL || in == NULL || out == NULL || sample_count == 0U) {
        return 0U;
    }

    size_t samples_done = 0U;
    size_t byte_idx = 0U;

    while (samples_done < sample_count) {
        uint8_t low_nibble = syn_adpcm_encode_sample(state, in[samples_done++]);
        uint8_t high_nibble = 0;
        if (samples_done < sample_count) {
            high_nibble = syn_adpcm_encode_sample(state, in[samples_done++]);
        }
        out[byte_idx++] = (uint8_t)((high_nibble << 4) | (low_nibble & 0x0FU));
    }

    return byte_idx;
}

#endif /* SYN_USE_ADPCM */
```


