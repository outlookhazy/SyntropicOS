

# File syn\_port\_gpio.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_gpio.h**](syn__port__gpio_8h.md)

[Go to the documentation of this file](syn__port__gpio_8h.md)


```C++

#ifndef SYN_PORT_GPIO_H
#define SYN_PORT_GPIO_H

#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode);

SYN_Status syn_port_gpio_deinit(SYN_GPIO_Pin pin);

SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state);

SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin);

SYN_Status syn_port_gpio_toggle(SYN_GPIO_Pin pin);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_GPIO_H */
```


