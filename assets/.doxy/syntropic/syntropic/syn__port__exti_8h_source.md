

# File syn\_port\_exti.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_exti.h**](syn__port__exti_8h.md)

[Go to the documentation of this file](syn__port__exti_8h.md)


```C++

#ifndef SYN_PORT_EXTI_H
#define SYN_PORT_EXTI_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_EXTI_RISING  = 0,   
    SYN_EXTI_FALLING = 1,   
    SYN_EXTI_BOTH    = 2,   
} SYN_EXTI_Edge;

SYN_Status syn_port_exti_configure(SYN_GPIO_Pin pin, SYN_EXTI_Edge edge);

void syn_port_exti_enable(SYN_GPIO_Pin pin);

void syn_port_exti_disable(SYN_GPIO_Pin pin);

void syn_port_exti_clear_pending(SYN_GPIO_Pin pin);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_EXTI_H */
```


