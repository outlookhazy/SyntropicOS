

# File syn\_adc.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_adc.h**](syn__adc_8h.md)

[Go to the documentation of this file](syn__adc_8h.md)


```C++

#ifndef SYN_ADC_H
#define SYN_ADC_H

#include "../common/syn_defs.h"
#include "../port/syn_port_adc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t adc_id;        
    uint32_t channel_mask; 
    uint32_t vref_mv;      
    bool use_dma;          
} SYN_ADC_Config;

typedef struct {
    SYN_ADC_Config cfg; 
    bool initialized;   
} SYN_ADC;

SYN_Status syn_adc_init(SYN_ADC *adc, const SYN_ADC_Config *cfg);

SYN_Status syn_adc_deinit(SYN_ADC *adc);

uint16_t syn_adc_read_raw(SYN_ADC *adc, uint8_t channel);

uint32_t syn_adc_read_mv(SYN_ADC *adc, uint8_t channel);

SYN_Status syn_adc_start_dma_scan(SYN_ADC *adc, uint16_t *buf, size_t num_channels);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ADC_H */
```


