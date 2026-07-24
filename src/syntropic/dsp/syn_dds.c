/**
 * @file syn_dds.c
 * @brief Direct Digital Synthesis (DDS) Waveform Synthesizer implementation.
 */

#include "syn_dds.h"
#include "../util/syn_random.h"
#include <string.h>

SYN_Status syn_dds_init(SYN_DDS *dds, SYN_DDS_Waveform type, uint32_t freq_hz, uint32_t sample_rate_hz)
{
    if (!dds || sample_rate_hz == 0) return SYN_INVALID_PARAM;

    memset(dds, 0, sizeof(*dds));
    dds->type        = type;
    dds->amplitude   = Q16_ONE;
    dds->offset      = 0;
    dds->duty_q16    = 32768U; /* 50% default duty cycle */
    dds->rand_state  = 0x12345678U;

    return syn_dds_set_freq(dds, freq_hz, sample_rate_hz);
}

SYN_Status syn_dds_set_freq(SYN_DDS *dds, uint32_t freq_hz, uint32_t sample_rate_hz)
{
    if (!dds || sample_rate_hz == 0) return SYN_INVALID_PARAM;

    /* phase_step = (freq_hz * 2^32) / sample_rate_hz */
    uint64_t step = ((uint64_t)freq_hz << 32) / (uint64_t)sample_rate_hz;
    dds->phase_step = (uint32_t)step;

    return SYN_OK;
}

SYN_Status syn_dds_set_gain(SYN_DDS *dds, q16_t amplitude, q16_t offset)
{
    if (!dds) return SYN_INVALID_PARAM;

    dds->amplitude = amplitude;
    dds->offset    = offset;

    return SYN_OK;
}

SYN_Status syn_dds_set_duty(SYN_DDS *dds, float duty_pct)
{
    if (!dds) return SYN_INVALID_PARAM;

    if (duty_pct < 0.0f) duty_pct = 0.0f;
    if (duty_pct > 100.0f) duty_pct = 100.0f;

    dds->duty_q16 = (uint16_t)((duty_pct / 100.0f) * 65535.0f);

    return SYN_OK;
}

q16_t syn_dds_step(SYN_DDS *dds)
{
    if (!dds) return 0;

    q16_t raw_sample = 0;
    uint32_t phase = dds->phase;

    switch (dds->type) {
    case SYN_DDS_SINE: {
        /* Convert 32-bit phase (0..2^32-1) to Q16.16 angle (0 to 2*PI), then wrap to -PI..+PI */
        int64_t norm_phase = (int64_t)(phase >> 16); /* 0..65535 */
        q16_t angle = (q16_t)((norm_phase * 411774LL) >> 16); /* 0 to 6.28318 */
        if (angle > 205887LL) {
            angle -= 411774LL;
        }
        raw_sample = q16_sin(angle);
        break;
    }
    case SYN_DDS_TRIANGLE: {
        /* Phase 0..2^31-1 ramps up, 2^31..2^32-1 ramps down */
        if (phase < 0x80000000U) {
            raw_sample = (q16_t)(((int64_t)phase * 131072LL) >> 31) - Q16_ONE;
        } else {
            uint32_t rem = phase - 0x80000000U;
            raw_sample = Q16_ONE - (q16_t)(((int64_t)rem * 131072LL) >> 31);
        }
        break;
    }
    case SYN_DDS_SAWTOOTH: {
        /* Full ramp from -1.0 to +1.0 */
        raw_sample = (q16_t)(((int64_t)phase * 131072LL) >> 32) - Q16_ONE;
        break;
    }
    case SYN_DDS_SQUARE: {
        /* Compare top 16 bits of phase against duty threshold */
        uint16_t top_phase = (uint16_t)(phase >> 16);
        if (top_phase < dds->duty_q16) {
            raw_sample = Q16_ONE;
        } else {
            raw_sample = -Q16_ONE;
        }
        break;
    }
    case SYN_DDS_NOISE: {
        uint32_t rnd = syn_random_u32();
        int32_t signed_val = (int32_t)(rnd & 0xFFFFU) - 32768;
        raw_sample = (q16_t)(signed_val * 2);
        break;
    }
    }

    dds->phase += dds->phase_step;

    /* Apply amplitude gain and DC offset: output = (raw_sample * amplitude) + offset */
    return q16_mul(raw_sample, dds->amplitude) + dds->offset;
}

SYN_Status syn_dds_fill_q16(SYN_DDS *dds, q16_t *buf, size_t count)
{
    if (!dds || !buf) return SYN_INVALID_PARAM;

    for (size_t i = 0; i < count; i++) {
        buf[i] = syn_dds_step(dds);
    }

    return SYN_OK;
}

SYN_Status syn_dds_fill_u16(SYN_DDS *dds, uint16_t *buf, size_t count, uint16_t dac_center, uint16_t dac_span)
{
    if (!dds || !buf) return SYN_INVALID_PARAM;

    for (size_t i = 0; i < count; i++) {
        q16_t sample = syn_dds_step(dds); /* normalized roughly -1.0 to +1.0 */
        int32_t val = (int32_t)dac_center + (int32_t)Q16_TO_INT(sample * dac_span);
        if (val < 0) val = 0;
        if (val > 65535) val = 65535;
        buf[i] = (uint16_t)val;
    }

    return SYN_OK;
}
