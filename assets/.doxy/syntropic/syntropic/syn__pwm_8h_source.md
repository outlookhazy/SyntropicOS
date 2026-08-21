

# File syn\_pwm.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_pwm.h**](syn__pwm_8h.md)

[Go to the documentation of this file](syn__pwm_8h.md)


```C++

#ifndef SYN_PWM_H
#define SYN_PWM_H

#include "../common/syn_defs.h"
#include "../port/syn_port_pwm.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t channel;  
    uint32_t freq_hz; 
} SYN_PWM;

SYN_Status syn_pwm_init(SYN_PWM *pwm, uint8_t channel, uint32_t freq_hz);

SYN_Status syn_pwm_set_duty(const SYN_PWM *pwm, uint8_t duty_pct);

SYN_Status syn_pwm_set_duty_raw(const SYN_PWM *pwm, uint16_t duty_u16);

SYN_Status syn_pwm_set_freq(SYN_PWM *pwm, uint32_t freq_hz);

SYN_Status syn_pwm_enable(const SYN_PWM *pwm, bool enable);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PWM_H */
```


