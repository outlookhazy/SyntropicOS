

# File syn\_fwboot.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_fwboot.h**](syn__fwboot_8h.md)

[Go to the documentation of this file](syn__fwboot_8h.md)


```C++

#ifndef SYN_FWBOOT_H
#define SYN_FWBOOT_H

#include "../common/syn_defs.h"
#include "syn_fwimage.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Slot indices ───────────────────────────────────────────────────────── */

#define SYN_FW_SLOT_A  0      
#define SYN_FW_SLOT_B  1      
#define SYN_FW_SLOT_NONE  0xFF 
/* ── Boot manager ───────────────────────────────────────────────────────── */

typedef struct {
    uint32_t           slot_addr[2];   
    SYN_FwImageHeader  slot_hdr[2];    
    uint8_t            active_slot;    
    bool               initialized;    
} SYN_FwBootManager;

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_fwboot_init(SYN_FwBootManager *mgr,
                            uint32_t slot_a, uint32_t slot_b);

uint8_t syn_fwboot_select(SYN_FwBootManager *mgr, bool rollback);

SYN_Status syn_fwboot_confirm(SYN_FwBootManager *mgr);

static inline uint32_t syn_fwboot_slot_addr(const SYN_FwBootManager *mgr,
                                             uint8_t slot)
{
    return (slot < 2) ? mgr->slot_addr[slot] : 0;
}

static inline const SYN_FwImageHeader *syn_fwboot_slot_header(
    const SYN_FwBootManager *mgr, uint8_t slot)
{
    return (slot < 2) ? &mgr->slot_hdr[slot] : NULL;
}

static inline uint8_t syn_fwboot_active_slot(const SYN_FwBootManager *mgr)
{
    return mgr->active_slot;
}

SYN_Status syn_fwboot_refresh(SYN_FwBootManager *mgr);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FWBOOT_H */
```


