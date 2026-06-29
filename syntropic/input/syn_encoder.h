/**
 * @file syn_encoder.h
 * @brief Quadrature rotary encoder decoder.
 *
 * Decodes A/B quadrature signals from rotary encoders. Supports:
 * - Direction detection (CW/CCW)
 * - Accumulated position
 * - Delta since last read
 * - Optional push-button (use syn_button for debouncing)
 *
 * Call `syn_encoder_update()` from a timer ISR or fast loop
 * (≥4× the maximum encoder frequency).
 *
 * @par Usage
 * @code
 *   SYN_Encoder enc;
 *   syn_encoder_init(&enc, PIN_A, PIN_B);
 *
 *   // In timer ISR or fast loop:
 *   syn_encoder_update(&enc);
 *
 *   // In main loop:
 *   int32_t delta = syn_encoder_get_delta(&enc);
 *   if (delta != 0) {
 *       volume += delta;
 *   }
 * @endcode
 * @ingroup syn_io
 */

#ifndef SYN_ENCODER_H
#define SYN_ENCODER_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"
#include "../dsp/syn_signal.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Encoder direction ──────────────────────────────────────────────────── */

/** @brief Encoder rotation direction. */
typedef enum {
    SYN_ENCODER_NONE = 0,   /**< No rotation detected             */
    SYN_ENCODER_CW   = 1,   /**< Clockwise                        */
    SYN_ENCODER_CCW   = -1, /**< Counter-clockwise                */
} SYN_EncoderDir;

/* ── Encoder descriptor ─────────────────────────────────────────────────── */

/** @brief Quadrature encoder instance — pins, position, delta. */
typedef struct {
    SYN_GPIO_Pin  pin_a;            /**< Channel A GPIO pin              */
    SYN_GPIO_Pin  pin_b;            /**< Channel B GPIO pin              */

    int32_t        position;    /**< Accumulated position (signed)        */
    int32_t        delta;       /**< Steps since last get_delta()         */
    uint8_t        last_state;  /**< Previous A/B state (2-bit)           */
    int8_t         last_dir;    /**< Last non-zero direction              */

    /* Optional: step multiplier (for encoders with detents every N states) */
    uint8_t        steps_per_detent; /**< Typically 1, 2, or 4            */
    int8_t         sub_count;        /**< Sub-step accumulator            */

    /* Statistics (optional) */
    SYN_Signal   *stats;            /**< If set, delta pushed on get_delta */
} SYN_Encoder;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize encoder.
 *
 * @param enc     Encoder instance.
 * @param pin_a   Channel A GPIO.
 * @param pin_b   Channel B GPIO.
 */
void syn_encoder_init(SYN_Encoder *enc,
                       SYN_GPIO_Pin pin_a,
                       SYN_GPIO_Pin pin_b);

/**
 * @brief Set steps-per-detent (default = 1).
 *
 * Many mechanical encoders generate 2 or 4 state changes per physical
 * detent. Setting this to 4 means @c position and @c delta only increment
 * once per detent rather than once per state change.
 *
 * @param enc  Encoder instance.
 * @param spd  Steps per detent (1, 2, or 4).
 */
void syn_encoder_set_steps_per_detent(SYN_Encoder *enc, uint8_t spd);

/**
 * @brief Sample pins and update position.
 *
 * Call from a timer ISR or fast polling loop. Must be called at
 * ≥4× the encoder's maximum rotation speed.
 *
 * @param enc  Encoder instance.
 */
void syn_encoder_update(SYN_Encoder *enc);

/**
 * @brief Get accumulated delta since last call and reset it.
 *
 * @param enc  Encoder instance.
 * @return Number of steps (positive = CW, negative = CCW).
 */
int32_t syn_encoder_get_delta(SYN_Encoder *enc);

/**
 * @brief Get absolute position.
 * @param enc  Encoder.
 * @return Accumulated position.
 */
static inline int32_t syn_encoder_position(const SYN_Encoder *enc)
{
    return enc->position;
}

/**
 * @brief Set absolute position.
 * @param enc  Encoder.
 * @param pos  New position value.
 */
static inline void syn_encoder_set_position(SYN_Encoder *enc, int32_t pos)
{
    enc->position = pos;
}

/**
 * @brief Get last direction.
 * @param enc  Encoder.
 * @return Last detected direction.
 */
static inline SYN_EncoderDir syn_encoder_direction(const SYN_Encoder *enc)
{
    return (SYN_EncoderDir)enc->last_dir;
}

/**
 * @brief Attach a signal statistics window for velocity tracking.
 *
 * Each call to syn_encoder_get_delta() pushes the delta value
 * into the signal window, giving you velocity min/max/mean/variance.
 *
 * @param enc    Encoder.
 * @param stats  Initialized SYN_Signal, or NULL to detach.
 */
void syn_encoder_set_stats(SYN_Encoder *enc, SYN_Signal *stats);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ENCODER_H */
