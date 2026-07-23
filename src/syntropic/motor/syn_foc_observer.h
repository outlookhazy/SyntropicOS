/**
 * @file syn_foc_observer.h
 * @brief Sensorless FOC Rotor Position & Speed Sliding Mode Observer (SMO) (Q16.16 fixed-point).
 */

#ifndef SYN_FOC_OBSERVER_H
#define SYN_FOC_OBSERVER_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Sliding Mode Observer configuration parameters. */
typedef struct {
    q16_t R;       /**< Stator phase resistance in Ohms (Q16.16) */
    q16_t L;       /**< Stator phase inductance in Henries (Q16.16) */
    q16_t G;       /**< Sliding mode observer gain (Q16.16) */
    q16_t dt;      /**< Control loop sample period in seconds (Q16.16) */
    q16_t Kp_pll;  /**< Software PLL proportional gain (Q16.16) */
    q16_t Ki_pll;  /**< Software PLL integral gain (Q16.16) */
} SYN_FOCObserverConfig;

/** @brief Sliding Mode Observer state structure. */
typedef struct {
    SYN_FOCObserverConfig cfg; /**< Observer configuration */

    q16_t I_alpha_est; /**< Estimated alpha-axis phase current (Q16.16) */
    q16_t I_beta_est;  /**< Estimated beta-axis phase current (Q16.16)  */

    q16_t E_alpha_est; /**< Estimated alpha-axis back-EMF (Q16.16) */
    q16_t E_beta_est;  /**< Estimated beta-axis back-EMF (Q16.16)  */

    q16_t theta_e;     /**< Estimated rotor electrical angle in rad (Q16.16, 0 to 2pi) */
    q16_t omega_e;     /**< Estimated rotor electrical speed in rad/s (Q16.16)        */

    q16_t pll_i_accum; /**< Software PLL integral error accumulator                    */
} SYN_FOCObserver;

/**
 * @brief Initialize the sensorless FOC Sliding Mode Observer.
 * @param obs Pointer to observer instance.
 * @param cfg Observer configuration parameters.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL or invalid parameters.
 */
SYN_Status syn_foc_observer_init(SYN_FOCObserver *obs, const SYN_FOCObserverConfig *cfg);

/**
 * @brief Update observer with latest phase voltages and measured phase currents.
 *
 * @param obs     Observer instance.
 * @param V_alpha Applied alpha-axis voltage in Volts (Q16.16).
 * @param V_beta  Applied beta-axis voltage in Volts (Q16.16).
 * @param I_alpha Measured alpha-axis current in Amps (Q16.16).
 * @param I_beta  Measured beta-axis current in Amps (Q16.16).
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_foc_observer_update(SYN_FOCObserver *obs, q16_t V_alpha, q16_t V_beta,
                                    q16_t I_alpha, q16_t I_beta);

/**
 * @brief Get current estimated rotor electrical angle.
 * @param obs Observer instance.
 * @return Estimated angle in radians (Q16.16, 0 to 2pi).
 */
q16_t syn_foc_observer_get_angle(const SYN_FOCObserver *obs);

/**
 * @brief Get current estimated rotor electrical angular speed.
 * @param obs Observer instance.
 * @return Estimated angular speed in rad/s (Q16.16).
 */
q16_t syn_foc_observer_get_speed(const SYN_FOCObserver *obs);

/**
 * @brief Reset observer state to zero.
 * @param obs Observer instance.
 */
void syn_foc_observer_reset(SYN_FOCObserver *obs);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FOC_OBSERVER_H */
