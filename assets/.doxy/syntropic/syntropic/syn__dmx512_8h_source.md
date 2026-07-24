

# File syn\_dmx512.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dmx512.h**](syn__dmx512_8h.md)

[Go to the documentation of this file](syn__dmx512_8h.md)


```C++

#ifndef SYN_DMX512_H
#define SYN_DMX512_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_DMX512_MAX_CHANNELS           512U
#define SYN_DMX512_START_CODE_NULL        0x00U 
#define SYN_DMX512_BAUD_RATE              250000UL

typedef enum {
    SYN_DMX512_STATE_WAIT_BREAK = 0,
    SYN_DMX512_STATE_WAIT_START,
    SYN_DMX512_STATE_READ_SLOTS,
} SYN_DMX512_RxState;

typedef struct {
    uint8_t  channels[SYN_DMX512_MAX_CHANNELS]; 
    uint16_t num_channels;                       
    uint8_t  start_code;                         
} SYN_DMX512_Master;

typedef struct {
    uint8_t            rx_channels[SYN_DMX512_MAX_CHANNELS]; 
    uint16_t           start_address;                        
    uint16_t           footprint;                            
    SYN_DMX512_RxState rx_state;                             
    uint16_t           current_slot;                         
    uint8_t            start_code;                           
    bool               frame_complete;                       
} SYN_DMX512_Slave;

void syn_dmx512_master_init(SYN_DMX512_Master *master, uint16_t num_channels);

SYN_Status syn_dmx512_master_set_channel(SYN_DMX512_Master *master, uint16_t channel, uint8_t value);

uint8_t syn_dmx512_master_get_channel(const SYN_DMX512_Master *master, uint16_t channel);

size_t syn_dmx512_master_build_frame(const SYN_DMX512_Master *master, uint8_t *out_buf, size_t max_len);

void syn_dmx512_slave_init(SYN_DMX512_Slave *slave, uint16_t start_address, uint16_t footprint);

void syn_dmx512_slave_rx_break(SYN_DMX512_Slave *slave);

void syn_dmx512_slave_rx_byte(SYN_DMX512_Slave *slave, uint8_t byte);

uint8_t syn_dmx512_slave_get_channel(const SYN_DMX512_Slave *slave, uint16_t offset);

bool syn_dmx512_slave_is_updated(SYN_DMX512_Slave *slave);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DMX512_H */
```


