/**
 * @file syn_exti.h
 * @brief GPIO interrupt dispatcher — register callbacks per pin.
 *
 * Thin layer over the EXTI port. Registers a callback for each pin;
 * when the platform ISR fires, call syn_exti_irq_handler(pin) and
 * the dispatcher invokes the registered callback.
 *
 * Optionally integrates with the work queue for deferred processing
 * (recommended for non-trivial handlers).
 *
 * @par Usage (direct callback in ISR)
 * @code
 *   void on_button_press(SYN_GPIO_Pin pin, void *ctx) {
 *       // runs in ISR context — keep it fast!
 *       *(bool *)ctx = true;
 *   }
 *
 *   syn_exti_init();
 *   syn_exti_register(PIN_BUTTON, SYN_EXTI_FALLING,
 *                      on_button_press, &button_flag);
 *   syn_exti_enable(PIN_BUTTON);
 * @endcode
 *
 * @par Platform ISR glue (implement once per MCU)
 * @code
 *   // STM32 example:
 *   void EXTI0_IRQHandler(void) {
 *       if (EXTI->PR & (1 << 0)) {
 *           EXTI->PR = (1 << 0);  // clear
 *           syn_exti_irq_handler(0);
 *       }
 *   }
 * @endcode
 * @ingroup syn_drivers
 */

#ifndef SYN_EXTI_H
#define SYN_EXTI_H

#include "../common/syn_defs.h"
#include "../port/syn_port_exti.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

#ifndef SYN_EXTI_MAX_PINS
#define SYN_EXTI_MAX_PINS  16  /**< Max number of EXTI-enabled pins      */
#endif

/* ── Callback ───────────────────────────────────────────────────────────── */

/** EXTI callback — called from ISR context unless deferred. */
typedef void (*SYN_EXTI_Callback)(SYN_GPIO_Pin pin, void *ctx);

/* ── API ────────────────────────────────────────────────────────────────── */

/** Initialize the EXTI dispatcher. */
void syn_exti_init(void);

/**
 * @brief Register a callback for a pin interrupt.
 *
 * @param pin   GPIO pin.
 * @param edge  Trigger edge.
 * @param cb    Callback (runs in ISR context).
 * @param ctx   User context passed to callback.
 * @return SYN_OK, or SYN_ERROR if table full.
 */
SYN_Status syn_exti_register(SYN_GPIO_Pin pin, SYN_EXTI_Edge edge,
                                SYN_EXTI_Callback cb, void *ctx);

/**
 * @brief Unregister a pin callback.
 * @param pin  GPIO pin.
 */
void syn_exti_unregister(SYN_GPIO_Pin pin);

/**
 * @brief Enable interrupt for a registered pin.
 * @param pin  GPIO pin.
 */
void syn_exti_enable(SYN_GPIO_Pin pin);

/**
 * @brief Disable interrupt for a pin.
 * @param pin  GPIO pin.
 */
void syn_exti_disable(SYN_GPIO_Pin pin);

/**
 * @brief IRQ handler — call from platform ISR.
 *
 * Looks up the registered callback for @p pin and invokes it.
 * This is the glue between hardware ISR and the dispatcher.
 *
 * @param pin  GPIO pin that triggered the interrupt.
 */
void syn_exti_irq_handler(SYN_GPIO_Pin pin);

/**
 * @brief Get the number of registered callbacks.
 * @return Registration count.
 */
size_t syn_exti_count(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_EXTI_H */
