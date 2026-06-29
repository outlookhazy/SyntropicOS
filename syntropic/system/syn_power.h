/**
 * @file syn_power.h
 * @brief Power / voltage monitor.
 *
 * Wraps ADC + hysteresis to provide voltage monitoring with brownout
 * detection and recovery callbacks. Optionally pushes readings to
 * a SYN_Signal window for noise statistics.
 *
 * Usage:
 * @code
 *   static SYN_Power pwr;
 *   static SYN_ADC   vbat_adc;
 *
 *   SYN_Power_Config cfg = {
 *       .adc           = &vbat_adc,
 *       .brownout_mv   = 3000,   // 3.0V
 *       .restore_mv    = 3200,   // 3.2V (hysteresis)
 *       .on_brownout   = my_brownout_handler,
 *       .on_restore    = my_restore_handler,
 *   };
 *   syn_power_init(&pwr, &cfg);
 *
 *   // In your main loop:
 *   syn_power_update(&pwr);
 *   int32_t mv = syn_power_voltage_mv(&pwr);
 * @endcode
 * @ingroup syn_system
 */

#ifndef SYN_POWER_H
#define SYN_POWER_H

#include "../common/syn_defs.h"
#include "../drivers/syn_adc.h"
#include "../util/syn_hysteresis.h"
#include "../dsp/syn_signal.h"
#include "../system/syn_errlog.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Callback type ──────────────────────────────────────────────────────── */

struct SYN_Power;

/**
 * @brief Power event callback.
 * @param pwr  Power monitor that fired.
 * @param ctx  User context.
 */
typedef void (*SYN_PowerCallback)(struct SYN_Power *pwr, void *ctx);

/* ── Configuration ──────────────────────────────────────────────────────── */

/** @brief Power monitor configuration. */
typedef struct {
    SYN_ADC            *adc;            /**< ADC channel for voltage       */
    int32_t              brownout_mv;    /**< Low-voltage threshold (mV)    */
    int32_t              restore_mv;     /**< Voltage restore threshold (mV)*/
    SYN_PowerCallback   on_brownout;    /**< Called on low-voltage event   */
    SYN_PowerCallback   on_restore;     /**< Called when voltage restores  */
    void                *ctx;            /**< User context for callbacks    */
} SYN_Power_Config;

/* ── Power monitor instance ─────────────────────────────────────────────── */

/** @brief Power monitor instance — ADC + hysteresis + brownout state. */
typedef struct SYN_Power {
    SYN_ADC           *adc;              /**< ADC channel                   */
    SYN_Hysteresis     hyst;            /**< Brownout hysteresis state     */
    int32_t             voltage_mv;      /**< Last reading in millivolts   */
    bool                brownout;        /**< Currently in brownout?       */
    SYN_PowerCallback  on_brownout;     /**< Brownout callback             */
    SYN_PowerCallback  on_restore;      /**< Restore callback              */
    void               *ctx;            /**< Callback context              */
    SYN_Signal        *stats;           /**< Optional noise stats         */
    SYN_ErrLog        *errlog;          /**< Optional error logging       */
} SYN_Power;

/* ── Error codes ────────────────────────────────────────────────────────── */

/** @brief Brownout voltage detected (errlog code). */
#define SYN_POWER_ERR_BROWNOUT  0x0400

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize power monitor.
 * @param pwr  Power monitor instance.
 * @param cfg  Configuration.
 */
void syn_power_init(SYN_Power *pwr, const SYN_Power_Config *cfg);

/**
 * @brief Update — read ADC and check thresholds.
 *
 * Call from your main loop or scheduler task.
 *
 * @param pwr  Power monitor.
 */
void syn_power_update(SYN_Power *pwr);

/**
 * @brief Get last voltage reading in millivolts.
 * @param pwr  Power monitor.
 * @return Voltage in mV.
 */
static inline int32_t syn_power_voltage_mv(const SYN_Power *pwr)
{
    return pwr->voltage_mv;
}

/**
 * @brief Check if currently in brownout state.
 * @param pwr  Power monitor.
 * @return true if in brownout.
 */
static inline bool syn_power_is_brownout(const SYN_Power *pwr)
{
    return pwr->brownout;
}

/**
 * @brief Attach signal stats window.
 * @param pwr    Power monitor.
 * @param stats  Initialized SYN_Signal, or NULL to detach.
 */
void syn_power_set_stats(SYN_Power *pwr, SYN_Signal *stats);

/**
 * @brief Attach error log for brownout events.
 * @param pwr     Power monitor.
 * @param errlog  Error log instance.
 */
void syn_power_set_errlog(SYN_Power *pwr, SYN_ErrLog *errlog);

#ifdef __cplusplus
}
#endif

#endif /* SYN_POWER_H */
