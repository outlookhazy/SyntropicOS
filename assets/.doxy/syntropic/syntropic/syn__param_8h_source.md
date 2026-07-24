

# File syn\_param.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_param.h**](syn__param_8h.md)

[Go to the documentation of this file](syn__param_8h.md)


```C++

#ifndef SYN_PARAM_H
#define SYN_PARAM_H

#include "../common/syn_defs.h"
#include "../port/syn_port_flash.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Slot header ────────────────────────────────────────────────────────── */

#define SYN_PARAM_MAGIC  0xC0DEu

typedef struct {
    uint16_t  magic;       
    uint16_t  seq;         
    uint16_t  data_size;   
    uint16_t  crc;         
    uint8_t   _pad[8];     
} SYN_ParamSlotHeader;

/* ── Parameter store ────────────────────────────────────────────────────── */

typedef struct {
    uint32_t  flash_base;    
    uint32_t  sector_size;   
    uint8_t   sector_count;  
    uint16_t  data_size;     
    uint16_t  slot_size;     
    uint16_t  slots_per_sector; 
    /* Current state */
    uint8_t   active_sector; 
    uint16_t  active_slot;   
    uint16_t  next_seq;      
    bool      initialized;   
} SYN_ParamStore;

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_param_init(SYN_ParamStore *store,
                            uint32_t flash_base,
                            uint8_t sector_count,
                            uint16_t data_size);

SYN_Status syn_param_load(const SYN_ParamStore *store, void *data);

SYN_Status syn_param_save(SYN_ParamStore *store, const void *data);

SYN_Status syn_param_erase_all(SYN_ParamStore *store);

static inline uint16_t syn_param_write_count(const SYN_ParamStore *store)
{
    return (uint16_t)(store->next_seq > 0 ? store->next_seq - 1 : 0);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_PARAM_H */
```


