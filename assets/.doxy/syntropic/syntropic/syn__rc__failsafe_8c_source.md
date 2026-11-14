

# File syn\_rc\_failsafe.c

[**File List**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_rc\_failsafe.c**](syn__rc__failsafe_8c.md)

[Go to the documentation of this file](syn__rc__failsafe_8c.md)


```C++


#include "syntropic/control/syn_rc_failsafe.h"

#include <string.h>

static uint16_t failsafe_default_channel_us(const SYN_Failsafe_Config *cfg, size_t idx)
{
    if (cfg->channel_modes[idx] == SYN_FAILSAFE_DISARM) {
        return 1000U;
    }
    if (cfg->fallback_us[idx] != 0U) {
        return cfg->fallback_us[idx];
    }
    return (idx == 2U) ? 1000U : 1500U;
}

SYN_Status syn_failsafe_init(SYN_Failsafe_Manager *mgr, const SYN_Failsafe_Config *config)
{
    if (!mgr || !config) {
        return SYN_INVALID_PARAM;
    }

    memset(mgr, 0, sizeof(*mgr));
    mgr->config = *config;
    mgr->in_failsafe = true;
    mgr->ever_received = false;

    for (size_t i = 0U; i < SYN_RC_FAILSAFE_MAX_CHANNELS; i++) {
        mgr->channels[i] = failsafe_default_channel_us(config, i);
    }

    return SYN_OK;
}

SYN_Status syn_failsafe_feed_frame(SYN_Failsafe_Manager *mgr, const uint16_t *in_channels,
                                   uint8_t num_channels, uint32_t timestamp_ms)
{
    if (!mgr || !in_channels || num_channels == 0U) {
        return SYN_INVALID_PARAM;
    }

    if (num_channels > SYN_RC_FAILSAFE_MAX_CHANNELS) {
        num_channels = SYN_RC_FAILSAFE_MAX_CHANNELS;
    }

    for (uint8_t i = 0U; i < num_channels; i++) {
        mgr->channels[i] = in_channels[i];
    }

    mgr->last_frame_ms = timestamp_ms;
    mgr->ever_received = true;
    mgr->in_failsafe = false;

    return SYN_OK;
}

bool syn_failsafe_step(SYN_Failsafe_Manager *mgr, uint32_t now_ms, uint16_t *out_channels)
{
    if (!mgr) {
        return true;
    }

    if (!mgr->ever_received) {
        mgr->in_failsafe = true;
    } else {
        int32_t diff = (int32_t)(now_ms - mgr->last_frame_ms);
        uint32_t elapsed = (diff > 0) ? (uint32_t)diff : 0U;
        if (elapsed >= mgr->config.timeout_ms && !mgr->in_failsafe) {
            mgr->in_failsafe = true;
            mgr->failsafe_events++;
        }
    }

    if (out_channels) {
        for (size_t i = 0U; i < SYN_RC_FAILSAFE_MAX_CHANNELS; i++) {
            if (mgr->in_failsafe) {
                switch (mgr->config.channel_modes[i]) {
                case SYN_FAILSAFE_FALLBACK:
                    out_channels[i] = failsafe_default_channel_us(&mgr->config, i);
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


