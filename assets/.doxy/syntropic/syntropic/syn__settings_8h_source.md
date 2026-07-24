

# File syn\_settings.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_settings.h**](syn__settings_8h.md)

[Go to the documentation of this file](syn__settings_8h.md)


```C++

#ifndef SYN_SETTINGS_H
#define SYN_SETTINGS_H

#include "../common/syn_defs.h"
#include "../storage/syn_param.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Callback ──────────────────────────────────────────────────────────── */

typedef void (*SYN_SettingsChangeCallback)(void *data, void *ctx);

/* ── Settings instance ─────────────────────────────────────────────────── */

typedef struct {
    SYN_ParamStore    store;         
    void              *data;         
    uint16_t           data_size;    
    const void        *defaults;     
    uint16_t           checksum;     
    SYN_SettingsChangeCallback on_change;    
    void                       *on_change_ctx; 
} SYN_Settings;

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_settings_init(SYN_Settings *s,
                               uint32_t flash_base, uint8_t sector_count,
                               void *data, uint16_t data_size,
                               const void *defaults);

SYN_Status syn_settings_save(SYN_Settings *s);

bool syn_settings_changed(const SYN_Settings *s);

static inline uint16_t syn_settings_checksum(const SYN_Settings *s)
{
    return s->checksum;
}

SYN_Status syn_settings_reset(SYN_Settings *s);

void syn_settings_on_change(SYN_Settings *s,
                              SYN_SettingsChangeCallback cb, void *ctx);

SYN_Status syn_settings_reload(SYN_Settings *s);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SETTINGS_H */
```


