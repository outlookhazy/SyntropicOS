

# File syn\_sbus.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_sbus.h**](syn__sbus_8h.md)

[Go to the documentation of this file](syn__sbus_8h.md)


```C++

#ifndef SYN_SBUS_H
#define SYN_SBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_SBUS_NUM_CHANNELS 16 
#define SYN_SBUS_FRAME_SIZE 25   
#define SYN_SBUS_HEADER 0x0FU    
typedef struct {
    uint16_t channels[SYN_SBUS_NUM_CHANNELS]; 
    bool ch17;                                
    bool ch18;                                
    bool frame_loss;                          
    bool failsafe;                            
} SYN_SBUS_Frame;

typedef struct {
    uint8_t buf[SYN_SBUS_FRAME_SIZE]; 
    uint8_t idx;                      
    uint32_t frames_received;         
    uint32_t frame_loss_count;        
    uint32_t failsafe_count;          
    SYN_SBUS_Frame last_frame;        
} SYN_SBUS_Parser;

SYN_Status syn_sbus_init(SYN_SBUS_Parser *parser);

SYN_Status syn_sbus_parse_byte(SYN_SBUS_Parser *parser, uint8_t byte, SYN_SBUS_Frame *frame);

SYN_Status syn_sbus_decode_buffer(const uint8_t buf[SYN_SBUS_FRAME_SIZE], SYN_SBUS_Frame *frame);

uint16_t syn_sbus_raw_to_us(uint16_t raw_val);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SBUS_H */
```


