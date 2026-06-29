/**
 * @file syn_port_exti.h
 * @brief GPIO interrupt (EXTI) port interface — implement for your platform.
 *
 * Provides pin-change interrupt registration. The platform layer
 * manages the hardware (NVIC, EXTI lines, etc.) and calls back
 * into the syn_exti dispatcher.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_EXTI_H
#define SYN_PORT_EXTI_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Interrupt trigger edge. */
typedef enum {
    SYN_EXTI_RISING  = 0,   /**< Trigger on rising edge                 */
    SYN_EXTI_FALLING = 1,   /**< Trigger on falling edge                */
    SYN_EXTI_BOTH    = 2,   /**< Trigger on both edges                  */
} SYN_EXTI_Edge;

/**
 * @brief Configure a pin for interrupt generation.
 *
 * @param pin    GPIO pin number.
 * @param edge   Trigger edge.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_exti_configure(SYN_GPIO_Pin pin, SYN_EXTI_Edge edge);

/**
 * @brief Enable interrupt on a configured pin.
 * @param pin  GPIO pin.
 */
void syn_port_exti_enable(SYN_GPIO_Pin pin);

/**
 * @brief Disable interrupt on a pin (without deconfiguring).
 * @param pin  GPIO pin.
 */
void syn_port_exti_disable(SYN_GPIO_Pin pin);

/**
 * @brief Clear pending interrupt flag for a pin.
 * @param pin  GPIO pin.
 */
void syn_port_exti_clear_pending(SYN_GPIO_Pin pin);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_EXTI_H */
