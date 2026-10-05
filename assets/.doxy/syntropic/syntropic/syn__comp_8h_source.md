

# File syn\_comp.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_comp.h**](syn__comp_8h.md)

[Go to the documentation of this file](syn__comp_8h.md)


```C++

#ifndef SYN_COMP_H
#define SYN_COMP_H

#include "../common/syn_defs.h"
#include "../port/syn_port_comp.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t comp_id;                
    SYN_COMP_InvertingInput inv_in; 
    bool enabled;                   
} SYN_COMP;

SYN_Status syn_comp_init(SYN_COMP *comp, uint8_t comp_id, SYN_COMP_InvertingInput inv_in);

bool syn_comp_read(const SYN_COMP *comp);

SYN_Status syn_comp_enable(SYN_COMP *comp, bool enable);

#ifdef __cplusplus
}
#endif

#endif /* SYN_COMP_H */
```


