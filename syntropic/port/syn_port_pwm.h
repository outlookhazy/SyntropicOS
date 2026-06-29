/**
 * @file syn_port_pwm.h
 * @brief Hardware PWM port interface — implement these for your platform.
 *
 * Provides channel-based PWM control. Each channel maps to a physical
 * timer/output pin on your MCU.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_PWM_H
#define SYN_PORT_PWM_H

#include "../common/syn_defs.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a PWM channel.
 *
 * @param channel   PWM channel number.
 * @param freq_hz   PWM frequency in Hz.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_pwm_init(uint8_t channel, uint32_t freq_hz);

/**
 * @brief Set PWM duty cycle.
 *
 * @param channel   PWM channel number.
 * @param duty_pct  Duty cycle 0–100 (percent).
 */
void syn_port_pwm_set_duty(uint8_t channel, uint8_t duty_pct);

/**
 * @brief Set PWM duty cycle with fine resolution.
 *
 * @param channel     PWM channel number.
 * @param duty_u16    Duty cycle 0–65535 (0 = off, 65535 = 100%).
 */
void syn_port_pwm_set_duty_raw(uint8_t channel, uint16_t duty_u16);

/**
 * @brief Enable/disable PWM output.
 * @param channel  PWM channel index.
 * @param enable   true to enable, false to disable.
 */
void syn_port_pwm_enable(uint8_t channel, bool enable);

/**
 * @brief Set PWM frequency (runtime change).
 *
 * @param channel   PWM channel number.
 * @param freq_hz   New frequency in Hz.
 */
void syn_port_pwm_set_freq(uint8_t channel, uint32_t freq_hz);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_PWM_H */
