/**
 * @file syn_gpio.h
 * @brief GPIO driver — high-level convenience functions.
 *
 * This module builds on the port layer (syn_port_gpio.h) and adds
 * convenience helpers like bulk operations. For simple use cases you can
 * call the port functions directly — this module is optional.
 * @ingroup syn_drivers
 */

#ifndef SYN_GPIO_H
#define SYN_GPIO_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Convenience wrappers (inline pass-through to port layer) ─────────── */

/**
 * @brief Initialize a GPIO pin.
 * @param pin   Pin identifier.
 * @param mode  Pin mode (input, output, etc.).
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    return syn_port_gpio_init(pin, mode);
}

/**
 * @brief Write a logical state to a pin.
 * @param pin    Pin identifier.
 * @param state  Logical state.
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    return syn_port_gpio_write(pin, state);
}

/**
 * @brief Read a pin's logical state.
 * @param pin  Pin identifier.
 * @return Logical state.
 */
static inline SYN_GPIO_State syn_gpio_read(SYN_GPIO_Pin pin)
{
    return syn_port_gpio_read(pin);
}

/**
 * @brief Toggle a pin.
 * @param pin  Pin identifier.
 * @return SYN_OK on success.
 */
static inline SYN_Status syn_gpio_toggle(SYN_GPIO_Pin pin)
{
    return syn_port_gpio_toggle(pin);
}

/* ── Extended API (implemented in syn_gpio.c) ────────────────────────── */

/**
 * @brief Initialize multiple pins with the same mode.
 *
 * @param pins   Array of pin identifiers.
 * @param count  Number of pins.
 * @param mode   Mode to apply to all pins.
 * @return SYN_OK if all succeeded, or the first error encountered.
 */
SYN_Status syn_gpio_init_multiple(const SYN_GPIO_Pin *pins,
                                    size_t count,
                                    SYN_GPIO_Mode mode);

/**
 * @brief Write the same state to multiple pins.
 *
 * @param pins   Array of pin identifiers.
 * @param count  Number of pins.
 * @param state  State to write.
 * @return SYN_OK if all succeeded, or the first error encountered.
 */
SYN_Status syn_gpio_write_multiple(const SYN_GPIO_Pin *pins,
                                     size_t count,
                                     SYN_GPIO_State state);

#ifdef __cplusplus
}
#endif

#endif /* SYN_GPIO_H */
