

# File syn\_adc.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_adc.c**](syn__adc_8c.md)

[Go to the documentation of this file](syn__adc_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_ADC) || SYN_USE_ADC

#include "../util/syn_assert.h"
#include "syn_adc.h"

#include <string.h>

SYN_Status syn_adc_init(SYN_ADC *adc, const SYN_ADC_Config *cfg)
{
    if (adc == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }

    memset(adc, 0, sizeof(*adc));
    adc->cfg = *cfg;

    if (adc->cfg.vref_mv == 0) {
        adc->cfg.vref_mv = 3300; /* Default 3.3V reference */
    }

    SYN_Status status = syn_port_adc_init(cfg->adc_id, cfg->channel_mask);
    if (status != SYN_OK) {
        return status;
    }

    adc->initialized = true;
    return SYN_OK;
}

SYN_Status syn_adc_deinit(SYN_ADC *adc)
{
    if (adc == NULL) {
        return SYN_INVALID_PARAM;
    }

    if (!adc->initialized) {
        return SYN_OK;
    }

    SYN_Status status = syn_port_adc_deinit(adc->cfg.adc_id);
    adc->initialized = false;
    return status;
}

uint16_t syn_adc_read_raw(SYN_ADC *adc, uint8_t channel)
{
    if (adc == NULL || !adc->initialized) {
        return 0;
    }

    return syn_port_adc_read_channel(adc->cfg.adc_id, channel);
}

uint32_t syn_adc_read_mv(SYN_ADC *adc, uint8_t channel)
{
    if (adc == NULL) {
        return 0;
    }
    uint16_t raw = syn_adc_read_raw(adc, channel);
    return ((uint32_t)raw * adc->cfg.vref_mv) / 4095U;
}

SYN_Status syn_adc_start_dma_scan(SYN_ADC *adc, uint16_t *buf, size_t num_channels)
{
    if (adc == NULL || buf == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (!adc->initialized) {
        return SYN_INVALID_PARAM;
    }

    return syn_port_adc_start_dma_scan(adc->cfg.adc_id, buf, num_channels);
}

#endif /* SYN_USE_ADC */
```


