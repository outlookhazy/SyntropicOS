/**
 * @file syn_random.c
 * @brief Random number utility implementation.
 */

#include "syn_random.h"
#include "../port/syn_port_system.h"
#include <string.h>

SYN_Status syn_random_fill(void *buf, size_t len)
{
    if (!buf || len == 0) return SYN_OK;
    return syn_port_random_fill(buf, len);
}

uint32_t syn_random_u32(void)
{
    uint32_t val = 0;
    syn_random_fill(&val, sizeof(val));
    return val;
}

uint32_t syn_random_range(uint32_t min, uint32_t max)
{
    if (min >= max) return min;
    uint32_t range = max - min + 1;
    return min + (syn_random_u32() % range);
}

/* Weak fallback: NOT SECURE. Platform port must override this. */
SYN_WEAK SYN_Status syn_port_random_fill(void *buf, size_t len)
{
    return syn_random_fallback_fill(buf, len);
}

/**
 * @brief Internal: XOR-shift fallback for when no hardware entropy is available.
 * @param buf Output buffer.
 * @param len Length in bytes.
 * @return SYN_OK.
 */
SYN_Status syn_random_fallback_fill(void *buf, size_t len)
{
    uint8_t *p = (uint8_t *)buf;
    static uint32_t state = 0x87654321;
    
    for (size_t i = 0; i < len; i++) {
        /* Simple XOR-shift for fallback */
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        p[i] = (uint8_t)(state ^ syn_port_get_tick_ms());
    }
    
    return SYN_OK;
}
