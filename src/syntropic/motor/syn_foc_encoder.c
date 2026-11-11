/**
 * @file syn_foc_encoder.c
 * @brief Sensored FOC Rotor Position & Speed Implementation.
 * @ingroup syn_motor
 */

#include "syn_foc_encoder.h"

#include <string.h>

/** @brief Standard 120-degree 6-step Hall state to electrical angle lookup table (rad in Q16). */
static const q16_t g_hall_angle_lut[8] = {
    0,                                 /* 000 Invalid */
    (q16_t)Q16_FROM_FLOAT(4.7123890f), /* 001: 270 deg */
    (q16_t)Q16_FROM_FLOAT(2.6179939f), /* 010: 150 deg */
    (q16_t)Q16_FROM_FLOAT(3.6651914f), /* 011: 210 deg */
    (q16_t)Q16_FROM_FLOAT(0.5235988f), /* 100: 30 deg  */
    (q16_t)Q16_FROM_FLOAT(5.7595865f), /* 101: 330 deg */
    (q16_t)Q16_FROM_FLOAT(1.5707963f), /* 110: 90 deg  */
    0                                  /* 111 Invalid */
};

/**
 * @brief Wrap signed Q16.16 angle in radians into [0, 2*pi).
 * @param angle Unbounded angle in Q16.16 radians.
 * @return Wrapped electrical angle in [0, Q16_2_PI).
 */
static q16_t foc_enc_wrap_2pi(int64_t angle)
{
    int64_t wrapped = angle % (int64_t)Q16_2_PI;
    if (wrapped < 0) {
        wrapped += (int64_t)Q16_2_PI;
    }
    return (q16_t)wrapped;
}

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

    int64_t delta = (int64_t)raw_count - (int64_t)enc->prev_count;
    enc->count = raw_count;
    enc->prev_count = raw_count;

    uint64_t cpr = (uint64_t)enc->config.cpr;
    int64_t mod_count = (int64_t)raw_count % (int64_t)cpr;
    if (mod_count < 0) {
        mod_count += (int64_t)cpr;
    }

    int64_t elec_cnt =
        (mod_count * (int64_t)enc->config.pole_pairs * (int64_t)Q16_2_PI) / (int64_t)cpr;
    enc->elec_angle_rad = foc_enc_wrap_2pi(elec_cnt + (int64_t)enc->config.zero_offset_rad);

    /* Speed calculation: (delta_count / cpr) * P * 2*PI * sample_rate_hz */
    if (enc->config.sample_rate_hz > 0U) {
        int64_t base = delta * (int64_t)enc->config.pole_pairs * (int64_t)Q16_2_PI;
        uint64_t ubase = (uint64_t)((base < 0) ? -base : base);
        uint64_t sr = (uint64_t)enc->config.sample_rate_hz;
        int64_t spd64;
        if (ubase == 0ULL || sr <= ((uint64_t)INT64_MAX / ubase)) {
            spd64 = (base * (int64_t)sr) / (int64_t)cpr;
        } else {
            spd64 = (base < 0) ? (int64_t)INT32_MIN : (int64_t)INT32_MAX;
        }
        if (spd64 > (int64_t)INT32_MAX) {
            spd64 = (int64_t)INT32_MAX;
        } else if (spd64 < (int64_t)INT32_MIN) {
            spd64 = (int64_t)INT32_MIN;
        }
        enc->elec_speed_rad_s = (q16_t)spd64;
    }
}

void syn_foc_encoder_update_hall(SYN_FOCEncoder *enc, bool hall_u, bool hall_v, bool hall_w)
{
    if (!enc)
        return;
    uint8_t state = (uint8_t)((hall_u ? 4 : 0) | (hall_v ? 2 : 0) | (hall_w ? 1 : 0));
    enc->hall_state = state;
    if (state >= 1 && state <= 6) {
        enc->elec_angle_rad = foc_enc_wrap_2pi((int64_t)g_hall_angle_lut[state] +
                                               (int64_t)enc->config.zero_offset_rad);
    }
}

void syn_foc_encoder_update_absolute(SYN_FOCEncoder *enc, uint16_t raw_angle_14bit)
{
    if (!enc)
        return;
    uint16_t angle_14 = raw_angle_14bit & 0x3FFFU;
    int64_t elec_abs =
        ((int64_t)angle_14 * (int64_t)enc->config.pole_pairs * (int64_t)Q16_2_PI) / 16384LL;
    enc->elec_angle_rad = foc_enc_wrap_2pi(elec_abs + (int64_t)enc->config.zero_offset_rad);
}

q16_t syn_foc_encoder_get_elec_angle(const SYN_FOCEncoder *enc)
{
    return enc ? enc->elec_angle_rad : 0;
}

q16_t syn_foc_encoder_get_elec_speed(const SYN_FOCEncoder *enc)
{
    return enc ? enc->elec_speed_rad_s : 0;
}
