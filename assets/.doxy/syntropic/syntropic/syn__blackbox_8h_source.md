

# File syn\_blackbox.h

[**File List**](files.md) **>** [**log**](dir_1a216ccd2a39b426a7ff78fb20f852eb.md) **>** [**syn\_blackbox.h**](syn__blackbox_8h.md)

[Go to the documentation of this file](syn__blackbox_8h.md)


```C++

#ifndef SYN_BLACKBOX_H
#define SYN_BLACKBOX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_BLACKBOX_FRAME_INTRA 'I' 
#define SYN_BLACKBOX_FRAME_DELTA 'P' 
#define SYN_BLACKBOX_FRAME_SLOW 'S'  
typedef struct {
    uint32_t iteration;  
    uint32_t time_us;    
    int16_t gyro[3];     
    int16_t accel[3];    
    int16_t setpoint[4]; 
    uint16_t motor[4];   
} SYN_Blackbox_Record;

typedef struct {
    SYN_Blackbox_Record last_record; 
    uint32_t frame_count;            
    uint32_t bytes_written;          
} SYN_Blackbox;

SYN_Status syn_blackbox_init(SYN_Blackbox *bb);

SYN_Status syn_blackbox_encode_intra(SYN_Blackbox *bb, const SYN_Blackbox_Record *record,
                                     uint8_t *buf_out, size_t *out_len);

SYN_Status syn_blackbox_encode_delta(SYN_Blackbox *bb, const SYN_Blackbox_Record *record,
                                     uint8_t *buf_out, size_t *out_len);

size_t syn_blackbox_encode_varint(int32_t val, uint8_t *buf_out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLACKBOX_H */
```


