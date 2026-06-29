/**
 * @file syn_soft_pwm.h
 * @brief Software PWM on any GPIO pin.
 *
 * When hardware PWM isn't available or you've run out of channels, this
 * module generates PWM by toggling GPIO in a high-frequency tick ISR.
 *
 * @par Usage
 * @code
 *   static SYN_SoftPWM pwm;
 *   syn_soft_pwm_init(&pwm, LED_PIN, 100);  // 100 steps resolution
 *   syn_soft_pwm_set_duty(&pwm, 75);         // 75% duty cycle
 *
 *   // Call from a timer ISR at (desired_freq * resolution) Hz:
 *   void TIM_IRQHandler(void) {
 *       syn_soft_pwm_tick(&pwm);
 *   }
 * @endcode
 * @ingroup syn_drivers
 */

#ifndef SYN_SOFT_PWM_H
#define SYN_SOFT_PWM_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Soft PWM descriptor ────────────────────────────────────────────────── */

/** @brief Software PWM channel descriptor. */
typedef struct {
    SYN_GPIO_Pin  pin;           /**< Target GPIO pin identifier */
    uint16_t       resolution;   /**< Total steps per period (e.g., 100)  */
    uint16_t       duty;         /**< Duty cycle (0 to resolution)        */
    uint16_t       counter;      /**< Current phase counter               */
    bool           active_high;  /**< true = GPIO high during on-phase    */
} SYN_SoftPWM;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a soft PWM channel.
 *
 * @param pwm         PWM instance to initialize.
 * @param pin         GPIO pin to drive.
 * @param resolution  Number of steps per PWM period (e.g., 100 or 256).
 *                    Higher = finer duty control, but requires faster tick.
 */
void syn_soft_pwm_init(SYN_SoftPWM *pwm, SYN_GPIO_Pin pin,
                        uint16_t resolution);

/**
 * @brief Set the duty cycle.
 *
 * @param pwm   PWM instance.
 * @param duty  Duty cycle value (0 = off, resolution = 100% on).
 */
void syn_soft_pwm_set_duty(SYN_SoftPWM *pwm, uint16_t duty);

/**
 * @brief Set duty cycle as a percentage (0–100).
 *
 * @param pwm     PWM instance.
 * @param percent Duty cycle percentage (0 to 100).
 */
void syn_soft_pwm_set_percent(SYN_SoftPWM *pwm, uint8_t percent);

/**
 * @brief Advance the PWM phase by one tick.
 *
 * Call this from a timer ISR or high-frequency polling loop.
 * The tick rate determines the PWM frequency:
 *   PWM_freq = tick_rate / resolution
 *
 * For example: 10 kHz tick with resolution=100 → 100 Hz PWM.
 *
 * @param pwm PWM instance.
 */
void syn_soft_pwm_tick(SYN_SoftPWM *pwm);

/**
 * @brief Service an array of PWM channels in one call.
 *
 * @param channels Array of PWM instances.
 * @param count    Number of channels in the array.
 */
void syn_soft_pwm_service(SYN_SoftPWM *channels, size_t count);

/**
 * @brief Get the current duty cycle value.
 *
 * @param pwm PWM instance.
 * @return Current duty cycle raw value.
 */
static inline uint16_t syn_soft_pwm_get_duty(const SYN_SoftPWM *pwm)
{
    return pwm->duty;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_SOFT_PWM_H */
