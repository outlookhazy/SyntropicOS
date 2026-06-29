#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_ENCODER) || SYN_USE_ENCODER

/**
 * @file syn_encoder.c
 * @brief Quadrature rotary encoder implementation.
 *
 * Uses a 2-bit state machine with a lookup table for direction decoding.
 * Handles all four quadrature transitions correctly with no missed steps.
 */

#include "syn_encoder.h"
#include "../util/syn_assert.h"

#include <string.h>

/**
 * @brief Quadrature state machine lookup table.
 *
 * Index = (prev_state << 2) | new_state.
 * Value = direction: +1 = CW, -1 = CCW, 0 = invalid/no change.
 */
static const int8_t quad_table[16] = {
    /*          new: 00  01  10  11 */
    /* prev 00 */  0, -1, +1,  0,
    /* prev 01 */ +1,  0,  0, -1,
    /* prev 10 */ -1,  0,  0, +1,
    /* prev 11 */  0, +1, -1,  0,
};

void syn_encoder_init(SYN_Encoder *enc,
                       SYN_GPIO_Pin pin_a,
                       SYN_GPIO_Pin pin_b)
{
    SYN_ASSERT(enc != NULL);

    memset(enc, 0, sizeof(*enc));
    enc->pin_a = pin_a;
    enc->pin_b = pin_b;
    enc->steps_per_detent = 1;

    /* Read initial state */
    uint8_t a = (syn_port_gpio_read(pin_a) == SYN_GPIO_HIGH) ? 1 : 0;
    uint8_t b = (syn_port_gpio_read(pin_b) == SYN_GPIO_HIGH) ? 1 : 0;
    enc->last_state = (uint8_t)((a << 1) | b);
}

void syn_encoder_set_steps_per_detent(SYN_Encoder *enc, uint8_t spd)
{
    SYN_ASSERT(enc != NULL);
    SYN_ASSERT(spd > 0);
    enc->steps_per_detent = spd;
    enc->sub_count = 0;
}

void syn_encoder_update(SYN_Encoder *enc)
{
    SYN_ASSERT(enc != NULL);

    uint8_t a = (syn_port_gpio_read(enc->pin_a) == SYN_GPIO_HIGH) ? 1 : 0;
    uint8_t b = (syn_port_gpio_read(enc->pin_b) == SYN_GPIO_HIGH) ? 1 : 0;
    uint8_t new_state = (uint8_t)((a << 1) | b);

    if (new_state == enc->last_state) return;

    uint8_t idx = (uint8_t)((enc->last_state << 2) | new_state);
    int8_t dir = quad_table[idx];

    enc->last_state = new_state;

    if (dir == 0) return; /* invalid transition (noise) — ignore */

    enc->last_dir = dir;

    if (enc->steps_per_detent <= 1) {
        enc->position += dir;
        enc->delta    += dir;
    } else {
        enc->sub_count += dir;
        if (enc->sub_count >= (int8_t)enc->steps_per_detent) {
            enc->position++;
            enc->delta++;
            enc->sub_count = 0;
        } else if (enc->sub_count <= -(int8_t)enc->steps_per_detent) {
            enc->position--;
            enc->delta--;
            enc->sub_count = 0;
        }
    }
}

int32_t syn_encoder_get_delta(SYN_Encoder *enc)
{
    SYN_ASSERT(enc != NULL);
    int32_t d = enc->delta;
    enc->delta = 0;

    /* Push to velocity stats window (if attached) */
    if (enc->stats != NULL) {
        syn_signal_push(enc->stats, d);
    }

    return d;
}

void syn_encoder_set_stats(SYN_Encoder *enc, SYN_Signal *stats)
{
    SYN_ASSERT(enc != NULL);
    enc->stats = stats;
}

#endif /* SYN_USE_ENCODER */
