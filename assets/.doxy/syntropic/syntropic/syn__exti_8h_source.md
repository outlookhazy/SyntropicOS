

# File syn\_exti.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_exti.h**](syn__exti_8h.md)

[Go to the documentation of this file](syn__exti_8h.md)


```C++

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
#define SYN_EXTI_MAX_PINS  16  
#endif

/* ── Callback ───────────────────────────────────────────────────────────── */

typedef void (*SYN_EXTI_Callback)(SYN_GPIO_Pin pin, void *ctx);

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_exti_init(void);

SYN_Status syn_exti_register(SYN_GPIO_Pin pin, SYN_EXTI_Edge edge,
                                SYN_EXTI_Callback cb, void *ctx);

void syn_exti_unregister(SYN_GPIO_Pin pin);

void syn_exti_enable(SYN_GPIO_Pin pin);

void syn_exti_disable(SYN_GPIO_Pin pin);

void syn_exti_irq_handler(SYN_GPIO_Pin pin);

size_t syn_exti_count(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_EXTI_H */
```


