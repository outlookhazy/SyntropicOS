#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_SOFT_PWM) || SYN_USE_SOFT_PWM

/**
 * @file syn_soft_pwm.c
 * @brief Software PWM implementation.
 */

#include "syn_soft_pwm.h"
#include "../util/syn_assert.h"

#include <string.h>

void syn_soft_pwm_init(SYN_SoftPWM *pwm, SYN_GPIO_Pin pin,
                        uint16_t resolution)
{
    SYN_ASSERT(pwm != NULL);
    SYN_ASSERT(resolution > 0);

    memset(pwm, 0, sizeof(*pwm));
    pwm->pin         = pin;
    pwm->resolution  = resolution;
    pwm->duty        = 0;
    pwm->counter     = 0;
    pwm->active_high = true;

    syn_port_gpio_write(pin, SYN_GPIO_LOW);
}

void syn_soft_pwm_set_duty(SYN_SoftPWM *pwm, uint16_t duty)
{
    SYN_ASSERT(pwm != NULL);

    if (duty > pwm->resolution) {
        duty = pwm->resolution;
    }
    pwm->duty = duty;
}

void syn_soft_pwm_set_percent(SYN_SoftPWM *pwm, uint8_t percent)
{
    SYN_ASSERT(pwm != NULL);

    if (percent > 100) percent = 100;
    uint16_t duty = (uint16_t)(((uint32_t)percent * pwm->resolution) / 100u);
    pwm->duty = duty;
}

void syn_soft_pwm_tick(SYN_SoftPWM *pwm)
{
    SYN_ASSERT(pwm != NULL);

    bool on;

    if (pwm->duty == 0) {
        on = false;
    } else if (pwm->duty >= pwm->resolution) {
        on = true;
    } else {
        on = (pwm->counter < pwm->duty);
    }

    SYN_GPIO_State level;
    if (pwm->active_high) {
        level = on ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
    } else {
        level = on ? SYN_GPIO_LOW : SYN_GPIO_HIGH;
    }
    syn_port_gpio_write(pwm->pin, level);

    pwm->counter++;
    if (pwm->counter >= pwm->resolution) {
        pwm->counter = 0;
    }
}

void syn_soft_pwm_service(SYN_SoftPWM *channels, size_t count)
{
    SYN_ASSERT(channels != NULL || count == 0);

    for (size_t i = 0; i < count; i++) {
        syn_soft_pwm_tick(&channels[i]);
    }
}

#endif /* SYN_USE_SOFT_PWM */
