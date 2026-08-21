

# File syn\_rc\_failsafe.c

[**File List**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_rc\_failsafe.c**](syn__rc__failsafe_8c.md)

[Go to the documentation of this file](syn__rc__failsafe_8c.md)


```C++

#include "syntropic/control/syn_rc_failsafe.h"

#include <string.h>

SYN_Status syn_failsafe_init(SYN_Failsafe_Manager *mgr, const SYN_Failsafe_Config *config)
{
    if (!mgr || !config) {
        return SYN_INVALID_PARAM;
    }

    memset(mgr, 0, sizeof(*mgr));
    mgr->config = *config;

    /* Initialize default channels to 1500 us neutral */
    for (size_t i = 0; i < SYN_RC_FAILSAFE_MAX_CHANNELS; i++) {
        mgr->channels[i] = 1500U;
    }

    return SYN_OK;
}

SYN_Status syn_failsafe_feed_frame(SYN_Failsafe_Manager *mgr, const uint16_t *in_channels,
                                   uint8_t num_channels, uint32_t timestamp_ms)
{
    if (!mgr || !in_channels) {
        return SYN_INVALID_PARAM;
    }

    if (num_channels > SYN_RC_FAILSAFE_MAX_CHANNELS) {
        num_channels = SYN_RC_FAILSAFE_MAX_CHANNELS;
    }

    for (uint8_t i = 0; i < num_channels; i++) {
        mgr->channels[i] = in_channels[i];
    }

    mgr->last_frame_ms = timestamp_ms;
    mgr->in_failsafe = false;

    return SYN_OK;
}

bool syn_failsafe_step(SYN_Failsafe_Manager *mgr, uint32_t now_ms, uint16_t *out_channels)
{
    if (!mgr) {
        return true;
    }

    uint32_t elapsed = now_ms - mgr->last_frame_ms;
    if (elapsed >= mgr->config.timeout_ms) {
        if (!mgr->in_failsafe) {
            mgr->in_failsafe = true;
            mgr->failsafe_events++;
        }
    }

    if (out_channels) {
        for (size_t i = 0; i < SYN_RC_FAILSAFE_MAX_CHANNELS; i++) {
            if (mgr->in_failsafe) {
                switch (mgr->config.channel_modes[i]) {
                case SYN_FAILSAFE_FALLBACK:
                    out_channels[i] = mgr->config.fallback_us[i];
                    break;
                case SYN_FAILSAFE_DISARM:
                    out_channels[i] = 1000U;
                    break;
                case SYN_FAILSAFE_HOLD:
                default:
                    out_channels[i] = mgr->channels[i];
                    break;
                }
            } else {
                out_channels[i] = mgr->channels[i];
            }
        }
    }

    return mgr->in_failsafe;
}
```


