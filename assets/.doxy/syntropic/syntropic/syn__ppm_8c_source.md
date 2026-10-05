

# File syn\_ppm.c

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_ppm.c**](syn__ppm_8c.md)

[Go to the documentation of this file](syn__ppm_8c.md)


```C++

#include "syntropic/input/syn_ppm.h"

#include <string.h>

SYN_Status syn_ppm_init(SYN_PPM_Decoder *ppm)
{
    if (!ppm) {
        return SYN_INVALID_PARAM;
    }

    memset(ppm, 0, sizeof(*ppm));
    for (int i = 0; i < SYN_PPM_MAX_CHANNELS; i++) {
        ppm->channels[i] = 1500U; /* 1500 us neutral default */
    }

    return SYN_OK;
}

SYN_Status syn_ppm_process_pulse(SYN_PPM_Decoder *ppm, uint16_t pulse_us)
{
    if (!ppm) {
        return SYN_INVALID_PARAM;
    }

    /* Sync gap detection (> 2700 us) */
    if (pulse_us >= SYN_PPM_SYNC_MIN_US) {
        if (ppm->in_frame && ppm->current_channel > 0) {
            ppm->channel_count = ppm->current_channel;
            ppm->frames_received++;
            ppm->current_channel = 0;
            return SYN_OK;
        }
        ppm->in_frame = true;
        ppm->current_channel = 0;
        return SYN_BUSY;
    }

    /* Channel pulse processing */
    if (ppm->in_frame) {
        if (ppm->current_channel < SYN_PPM_MAX_CHANNELS) {
            /* Clamp channel pulse to valid RC range (750..2250 us) */
            uint16_t val = pulse_us;
            if (val < 750U) {
                val = 750U;
            }
            if (val > 2250U) {
                val = 2250U;
            }
            ppm->channels[ppm->current_channel++] = val;
        }
    }

    return SYN_BUSY;
}

uint16_t syn_ppm_get_channel(const SYN_PPM_Decoder *ppm, uint8_t channel_idx)
{
    if (!ppm || channel_idx >= SYN_PPM_MAX_CHANNELS) {
        return 1500U;
    }

    return ppm->channels[channel_idx];
}
```


