

# File syn\_cia401.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia401.h**](syn__cia401_8h.md)

[Go to the documentation of this file](syn__cia401_8h.md)


```C++

#ifndef SYN_CIA401_H
#define SYN_CIA401_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../common/syn_defs.h"
#include "syn_canopen.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_CIA401_OD_DIGITAL_INPUT_8BIT   0x6000U 
#define SYN_CIA401_OD_DIGITAL_OUTPUT_8BIT  0x6200U 
#define SYN_CIA401_OD_ANALOG_INPUT_16BIT   0x6401U 
#define SYN_CIA401_OD_ANALOG_OUTPUT_16BIT  0x6411U 
#define SYN_CIA401_OD_ANALOG_INPUT_DELTA   0x6421U 
#define SYN_CIA401_OD_ANALOG_INPUT_LIMIT_L 0x6424U 
#define SYN_CIA401_OD_ANALOG_INPUT_LIMIT_H 0x6425U 
#define SYN_CIA401_MAX_DIGITAL_8BIT 4  
#define SYN_CIA401_MAX_ANALOG_16BIT 8  
typedef struct {
    uint8_t  digital_in[SYN_CIA401_MAX_DIGITAL_8BIT];   
    uint8_t  digital_out[SYN_CIA401_MAX_DIGITAL_8BIT];  
    int16_t  analog_in[SYN_CIA401_MAX_ANALOG_16BIT];    
    int16_t  analog_out[SYN_CIA401_MAX_ANALOG_16BIT];   
    int16_t  analog_delta[SYN_CIA401_MAX_ANALOG_16BIT]; 
    int16_t  prev_analog_in[SYN_CIA401_MAX_ANALOG_16BIT]; 
    uint8_t  digital_in_count;                          
    uint8_t  digital_out_count;                         
    uint8_t  analog_in_count;                           
    uint8_t  analog_out_count;                          
} SYN_CiA401_Device;

void syn_cia401_init(SYN_CiA401_Device *dev, uint8_t dig_in_bytes, uint8_t dig_out_bytes,
                     uint8_t analog_in_ch, uint8_t analog_out_ch);

size_t syn_cia401_populate_od(SYN_CiA401_Device *dev, SYN_CANOpenODEntry *od_table, size_t max_entries);

bool syn_cia401_check_analog_delta(SYN_CiA401_Device *dev, uint8_t *channel);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CIA401_H */
```


