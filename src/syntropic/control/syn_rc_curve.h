/**
 * @file syn_rc_curve.h
 * @brief Zero-Heap RC Stick Exponential (Expo) & Deadband Curve Mapper.
 *
 * Provides stick input response shaping for drones/RC models:
 * - Deadband filtering: Ignores micro-stick jitter around neutral center (1500 us +/- deadband).
 * - Exponential curve: y = (1 - expo) * x + expo * x^3 (softens center
 * stick precision).
 * - Dual Rate: Scales output range (0..100%).
 *
 * All operations use Q16.16 fixed-point arithmetic. Zero float / zero heap.
 */

#ifndef SYN_RC_CURVE_H
#define SYN_RC_CURVE_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/util/syn_qmath.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** RC Curve Configuration. */
typedef struct {
    uint16_t
        deadband_us; /**< Center deadband width in us (e.g., 10 us -> [1490, 1510] deadband). */
    q16_t expo;      /**< Exponential factor in Q16.16 (0.0 = linear, 1.0 = full cubic). */
    q16_t dual_rate; /**< Dual rate scaling factor in Q16.16 (1.0 = 100% full travel). */
} SYN_RCCurve_Config;

/**
 * @brief Apply deadband, exponential response, and dual-rate scaling to raw RC channel (1000..2000
 * us).
 *
 * @param input_us Raw input pulse width in microseconds (1000..2000 us).
 * @param config   Pointer to RC curve configuration.
 * @return Scaled & shaped output in microseconds (clamped 1000..2000 us).
 */
uint16_t syn_rc_curve_apply(uint16_t input_us, const SYN_RCCurve_Config *config);

#ifdef __cplusplus
}
#endif

#endif /* SYN_RC_CURVE_H */
