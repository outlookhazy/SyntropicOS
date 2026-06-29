#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_HWWDT) || SYN_USE_HWWDT

/**
 * @file syn_hwwdt.c
 * @brief Hardware Watchdog Timer driver implementation.
 */

#include "syn_hwwdt.h"
#include "../port/syn_port_wdt.h"
#include "../util/syn_assert.h"

SYN_Status syn_hwwdt_init(uint32_t timeout_ms)
{
    SYN_ASSERT(timeout_ms > 0u);
    return syn_port_wdt_init(timeout_ms);
}

void syn_hwwdt_feed(void)
{
    syn_port_wdt_feed();
}

#endif /* SYN_USE_HWWDT */
