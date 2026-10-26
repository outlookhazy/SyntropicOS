

# File syn\_dshot.h

[**File List**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_dshot.h**](syn__dshot_8h.md)

[Go to the documentation of this file](syn__dshot_8h.md)


```C++

#ifndef SYN_DSHOT_H
#define SYN_DSHOT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    SYN_DSHOT150 = 150, 
    SYN_DSHOT300 = 300, 
    SYN_DSHOT600 = 600  
} SYN_DShot_Speed;

typedef struct {
    uint16_t throttle;  
    bool telemetry;     
    uint8_t crc;        
    uint16_t raw_frame; 
} SYN_DShot_Packet;

uint8_t syn_dshot_calc_crc(uint16_t payload_12bit);

SYN_Status syn_dshot_encode(uint16_t throttle, bool telemetry, SYN_DShot_Packet *packet);

uint16_t syn_dshot_us_to_throttle(uint16_t us);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DSHOT_H */
```


