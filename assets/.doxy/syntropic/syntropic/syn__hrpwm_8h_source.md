

# File syn\_hrpwm.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_hrpwm.h**](syn__hrpwm_8h.md)

[Go to the documentation of this file](syn__hrpwm_8h.md)


```C++

#ifndef SYN_HRPWM_H
#define SYN_HRPWM_H

#include "../common/syn_defs.h"
#include "../port/syn_port_hrpwm.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t channel;    
    uint32_t freq_hz;   
    uint16_t rise_ns;   
    uint16_t fall_ns;   
    uint16_t phase_deg; 
    bool enabled;       
} SYN_HRPWM;

SYN_Status syn_hrpwm_init(SYN_HRPWM *hrpwm, uint8_t channel, uint32_t freq_hz);

SYN_Status syn_hrpwm_set_duty_q16(const SYN_HRPWM *hrpwm, int32_t duty_q16);

SYN_Status syn_hrpwm_set_duty_float(const SYN_HRPWM *hrpwm, float duty_float);

SYN_Status syn_hrpwm_set_deadtime_ns(SYN_HRPWM *hrpwm, uint16_t rise_ns, uint16_t fall_ns);

SYN_Status syn_hrpwm_set_phase_deg(SYN_HRPWM *hrpwm, uint16_t phase_deg);

SYN_Status syn_hrpwm_bind_fault(const SYN_HRPWM *hrpwm, uint8_t fault_id);

SYN_Status syn_hrpwm_enable(SYN_HRPWM *hrpwm, bool enable);

#ifdef __cplusplus
}
#endif

#endif /* SYN_HRPWM_H */
```


