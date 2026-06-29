#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_DAC) || SYN_USE_DAC

/**
 * @file syn_dac.c
 * @brief DAC driver implementation.
 */

#include "syn_dac.h"
#include "../util/syn_assert.h"

SYN_Status syn_dac_init(SYN_DAC *dac, uint8_t channel)
{
    SYN_ASSERT(dac != NULL);
    dac->channel = channel;
    return syn_port_dac_init(channel);
}

SYN_Status syn_dac_write_raw(const SYN_DAC *dac, uint16_t raw)
{
    SYN_ASSERT(dac != NULL);
    return syn_port_dac_write(dac->channel, raw);
}

SYN_Status syn_dac_write_mv(const SYN_DAC *dac, uint16_t mv)
{
    SYN_ASSERT(dac != NULL);
    uint16_t ref_mv = syn_port_dac_reference_mv();
    uint16_t max    = syn_dac_max_raw(dac);
    /* Clamp to reference */
    if (mv > ref_mv) {
        mv = ref_mv;
    }
    /* raw = mv * max / ref_mv — use 32-bit intermediate to avoid overflow */
    uint16_t raw = (uint16_t)(((uint32_t)mv * (uint32_t)max) / (uint32_t)ref_mv);
    return syn_port_dac_write(dac->channel, raw);
}

SYN_Status syn_dac_write_percent(const SYN_DAC *dac, uint8_t percent)
{
    SYN_ASSERT(dac != NULL);
    if (percent > 100u) {
        percent = 100u;
    }
    uint16_t max = syn_dac_max_raw(dac);
    uint16_t raw = (uint16_t)(((uint32_t)percent * (uint32_t)max) / 100u);
    return syn_port_dac_write(dac->channel, raw);
}

uint16_t syn_dac_max_raw(const SYN_DAC *dac)
{
    SYN_ASSERT(dac != NULL);
    (void)dac;
    uint8_t res = syn_port_dac_resolution();
    return (uint16_t)((1u << res) - 1u);
}

#endif /* SYN_USE_DAC */
