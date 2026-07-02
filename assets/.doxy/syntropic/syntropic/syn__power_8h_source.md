

# File syn\_power.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_power.h**](syn__power_8h.md)

[Go to the documentation of this file](syn__power_8h.md)


```C++

#ifndef SYN_POWER_H
#define SYN_POWER_H

#include "../common/syn_defs.h"
#include "../drivers/syn_adc.h"
#include "../util/syn_hysteresis.h"
#include "../dsp/syn_signal.h"
#include "../system/syn_errlog.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Callback type ──────────────────────────────────────────────────────── */

struct SYN_Power;

typedef void (*SYN_PowerCallback)(struct SYN_Power *pwr, void *ctx);

/* ── Configuration ──────────────────────────────────────────────────────── */

typedef struct {
    SYN_ADC            *adc;            
    int32_t              brownout_mv;    
    int32_t              restore_mv;     
    SYN_PowerCallback   on_brownout;    
    SYN_PowerCallback   on_restore;     
    void                *ctx;            
} SYN_Power_Config;

/* ── Power monitor instance ─────────────────────────────────────────────── */

typedef struct SYN_Power {
    SYN_ADC           *adc;              
    SYN_Hysteresis     hyst;            
    int32_t             voltage_mv;      
    bool                brownout;        
    SYN_PowerCallback  on_brownout;     
    SYN_PowerCallback  on_restore;      
    void               *ctx;            
    SYN_Signal        *stats;           
    SYN_ErrLog        *errlog;          
} SYN_Power;

/* ── Error codes ────────────────────────────────────────────────────────── */

#define SYN_POWER_ERR_BROWNOUT  0x0400

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_power_init(SYN_Power *pwr, const SYN_Power_Config *cfg);

void syn_power_update(SYN_Power *pwr);

static inline int32_t syn_power_voltage_mv(const SYN_Power *pwr)
{
    return pwr->voltage_mv;
}

static inline bool syn_power_is_brownout(const SYN_Power *pwr)
{
    return pwr->brownout;
}

void syn_power_set_stats(SYN_Power *pwr, SYN_Signal *stats);

void syn_power_set_errlog(SYN_Power *pwr, SYN_ErrLog *errlog);

#ifdef __cplusplus
}
#endif

#endif /* SYN_POWER_H */
```


