

# File syn\_boot.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_boot.h**](syn__boot_8h.md)

[Go to the documentation of this file](syn__boot_8h.md)


```C++

#ifndef SYN_BOOT_H
#define SYN_BOOT_H

#include "../common/syn_defs.h"
#include "../storage/syn_param.h"
#include "../system/syn_errlog.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Boot data (persisted to flash) ─────────────────────────────────────── */

typedef struct {
    uint32_t  boot_count;       
    uint16_t  fail_count;       
    uint8_t   last_reset;       
    uint8_t   was_healthy;      
} SYN_BootData;

/* ── Reset reasons (application can extend) ─────────────────────────────── */

typedef enum {
    SYN_RESET_UNKNOWN     = 0,   
    SYN_RESET_POWER_ON    = 1,   
    SYN_RESET_WATCHDOG    = 2,   
    SYN_RESET_SOFTWARE    = 3,   
    SYN_RESET_EXTERNAL    = 4,   
    SYN_RESET_BROWNOUT    = 5,   
    SYN_RESET_ASSERT      = 6,   
    SYN_RESET_USER        = 0x80,  
} SYN_ResetReason;

/* ── Boot manager ───────────────────────────────────────────────────────── */

typedef struct {
    SYN_ParamStore  *store;         
    SYN_BootData     data;          
    uint16_t          safe_threshold; 
    bool              safe_mode;     
    bool              initialized;   
    SYN_ErrLog       *errlog;      
} SYN_Boot;

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_boot_init(SYN_Boot *boot, SYN_ParamStore *store,
                           uint16_t safe_threshold);

SYN_Status syn_boot_mark_healthy(SYN_Boot *boot);

void syn_boot_set_reset_reason(SYN_Boot *boot, uint8_t reason);

static inline bool syn_boot_is_safe_mode(const SYN_Boot *boot)
{
    return boot->safe_mode;
}

static inline uint32_t syn_boot_count(const SYN_Boot *boot)
{
    return boot->data.boot_count;
}

static inline uint16_t syn_boot_fail_count(const SYN_Boot *boot)
{
    return boot->data.fail_count;
}

static inline uint8_t syn_boot_reset_reason(const SYN_Boot *boot)
{
    return boot->data.last_reset;
}

SYN_Status syn_boot_clear_safe_mode(SYN_Boot *boot);

void syn_boot_set_errlog(SYN_Boot *boot, SYN_ErrLog *errlog);

void syn_boot_log_events(SYN_Boot *boot);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BOOT_H */
```


