

# File syn\_dshot\_telemetry.h

[**File List**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_dshot\_telemetry.h**](syn__dshot__telemetry_8h.md)

[Go to the documentation of this file](syn__dshot__telemetry_8h.md)


```C++

#ifndef SYN_DSHOT_TELEMETRY_H
#define SYN_DSHOT_TELEMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t period_us; 
    uint32_t erpm;      
    uint32_t rpm;       
    bool valid;         
} SYN_DShot_Telemetry;

SYN_Status syn_dshot_decode_gcr_20bit(uint32_t gcr_20bit, uint16_t *payload_out);

SYN_Status syn_dshot_parse_telemetry(uint32_t gcr_20bit, uint8_t pole_pairs,
                                     SYN_DShot_Telemetry *telemetry);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DSHOT_TELEMETRY_H */
```


