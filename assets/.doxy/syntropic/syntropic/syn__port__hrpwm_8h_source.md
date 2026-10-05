

# File syn\_port\_hrpwm.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_hrpwm.h**](syn__port__hrpwm_8h.md)

[Go to the documentation of this file](syn__port__hrpwm_8h.md)


```C++

#ifndef SYN_PORT_HRPWM_H
#define SYN_PORT_HRPWM_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_hrpwm_init(uint8_t channel, uint32_t freq_hz);

SYN_Status syn_port_hrpwm_set_duty_q16(uint8_t channel, int32_t duty_q16);

SYN_Status syn_port_hrpwm_set_deadtime_ns(uint8_t channel, uint16_t rise_ns, uint16_t fall_ns);

SYN_Status syn_port_hrpwm_set_phase_deg(uint8_t channel, uint16_t phase_deg);

SYN_Status syn_port_hrpwm_enable_fault(uint8_t channel, uint8_t fault_id, bool enable);

SYN_Status syn_port_hrpwm_enable(uint8_t channel, bool enable);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_HRPWM_H */
```


