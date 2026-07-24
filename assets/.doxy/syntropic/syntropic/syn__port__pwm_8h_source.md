

# File syn\_port\_pwm.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_pwm.h**](syn__port__pwm_8h.md)

[Go to the documentation of this file](syn__port__pwm_8h.md)


```C++

#ifndef SYN_PORT_PWM_H
#define SYN_PORT_PWM_H

#include "../common/syn_defs.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_pwm_init(uint8_t channel, uint32_t freq_hz);

void syn_port_pwm_set_duty(uint8_t channel, uint8_t duty_pct);

void syn_port_pwm_set_duty_raw(uint8_t channel, uint16_t duty_u16);

void syn_port_pwm_enable(uint8_t channel, bool enable);

void syn_port_pwm_set_freq(uint8_t channel, uint32_t freq_hz);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_PWM_H */
```


