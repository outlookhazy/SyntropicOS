

# File syn\_gpio.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_gpio.h**](syn__gpio_8h.md)

[Go to the documentation of this file](syn__gpio_8h.md)


```C++

#ifndef SYN_GPIO_H
#define SYN_GPIO_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Convenience wrappers (inline pass-through to port layer) ─────────── */

static inline SYN_Status syn_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    return syn_port_gpio_init(pin, mode);
}

static inline SYN_Status syn_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    return syn_port_gpio_write(pin, state);
}

static inline SYN_GPIO_State syn_gpio_read(SYN_GPIO_Pin pin)
{
    return syn_port_gpio_read(pin);
}

static inline SYN_Status syn_gpio_toggle(SYN_GPIO_Pin pin)
{
    return syn_port_gpio_toggle(pin);
}

/* ── Extended API (implemented in syn_gpio.c) ────────────────────────── */

SYN_Status syn_gpio_init_multiple(const SYN_GPIO_Pin *pins,
                                    size_t count,
                                    SYN_GPIO_Mode mode);

SYN_Status syn_gpio_write_multiple(const SYN_GPIO_Pin *pins,
                                     size_t count,
                                     SYN_GPIO_State state);

#ifdef __cplusplus
}
#endif

#endif /* SYN_GPIO_H */
```


