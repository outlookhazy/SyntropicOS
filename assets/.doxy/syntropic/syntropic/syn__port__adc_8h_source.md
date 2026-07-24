

# File syn\_port\_adc.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_adc.h**](syn__port__adc_8h.md)

[Go to the documentation of this file](syn__port__adc_8h.md)


```C++

#ifndef SYN_PORT_ADC_H
#define SYN_PORT_ADC_H

#include "../common/syn_defs.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_adc_init(uint8_t channel);

uint16_t syn_port_adc_read(uint8_t channel);

uint8_t syn_port_adc_resolution(void);

uint16_t syn_port_adc_reference_mv(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_ADC_H */
```


