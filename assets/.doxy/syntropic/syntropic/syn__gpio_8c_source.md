

# File syn\_gpio.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_gpio.c**](syn__gpio_8c.md)

[Go to the documentation of this file](syn__gpio_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_GPIO) || SYN_USE_GPIO

#include "syn_gpio.h"
#include "../util/syn_assert.h"

SYN_Status syn_gpio_init_multiple(const SYN_GPIO_Pin *pins,
                                    size_t count,
                                    SYN_GPIO_Mode mode)
{
    SYN_ASSERT(pins != NULL || count == 0);

    for (size_t i = 0; i < count; i++) {
        SYN_Status status = syn_port_gpio_init(pins[i], mode);
        if (status != SYN_OK) {
            return status;
        }
    }
    return SYN_OK;
}

SYN_Status syn_gpio_write_multiple(const SYN_GPIO_Pin *pins,
                                     size_t count,
                                     SYN_GPIO_State state)
{
    SYN_ASSERT(pins != NULL || count == 0);

    for (size_t i = 0; i < count; i++) {
        SYN_Status status = syn_port_gpio_write(pins[i], state);
        if (status != SYN_OK) {
            return status;
        }
    }
    return SYN_OK;
}

#endif /* SYN_USE_GPIO */
```


