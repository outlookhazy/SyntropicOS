#ifndef MOCK_DRIVER_GPIO_H
#define MOCK_DRIVER_GPIO_H

#include <stdint.h>

typedef int gpio_num_t;
typedef int gpio_mode_t;
typedef int gpio_int_type_t;
typedef int gpio_pullup_t;
typedef int gpio_pulldown_t;

#define GPIO_MODE_INPUT 0
#define GPIO_MODE_OUTPUT 1
#define GPIO_PULLUP_ENABLE 1
#define GPIO_PULLUP_DISABLE 0
#define GPIO_PULLDOWN_ENABLE 1
#define GPIO_PULLDOWN_DISABLE 0
#define GPIO_INTR_DISABLE 0

typedef struct {
    uint64_t pin_bit_mask;
    gpio_mode_t mode;
    gpio_pullup_t pull_up_en;
    gpio_pulldown_t pull_down_en;
    gpio_int_type_t intr_type;
} gpio_config_t;

static inline int gpio_config(const gpio_config_t *c)
{
    (void)c;
    return 0;
}
static inline int gpio_set_level(gpio_num_t g, uint32_t l)
{
    (void)g;
    (void)l;
    return 0;
}
static inline int gpio_get_level(gpio_num_t g)
{
    (void)g;
    return 0;
}
static inline int gpio_reset_pin(gpio_num_t g)
{
    (void)g;
    return 0;
}

#endif /* MOCK_DRIVER_GPIO_H */
