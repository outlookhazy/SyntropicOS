/**
 * @file syn_adc.h
 * @brief ADC abstraction — oversampling, calibration, unit conversion.
 *
 * Wraps the raw ADC port with oversampling, digital filtering,
 * calibration (via LUT or linear scale/offset), and conversion
 * to engineering units (millivolts, etc.).
 *
 * @par Usage
 * @code
 *   SYN_ADC adc;
 *   SYN_FilterEMA adc_filter;
 *   syn_filter_ema_init(&adc_filter, 128);
 *
 *   SYN_ADC_Config cfg = {
 *       .channel = 0,
 *       .oversample = 4,      // 4× oversampling (2 extra bits)
 *       .filter = &adc_filter,
 *       .filter_type = SYN_ADC_FILTER_EMA,
 *       .cal_offset = 0,
 *       .cal_scale = 1000,    // × 1.000
 *       .cal_scale_shift = 10, // scale denominator = 1024
 *   };
 *   syn_adc_init(&adc, &cfg);
 *
 *   int32_t mv = syn_adc_read_mv(&adc);
 * @endcode
 * @ingroup syn_drivers
 */

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

/** @brief ADC digital filter selector. */
typedef enum {
    SYN_ADC_FILTER_NONE   = 0,  /**< No filtering                        */
    SYN_ADC_FILTER_MA     = 1,  /**< Moving average                      */
    SYN_ADC_FILTER_EMA    = 2,  /**< Exponential moving average          */
    SYN_ADC_FILTER_MEDIAN = 3,  /**< Median filter                       */
} SYN_ADC_FilterType;

/* ── ADC configuration ──────────────────────────────────────────────────── */

/** @brief ADC channel configuration. */
typedef struct {
    uint8_t              channel;        /**< ADC channel number           */
    uint8_t              oversample;     /**< Oversampling count (1,4,16..)*/

    /* Optional filter */
    void                *filter;         /**< Filter instance              */
    SYN_ADC_FilterType  filter_type;    /**< SYN_ADC_FilterType           */

    /* Calibration: result = (raw + cal_offset) * cal_scale >> cal_scale_shift */
    int16_t              cal_offset;     /**< Zero-point offset            */
    uint16_t             cal_scale;      /**< Scale numerator              */
    uint8_t              cal_scale_shift; /**< Scale denominator = 1<<shift*/
} SYN_ADC_Config;

/* ── ADC instance ───────────────────────────────────────────────────────── */

/** @brief ADC channel instance — config + last readings. */
typedef struct {
    SYN_ADC_Config cfg;          /**< Configuration                       */
    int32_t         raw;         /**< Last raw reading (after oversample)  */
    int32_t         filtered;    /**< After filter                         */
    int32_t         calibrated;  /**< After calibration                    */
    SYN_Signal    *stats;       /**< If set, calibrated value pushed here */
} SYN_ADC;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize an ADC channel with oversampling and calibration.
 * @param adc  ADC instance.
 * @param cfg  Configuration.
 * @return SYN_OK on success.
 */
SYN_Status syn_adc_init(SYN_ADC *adc, const SYN_ADC_Config *cfg);

/**
 * @brief Read the ADC (oversample + filter + calibrate).
 *
 * @param adc  ADC instance.
 * @return Calibrated value.
 */
int32_t syn_adc_read(SYN_ADC *adc);

/**
 * @brief Read and convert to millivolts.
 *
 * Uses the port's reference voltage and resolution for conversion.
 *
 * @param adc  ADC instance.
 * @return Value in millivolts.
 */
int32_t syn_adc_read_mv(SYN_ADC *adc);

/**
 * @brief Get last raw value.
 * @param adc  ADC instance.
 * @return Raw ADC reading.
 */
static inline int32_t syn_adc_raw(const SYN_ADC *adc) { return adc->raw; }

/**
 * @brief Get last filtered value.
 * @param adc  ADC instance.
 * @return Filtered reading.
 */
static inline int32_t syn_adc_filtered(const SYN_ADC *adc) { return adc->filtered; }

/**
 * @brief Get last calibrated value.
 * @param adc  ADC instance.
 * @return Calibrated reading.
 */
static inline int32_t syn_adc_calibrated(const SYN_ADC *adc) { return adc->calibrated; }

/**
 * @brief Update calibration at runtime.
 * @param adc     ADC instance.
 * @param offset  New zero-point offset.
 * @param scale   New scale numerator.
 * @param shift   New scale shift (denominator = 1 << shift).
 */
void syn_adc_set_calibration(SYN_ADC *adc, int16_t offset,
                              uint16_t scale, uint8_t shift);

/**
 * @brief Attach a signal statistics window.
 *
 * Each syn_adc_read() pushes the calibrated value into the signal
 * window, giving you noise stats (min/max/mean/variance) for free.
 *
 * @param adc    ADC instance.
 * @param stats  Initialized SYN_Signal, or NULL to detach.
 */
void syn_adc_set_stats(SYN_ADC *adc, SYN_Signal *stats);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ADC_H */
