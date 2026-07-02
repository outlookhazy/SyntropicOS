

# File syn\_adc.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_adc.h**](syn__adc_8h.md)

[Go to the documentation of this file](syn__adc_8h.md)


```C++

#ifndef SYN_ADC_H
#define SYN_ADC_H

#include "../common/syn_defs.h"
#include "../port/syn_port_adc.h"
#include "../dsp/syn_filter.h"
#include "../dsp/syn_signal.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Filter type ────────────────────────────────────────────────────────── */

typedef enum {
    SYN_ADC_FILTER_NONE   = 0,  
    SYN_ADC_FILTER_MA     = 1,  
    SYN_ADC_FILTER_EMA    = 2,  
    SYN_ADC_FILTER_MEDIAN = 3,  
} SYN_ADC_FilterType;

/* ── ADC configuration ──────────────────────────────────────────────────── */

typedef struct {
    uint8_t              channel;        
    uint8_t              oversample;     
    /* Optional filter */
    void                *filter;         
    SYN_ADC_FilterType  filter_type;    
    /* Calibration: result = (raw + cal_offset) * cal_scale >> cal_scale_shift */
    int16_t              cal_offset;     
    uint16_t             cal_scale;      
    uint8_t              cal_scale_shift; 
} SYN_ADC_Config;

/* ── ADC instance ───────────────────────────────────────────────────────── */

typedef struct {
    SYN_ADC_Config cfg;          
    int32_t         raw;         
    int32_t         filtered;    
    int32_t         calibrated;  
    SYN_Signal    *stats;       
} SYN_ADC;

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_adc_init(SYN_ADC *adc, const SYN_ADC_Config *cfg);

int32_t syn_adc_read(SYN_ADC *adc);

int32_t syn_adc_read_mv(SYN_ADC *adc);

static inline int32_t syn_adc_raw(const SYN_ADC *adc) { return adc->raw; }

static inline int32_t syn_adc_filtered(const SYN_ADC *adc) { return adc->filtered; }

static inline int32_t syn_adc_calibrated(const SYN_ADC *adc) { return adc->calibrated; }

void syn_adc_set_calibration(SYN_ADC *adc, int16_t offset,
                              uint16_t scale, uint8_t shift);

void syn_adc_set_stats(SYN_ADC *adc, SYN_Signal *stats);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ADC_H */
```


