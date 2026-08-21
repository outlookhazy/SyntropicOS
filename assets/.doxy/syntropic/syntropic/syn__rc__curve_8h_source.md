

# File syn\_rc\_curve.h

[**File List**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_rc\_curve.h**](syn__rc__curve_8h.md)

[Go to the documentation of this file](syn__rc__curve_8h.md)


```C++

#ifndef SYN_RC_CURVE_H
#define SYN_RC_CURVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/util/syn_qmath.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint16_t
        deadband_us; 
    q16_t expo;      
    q16_t dual_rate; 
} SYN_RCCurve_Config;

uint16_t syn_rc_curve_apply(uint16_t input_us, const SYN_RCCurve_Config *config);

#ifdef __cplusplus
}
#endif

#endif /* SYN_RC_CURVE_H */
```


