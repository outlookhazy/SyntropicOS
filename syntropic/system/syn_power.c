#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_POWER) || SYN_USE_POWER

/**
 * @file syn_power.c
 * @brief Power / voltage monitor implementation.
 */

#include "syn_power.h"
#include "../util/syn_assert.h"

#include <string.h>

void syn_power_init(SYN_Power *pwr, const SYN_Power_Config *cfg)
{
    SYN_ASSERT(pwr != NULL);
    SYN_ASSERT(cfg != NULL);
    SYN_ASSERT(cfg->adc != NULL);

    memset(pwr, 0, sizeof(*pwr));
    pwr->adc        = cfg->adc;
    pwr->on_brownout = cfg->on_brownout;
    pwr->on_restore  = cfg->on_restore;
    pwr->ctx         = cfg->ctx;

    /* Hysteresis: low threshold = brownout, high threshold = restore.
     * We invert the logic: brownout triggers when voltage drops BELOW,
     * so we use threshold = midpoint, band = half the gap. */
    int32_t mid  = (cfg->brownout_mv + cfg->restore_mv) / 2;
    int32_t band = (cfg->restore_mv - cfg->brownout_mv) / 2;
    if (band < 0) band = -band;
    syn_hyst_init(&pwr->hyst, mid, band, true);
}

void syn_power_update(SYN_Power *pwr)
{
    SYN_ASSERT(pwr != NULL);

    pwr->voltage_mv = syn_adc_read_mv(pwr->adc);

    /* Push to stats window */
    if (pwr->stats != NULL) {
        syn_signal_push(pwr->stats, pwr->voltage_mv);
    }

    /* Hysteresis returns true when above threshold (healthy) */
    bool was_brownout = pwr->brownout;
    bool healthy = syn_hyst_update(&pwr->hyst, pwr->voltage_mv);

    pwr->brownout = !healthy;

    if (pwr->brownout && !was_brownout) {
        /* Just entered brownout */
        if (pwr->errlog != NULL) {
            syn_errlog_record(pwr->errlog, SYN_POWER_ERR_BROWNOUT,
                               SYN_ERR_WARNING, (uint32_t)pwr->voltage_mv);
        }
        if (pwr->on_brownout != NULL) {
            pwr->on_brownout(pwr, pwr->ctx);
        }
    } else if (!pwr->brownout && was_brownout) {
        /* Just restored */
        if (pwr->on_restore != NULL) {
            pwr->on_restore(pwr, pwr->ctx);
        }
    }
}

void syn_power_set_stats(SYN_Power *pwr, SYN_Signal *stats)
{
    SYN_ASSERT(pwr != NULL);
    pwr->stats = stats;
}

void syn_power_set_errlog(SYN_Power *pwr, SYN_ErrLog *errlog)
{
    SYN_ASSERT(pwr != NULL);
    pwr->errlog = errlog;
}

#endif /* SYN_USE_POWER */
