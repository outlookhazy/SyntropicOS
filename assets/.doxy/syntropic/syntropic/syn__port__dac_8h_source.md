

# File syn\_port\_dac.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_dac.h**](syn__port__dac_8h.md)

[Go to the documentation of this file](syn__port__dac_8h.md)


```C++

#ifndef SYN_PORT_DAC_H
#define SYN_PORT_DAC_H

#include <stdint.h>
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_dac_init(uint8_t channel);

SYN_Status syn_port_dac_write(uint8_t channel, uint16_t value);

uint8_t syn_port_dac_resolution(void);

uint16_t syn_port_dac_reference_mv(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_DAC_H */
```


