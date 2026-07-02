

# File syn\_soft\_pwm.h

[**File List**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_soft\_pwm.h**](syn__soft__pwm_8h.md)

[Go to the documentation of this file](syn__soft__pwm_8h.md)


```C++

#ifndef SYN_SOFT_PWM_H
#define SYN_SOFT_PWM_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Soft PWM descriptor ────────────────────────────────────────────────── */

typedef struct {
    SYN_GPIO_Pin  pin;           
    uint16_t       resolution;   
    uint16_t       duty;         
    uint16_t       counter;      
    bool           active_high;  
} SYN_SoftPWM;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_soft_pwm_init(SYN_SoftPWM *pwm, SYN_GPIO_Pin pin,
                        uint16_t resolution);

void syn_soft_pwm_set_duty(SYN_SoftPWM *pwm, uint16_t duty);

void syn_soft_pwm_set_percent(SYN_SoftPWM *pwm, uint8_t percent);

void syn_soft_pwm_tick(SYN_SoftPWM *pwm);

void syn_soft_pwm_service(SYN_SoftPWM *channels, size_t count);

static inline uint16_t syn_soft_pwm_get_duty(const SYN_SoftPWM *pwm)
{
    return pwm->duty;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_SOFT_PWM_H */
```


