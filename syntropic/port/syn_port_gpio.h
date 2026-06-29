/**
 * @file syn_port_gpio.h
 * @brief GPIO port interface — functions the user must implement.
 *
 * To use SyntropicOS GPIO functionality, provide strong definitions of every
 * function declared here in your platform-specific source file.
 *
 * If you compile syn_port_stubs.c, any function you forget to implement
 * will link to a weak stub that fires an assert, making integration
 * mistakes visible at runtime.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_GPIO_H
#define SYN_PORT_GPIO_H

#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a GPIO pin with the given mode.
 *
 * @param pin   Platform-specific pin identifier.
 * @param mode  Desired pin mode (input, output, etc.).
 * @return SYN_OK on success, or an error code.
 */
SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode);

/**
 * @brief De-initialize a GPIO pin, returning it to its reset state.
 *
 * @param pin   Pin to de-initialize.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_gpio_deinit(SYN_GPIO_Pin pin);

/**
 * @brief Write a logical state to an output pin.
 *
 * @param pin   Pin to write.
 * @param state SYN_GPIO_HIGH or SYN_GPIO_LOW.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state);

/**
 * @brief Read the current logical state of a pin.
 *
 * @param pin   Pin to read.
 * @return SYN_GPIO_HIGH or SYN_GPIO_LOW.
 */
SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin);

/**
 * @brief Toggle the state of an output pin.
 *
 * @param pin   Pin to toggle.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_gpio_toggle(SYN_GPIO_Pin pin);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_GPIO_H */
