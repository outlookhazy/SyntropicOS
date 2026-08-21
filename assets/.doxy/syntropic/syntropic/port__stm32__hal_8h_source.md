

# File port\_stm32\_hal.h

[**File List**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**stm32\_hal**](dir_beb8b7258a6e20be6ee3b2db46ef4785.md) **>** [**port\_stm32\_hal.h**](port__stm32__hal_8h.md)

[Go to the documentation of this file](port__stm32__hal_8h.md)


```C++

#ifndef PORT_STM32_HAL_H
#define PORT_STM32_HAL_H

#include "syntropic/drivers/syn_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_PORT_STM32_PIN(gpio_port, gpio_pin)                                           \
    SYN_GPIO_PIN((uint8_t)(((uintptr_t)(gpio_port) - (uintptr_t)GPIOA) / 0x0400UL),       \
                 (uint8_t)(((uint32_t)(gpio_pin) == 0U) ? 0U                              \
                           : (((uint32_t)(gpio_pin) & ((uint32_t)(gpio_pin) - 1U)) == 0U) \
                               ? (uint32_t)__builtin_ctz((uint32_t)(gpio_pin))            \
                               : (uint32_t)(gpio_pin)))

SYN_Status syn_port_stm32_register_uart(SYN_UARTInstance instance, void *huart);

SYN_Status syn_port_stm32_register_cryp(void *hcryp);

#ifdef __cplusplus
}
#endif

#endif /* PORT_STM32_HAL_H */
```


