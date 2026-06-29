#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_ADC) || SYN_USE_ADC

/**
 * @file syn_adc.c
 * @brief ADC abstraction implementation.
 */

#include "syn_adc.h"
#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_adc_init(SYN_ADC *adc, const SYN_ADC_Config *cfg)
{
    SYN_ASSERT(adc != NULL);
    SYN_ASSERT(cfg != NULL);

    memset(adc, 0, sizeof(*adc));
    adc->cfg = *cfg;

    if (adc->cfg.oversample == 0) adc->cfg.oversample = 1;
    if (adc->cfg.cal_scale == 0)  adc->cfg.cal_scale  = 1;

    return syn_port_adc_init(cfg->channel);
}

/**
 * @brief Apply the configured filter to an ADC reading.
 * @param adc    ADC instance.
 * @param value  Raw ADC value.
 * @return Filtered value.
 */
static int16_t apply_filter(SYN_ADC *adc, int16_t value)
{
    if (adc->cfg.filter == NULL) return value;

    switch (adc->cfg.filter_type) {
    case SYN_ADC_FILTER_MA:
        return syn_filter_ma_update((SYN_FilterMA *)adc->cfg.filter, value);
    case SYN_ADC_FILTER_EMA:
        return syn_filter_ema_update((SYN_FilterEMA *)adc->cfg.filter, value);
    case SYN_ADC_FILTER_MEDIAN:
        return syn_filter_median_update((SYN_FilterMedian *)adc->cfg.filter, value);
    default:
        return value;
    }
}

int32_t syn_adc_read(SYN_ADC *adc)
{
    SYN_ASSERT(adc != NULL);

    /* Oversampling */
    int32_t sum = 0;
    for (uint8_t i = 0; i < adc->cfg.oversample; i++) {
        sum += (int32_t)syn_port_adc_read(adc->cfg.channel);
    }
    adc->raw = sum / adc->cfg.oversample;

    /* Filter */
    adc->filtered = apply_filter(adc, (int16_t)adc->raw);

    /* Calibration: (filtered + offset) * scale >> shift */
    int32_t cal = ((int32_t)adc->filtered + adc->cfg.cal_offset);
    if (adc->cfg.cal_scale != 1 || adc->cfg.cal_scale_shift != 0) {
        cal = (cal * (int32_t)adc->cfg.cal_scale) >> adc->cfg.cal_scale_shift;
    }
    adc->calibrated = cal;

    /* Push to stats window (if attached) */
    if (adc->stats != NULL) {
        syn_signal_push(adc->stats, adc->calibrated);
    }

    return adc->calibrated;
}

int32_t syn_adc_read_mv(SYN_ADC *adc)
{
    SYN_ASSERT(adc != NULL);

    syn_adc_read(adc);

    /* Convert raw (after oversample) to millivolts */
    uint16_t ref_mv = syn_port_adc_reference_mv();
    uint8_t  bits   = syn_port_adc_resolution();
    int32_t  max_raw = (int32_t)((1L << bits) - 1);

    if (max_raw == 0) return 0;

    return (adc->raw * (int32_t)ref_mv) / max_raw;
}

void syn_adc_set_calibration(SYN_ADC *adc, int16_t offset,
                              uint16_t scale, uint8_t shift)
{
    SYN_ASSERT(adc != NULL);
    adc->cfg.cal_offset      = offset;
    adc->cfg.cal_scale        = scale;
    adc->cfg.cal_scale_shift  = shift;
}

void syn_adc_set_stats(SYN_ADC *adc, SYN_Signal *stats)
{
    SYN_ASSERT(adc != NULL);
    adc->stats = stats;
}

#endif /* SYN_USE_ADC */
