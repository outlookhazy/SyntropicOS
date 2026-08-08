

# File syn\_port\_comp.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_comp.h**](syn__port__comp_8h.md)

[Go to the documentation of this file](syn__port__comp_8h.md)


```C++

#ifndef SYN_PORT_COMP_H
#define SYN_PORT_COMP_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_COMP_INV_VREFINT_1_4 = 0, 
    SYN_COMP_INV_VREFINT_1_2 = 1, 
    SYN_COMP_INV_VREFINT_3_4 = 2, 
    SYN_COMP_INV_VREFINT = 3,     
    SYN_COMP_INV_DAC1_CH1 = 4,    
    SYN_COMP_INV_EXTERNAL = 5     
} SYN_COMP_InvertingInput;

SYN_Status syn_port_comp_init(uint8_t comp_id, SYN_COMP_InvertingInput inv_in);

bool syn_port_comp_read(uint8_t comp_id);

SYN_Status syn_port_comp_enable(uint8_t comp_id, bool enable);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_COMP_H */
```


