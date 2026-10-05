

# File syn\_ibus.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ibus.h**](syn__ibus_8h.md)

[Go to the documentation of this file](syn__ibus_8h.md)


```C++

#ifndef SYN_IBUS_H
#define SYN_IBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_IBUS_FRAME_SIZE 32U   
#define SYN_IBUS_NUM_CHANNELS 14U 
#define SYN_IBUS_HEADER1 0x20U    
#define SYN_IBUS_HEADER2 0x40U    
typedef struct {
    uint16_t channels[SYN_IBUS_NUM_CHANNELS]; 
} SYN_IBUS_Frame;

typedef struct {
    uint8_t buf[SYN_IBUS_FRAME_SIZE]; 
    uint8_t idx;                      
    uint32_t frames_received;         
    uint32_t checksum_errors;         
    SYN_IBUS_Frame last_frame;        
} SYN_IBUS_Parser;

SYN_Status syn_ibus_init(SYN_IBUS_Parser *parser);

SYN_Status syn_ibus_parse_byte(SYN_IBUS_Parser *parser, uint8_t byte, SYN_IBUS_Frame *frame);

uint16_t syn_ibus_calc_checksum(const uint8_t buf[30]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_IBUS_H */
```


