

# File syn\_foc\_encoder.c

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_foc\_encoder.c**](syn__foc__encoder_8c.md)

[Go to the documentation of this file](syn__foc__encoder_8c.md)


```C++

#include "syn_foc_encoder.h"

#include <string.h>

static const q16_t g_hall_angle_lut[8] = {
    0,                                 /* 000 Invalid */
    (q16_t)Q16_FROM_FLOAT(5.2359877f), /* 001: 300 deg */
    (q16_t)Q16_FROM_FLOAT(2.0943951f), /* 010: 120 deg */
    (q16_t)Q16_FROM_FLOAT(3.6651914f), /* 011: 210 deg */
    (q16_t)Q16_FROM_FLOAT(0.5235987f), /* 100: 30 deg  */
    (q16_t)Q16_FROM_FLOAT(5.7595865f), /* 101: 330 deg */
    (q16_t)Q16_FROM_FLOAT(1.0471975f), /* 110: 60 deg  */
    0                                  /* 111 Invalid */
};

SYN_Status syn_foc_encoder_init(SYN_FOCEncoder *enc, const SYN_FOCEncoderConfig *cfg)
{
    if (!enc || !cfg || cfg->pole_pairs == 0) {
        return SYN_INVALID_PARAM;
    }
    memset(enc, 0, sizeof(*enc));
    enc->config = *cfg;
    return SYN_OK;
}

void syn_foc_encoder_update_quadrature(SYN_FOCEncoder *enc, int32_t raw_count)
{
    if (!enc || enc->config.cpr == 0)
        return;

    int32_t delta = raw_count - enc->prev_count;
    enc->count = raw_count;
    enc->prev_count = raw_count;

    /* Mechanical angle in Q16: (count % cpr) / cpr * 2*PI */
    int32_t cpr = (int32_t)enc->config.cpr;
    int32_t mod_count = raw_count % cpr;
    if (mod_count < 0)
        mod_count += cpr;

    q16_t mech_angle = q16_div(Q16_FROM_INT(mod_count), Q16_FROM_INT(cpr));
    mech_angle = q16_mul(mech_angle, Q16_2_PI);

    /* Electrical angle = (P * mech_angle + offset) % 2*PI */
    q16_t elec_raw = q16_mul(Q16_FROM_INT(enc->config.pole_pairs), mech_angle);
    elec_raw += enc->config.zero_offset_rad;

    while (elec_raw >= Q16_2_PI)
        elec_raw -= Q16_2_PI;
    while (elec_raw < 0)
        elec_raw += Q16_2_PI;

    enc->elec_angle_rad = elec_raw;

    /* Speed calculation: (delta_count / cpr) * P * 2*PI * sample_rate_hz */
    if (enc->config.sample_rate_hz > 0) {
        q16_t revs = q16_div(Q16_FROM_INT(delta), Q16_FROM_INT(cpr));
        q16_t mech_speed = q16_mul(revs, Q16_FROM_INT((int32_t)enc->config.sample_rate_hz));
        enc->elec_speed_rad_s =
            q16_mul(q16_mul(mech_speed, Q16_FROM_INT(enc->config.pole_pairs)), Q16_2_PI);
    }
}

void syn_foc_encoder_update_hall(SYN_FOCEncoder *enc, bool hall_u, bool hall_v, bool hall_w)
{
    if (!enc)
        return;
    uint8_t state = (uint8_t)((hall_u ? 4 : 0) | (hall_v ? 2 : 0) | (hall_w ? 1 : 0));
    enc->hall_state = state;
    if (state >= 1 && state <= 6) {
        q16_t angle = g_hall_angle_lut[state] + enc->config.zero_offset_rad;
        while (angle >= Q16_2_PI)
            angle -= Q16_2_PI;
        while (angle < 0)
            angle += Q16_2_PI;
        enc->elec_angle_rad = angle;
    }
}

void syn_foc_encoder_update_absolute(SYN_FOCEncoder *enc, uint16_t raw_angle_14bit)
{
    if (!enc)
        return;
    uint16_t angle_14 = raw_angle_14bit & 0x3FFFU;
    q16_t norm_angle = q16_div(Q16_FROM_INT((int32_t)angle_14), Q16_FROM_INT(16384));
    q16_t mech_angle = q16_mul(norm_angle, Q16_2_PI);

    q16_t elec_raw =
        q16_mul(Q16_FROM_INT(enc->config.pole_pairs), mech_angle) + enc->config.zero_offset_rad;
    while (elec_raw >= Q16_2_PI)
        elec_raw -= Q16_2_PI;
    while (elec_raw < 0)
        elec_raw += Q16_2_PI;

    enc->elec_angle_rad = elec_raw;
}

q16_t syn_foc_encoder_get_elec_angle(const SYN_FOCEncoder *enc)
{
    return enc ? enc->elec_angle_rad : 0;
}

q16_t syn_foc_encoder_get_elec_speed(const SYN_FOCEncoder *enc)
{
    return enc ? enc->elec_speed_rad_s : 0;
}
```


