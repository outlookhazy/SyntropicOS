

# File syn\_pwm.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_pwm.c**](syn__pwm_8c.md)

[Go to the documentation of this file](syn__pwm_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_PWM) || SYN_USE_PWM

#include "../util/syn_assert.h"
#include "syn_pwm.h"

SYN_Status syn_pwm_init(SYN_PWM *pwm, uint8_t channel, uint32_t freq_hz)
{
    if (pwm == NULL || freq_hz == 0) {
        return SYN_INVALID_PARAM;
    }

    pwm->channel = channel;
    pwm->freq_hz = freq_hz;

    return syn_port_pwm_init(channel, freq_hz);
}

SYN_Status syn_pwm_set_duty(const SYN_PWM *pwm, uint8_t duty_pct)
{
    if (pwm == NULL) {
        return SYN_INVALID_PARAM;
    }

    if (duty_pct > 100) {
        duty_pct = 100;
    }

    syn_port_pwm_set_duty(pwm->channel, duty_pct);
    return SYN_OK;
}

SYN_Status syn_pwm_set_duty_raw(const SYN_PWM *pwm, uint16_t duty_u16)
{
    if (pwm == NULL) {
        return SYN_INVALID_PARAM;
    }

    syn_port_pwm_set_duty_raw(pwm->channel, duty_u16);
    return SYN_OK;
}

SYN_Status syn_pwm_set_freq(SYN_PWM *pwm, uint32_t freq_hz)
{
    if (pwm == NULL || freq_hz == 0) {
        return SYN_INVALID_PARAM;
    }

    pwm->freq_hz = freq_hz;
    syn_port_pwm_set_freq(pwm->channel, freq_hz);
    return SYN_OK;
}

SYN_Status syn_pwm_enable(const SYN_PWM *pwm, bool enable)
{
    if (pwm == NULL) {
        return SYN_INVALID_PARAM;
    }

    syn_port_pwm_enable(pwm->channel, enable);
    return SYN_OK;
}

#endif /* SYN_USE_PWM */
```


