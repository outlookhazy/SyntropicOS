

# File syn\_port\_adc.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_adc.h**](syn__port__adc_8h.md)

[Go to the documentation of this file](syn__port__adc_8h.md)


```C++

#ifndef SYN_PORT_ADC_H
#define SYN_PORT_ADC_H

#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_adc_init(uint8_t adc_id, uint32_t channel_mask);

SYN_Status syn_port_adc_deinit(uint8_t adc_id);

uint16_t syn_port_adc_read_channel(uint8_t adc_id, uint8_t channel);

SYN_Status syn_port_adc_start_dma_scan(uint8_t adc_id, uint16_t *dest, size_t num_channels);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_ADC_H */
```


