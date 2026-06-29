/**
 * @file syn_hysteresis.h
 * @brief Threshold comparator with hysteresis (deadband).
 *
 * Prevents rapid on/off cycling when a signal hovers near a threshold.
 * Classic use cases: thermostat, battery low warning, tank level control.
 *
 * Header-only — zero code size if unused.
 *
 * @par Usage
 * @code
 *   SYN_Hysteresis hyst;
 *   syn_hyst_init(&hyst, 1000, 50, false);  // threshold=1000, band=±50
 *
 *   // In sensor loop:
 *   if (syn_hyst_update(&hyst, temperature)) {
 *       // crossed high threshold (1050) going up
 *       heater_off();
 *   } else if (!syn_hyst_state(&hyst)) {
 *       // crossed low threshold (950) going down
 *       heater_on();
 *   }
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_HYSTERESIS_H
#define SYN_HYSTERESIS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Hysteresis comparator with configurable dead-band. */
typedef struct {
    int32_t  threshold;     /**< Center threshold value                   */
    int32_t  band;          /**< Half-width of the deadband               */
    bool     state;         /**< Current output state (high/low)          */
} SYN_Hysteresis;

/**
 * @brief Initialize a hysteresis comparator.
 *
 * @param h          Instance.
 * @param threshold  Center threshold value.
 * @param band       Half-width of hysteresis band. The high trip point
 *                   is (threshold + band) and the low trip point is
 *                   (threshold - band).
 * @param initial    Initial output state.
 */
static inline void syn_hyst_init(SYN_Hysteresis *h,
                                  int32_t threshold,
                                  int32_t band,
                                  bool initial)
{
    h->threshold = threshold;
    h->band      = band;
    h->state     = initial;
}

/**
 * @brief Feed a new value and get the output state.
 *
 * @param h      Hysteresis instance.
 * @param value  New input value.
 * @return true if the value crossed the high trip point (threshold + band)
 *         while state was low, or remains above the low trip point.
 */
static inline bool syn_hyst_update(SYN_Hysteresis *h, int32_t value)
{
    if (h->state) {
        /* Currently high — switch low only if value drops below (threshold - band) */
        if (value < h->threshold - h->band) {
            h->state = false;
        }
    } else {
        /* Currently low — switch high only if value rises above (threshold + band) */
        if (value > h->threshold + h->band) {
            h->state = true;
        }
    }
    return h->state;
}

/**
 * @brief Get the current output state without updating.
 * @param h  Hysteresis instance.
 * @return Current state.
 */
static inline bool syn_hyst_state(const SYN_Hysteresis *h)
{
    return h->state;
}

/**
 * @brief Update the threshold and band at runtime.
 * @param h          Hysteresis instance.
 * @param threshold  New center threshold.
 * @param band       New half-width of dead-band.
 */
static inline void syn_hyst_set(SYN_Hysteresis *h,
                                 int32_t threshold, int32_t band)
{
    h->threshold = threshold;
    h->band      = band;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_HYSTERESIS_H */
