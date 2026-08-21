

# File syn\_rc\_failsafe.h

[**File List**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_rc\_failsafe.h**](syn__rc__failsafe_8h.md)

[Go to the documentation of this file](syn__rc__failsafe_8h.md)


```C++

#ifndef SYN_RC_FAILSAFE_H
#define SYN_RC_FAILSAFE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_RC_FAILSAFE_MAX_CHANNELS 16U

typedef enum {
    SYN_FAILSAFE_HOLD = 0, 
    SYN_FAILSAFE_FALLBACK, 
    SYN_FAILSAFE_DISARM    
} SYN_Failsafe_ChannelMode;

typedef struct {
    uint32_t timeout_ms; 
    SYN_Failsafe_ChannelMode
        channel_modes[SYN_RC_FAILSAFE_MAX_CHANNELS];    
    uint16_t fallback_us[SYN_RC_FAILSAFE_MAX_CHANNELS]; 
} SYN_Failsafe_Config;

typedef struct {
    SYN_Failsafe_Config config;                      
    uint32_t last_frame_ms;                          
    bool in_failsafe;                                
    uint32_t failsafe_events;                        
    uint16_t channels[SYN_RC_FAILSAFE_MAX_CHANNELS]; 
} SYN_Failsafe_Manager;

SYN_Status syn_failsafe_init(SYN_Failsafe_Manager *mgr, const SYN_Failsafe_Config *config);

SYN_Status syn_failsafe_feed_frame(SYN_Failsafe_Manager *mgr, const uint16_t *in_channels,
                                   uint8_t num_channels, uint32_t timestamp_ms);

bool syn_failsafe_step(SYN_Failsafe_Manager *mgr, uint32_t now_ms, uint16_t *out_channels);

#ifdef __cplusplus
}
#endif

#endif /* SYN_RC_FAILSAFE_H */
```


