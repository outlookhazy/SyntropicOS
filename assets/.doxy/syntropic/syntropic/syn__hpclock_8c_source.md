

# File syn\_hpclock.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_hpclock.c**](syn__hpclock_8c.md)

[Go to the documentation of this file](syn__hpclock_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_HPCLOCK) || SYN_USE_HPCLOCK

#include "../util/syn_assert.h"
#include "syn_hpclock.h"

/* ── Overflow counter (shared with port ISR) ───────────────────────────── */

volatile uint32_t syn_hpclock_msb;

/* ── Resolution ────────────────────────────────────────────────────────── */

uint64_t syn_hpclock_resolve(const SYN_HPTimestamp *ts)
{
    if (ts == NULL)
        return 0;

    SYN_ASSERT(ts != NULL);

    /*
     * Three reads: msb_1, lsb, msb_2.
     *
     * The timer overflow ISR (highest priority) can preempt at any
     * point during the capture.  Two cases:
     *
     *   msb_1 == msb_2:
     *       No overflow during the capture window.  Use either MSB.
     *
     *   msb_1 != msb_2:
     *       Overflow occurred during the window.  The hardware counter
     *       wrapped from 0xFFFFFFFF to 0x00000000 at some point between
     *       the msb_1 and msb_2 reads.  The LSB value tells us which
     *       side of the wrap it was captured on:
     *
     *       - lsb < 0x80000000:  The counter already wrapped before the
     *         LSB read.  lsb is a small post-wrap value.  Use msb_2
     *         (the post-overflow count).
     *
     *       - lsb >= 0x80000000: The counter had not yet wrapped when
     *         the LSB was read.  lsb is a large pre-wrap value.  Use
     *         msb_1 (the pre-overflow count).
     *
     * Safety of the half-range check:  the three reads complete in
     * ~10 CPU cycles.  Half a 32-bit counter period at any practical
     * system clock (≥ 1 MHz) is > 2000 seconds.  The LSB cannot
     * advance through half the range during the capture window.
     */
    uint32_t msb;

    if (ts->msb_1 == ts->msb_2) {
        /* No overflow in the capture window. */
        msb = ts->msb_1;
    } else {
        /* Overflow occurred — determine which side of the wrap. */
        msb = (ts->lsb < 0x80000000u) ? ts->msb_2  /* post-wrap  */
                                      : ts->msb_1; /* pre-wrap   */
    }

    return ((uint64_t)msb << 32) | (uint64_t)ts->lsb;
}

/* ── Conversion ────────────────────────────────────────────────────────── */

uint64_t syn_hpclock_ticks_to_ns(uint64_t ticks)
{
    uint32_t freq_hz = syn_port_hpclock_freq_hz();
    if (freq_hz == 0)
        return 0;

    /*
     * ns = ticks * 1,000,000,000 / freq_hz
     *
     * To avoid overflow in the multiply, split into whole seconds
     * and remainder ticks:
     *   whole_s    = ticks / freq_hz
     *   rem_ticks  = ticks % freq_hz
     *   ns = whole_s * 1,000,000,000 + rem_ticks * 1,000,000,000 / freq_hz
     *
     * The remainder multiply fits in 64 bits as long as
     * rem_ticks < freq_hz and freq_hz < 4.29 GHz (always true for
     * any practical MCU clock).
     */
    uint64_t whole_s = ticks / freq_hz;
    uint64_t rem_ticks = ticks % freq_hz;

    return whole_s * 1000000000ULL + rem_ticks * 1000000000ULL / freq_hz;
}

/* ── Elapsed ───────────────────────────────────────────────────────────── */

uint64_t syn_hpclock_elapsed(const SYN_HPTimestamp *start, const SYN_HPTimestamp *end)
{
    SYN_ASSERT(start != NULL);
    SYN_ASSERT(end != NULL);

    return syn_hpclock_resolve(end) - syn_hpclock_resolve(start);
}

#endif /* SYN_USE_HPCLOCK */
```


