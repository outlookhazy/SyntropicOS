

# File syn\_timesync.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_timesync.c**](syn__timesync_8c.md)

[Go to the documentation of this file](syn__timesync_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_TIMESYNC) || SYN_USE_TIMESYNC

#include "../util/syn_assert.h"
#include "syn_timesync.h"

#include <stdlib.h>
#include <string.h>

/* ── Init ───────────────────────────────────────────────────────────────── */

void syn_timesync_init(SYN_TimeSync *tsync)
{
    SYN_ASSERT(tsync != NULL);
    if (tsync == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL guard after SYN_ASSERT in release builds */
    }

    memset(tsync, 0, sizeof(SYN_TimeSync));
    tsync->base_jitter_ns = SYN_TIMESYNC_DEFAULT_JITTER_NS;
    tsync->max_holdover_s = SYN_TIMESYNC_DEFAULT_HOLDOVER_S;

#if SYN_TIMESYNC_HAS_RTC
    /* If hardware RTC is configured and valid, mark RTC as available */
    SYN_RTC_DateTime dt;
    if (syn_rtc_get(&dt) == SYN_OK && syn_rtc_is_valid(&dt)) {
        tsync->rtc_synced = true;
    }
#endif
}

/* ── Bind PPS ───────────────────────────────────────────────────────────── */

SYN_Status syn_timesync_bind_pps(SYN_TimeSync *tsync, const SYN_HPTimestamp *pps_ts,
                                 uint64_t utc_sec)
{
    if (tsync == NULL || pps_ts == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint64_t curr_ticks = syn_hpclock_resolve(pps_ts);
    uint32_t nominal_freq = syn_port_hpclock_freq_hz();

    if (tsync->has_pps_lock && tsync->pps_count > 0 && nominal_freq > 0) {
        /*
         * Measure actual ticks elapsed between PPS pulses.
         * For a 1-second interval, expected ticks = nominal_freq.
         * Calculate crystal drift PPM = ((actual - expected) * 1,000,000) / expected.
         */
        uint64_t actual_ticks = curr_ticks - tsync->last_pps_ticks;
        int64_t diff_ticks = (int64_t)actual_ticks - (int64_t)nominal_freq;

        /* Convert tick difference to Parts-Per-Million */
        int32_t new_ppm = (int32_t)((diff_ticks * 1000000LL) / (int64_t)nominal_freq);

        if (tsync->pps_count == 1 || tsync->drift_ppm == 0) {
            tsync->drift_ppm = new_ppm;
        } else {
            /* 8-sample Exponential Moving Average (EMA) to smooth out PPS jitter */
            tsync->drift_ppm = (int32_t)(((int64_t)tsync->drift_ppm * 7 + new_ppm) / 8);
        }
    }

    /* Update anchors */
    tsync->prev_pps_ticks = tsync->last_pps_ticks;
    tsync->last_pps_ts = *pps_ts;
    tsync->last_pps_ticks = curr_ticks;
    tsync->last_utc_sec = utc_sec;
    tsync->pps_count++;
    tsync->has_pps_lock = true;

#if SYN_TIMESYNC_HAS_RTC
    /* Discipline hardware RTC to GPS time if RTC is enabled */
    SYN_RTC_DateTime dt;
    syn_rtc_from_epoch((uint32_t)utc_sec, &dt);
    if (syn_rtc_set(&dt) == SYN_OK) {
        tsync->rtc_synced = true;
    }
#endif

    return SYN_OK;
}

/* ── Resolve UTC ────────────────────────────────────────────────────────── */

SYN_Status syn_timesync_resolve_utc(const SYN_TimeSync *tsync, const SYN_HPTimestamp *event_ts,
                                    SYN_UTCTimestamp *out_utc)
{
    if (tsync == NULL || event_ts == NULL || out_utc == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint64_t event_ticks = syn_hpclock_resolve(event_ts);
    uint32_t freq_hz = syn_port_hpclock_freq_hz();

    if (!tsync->has_pps_lock || freq_hz == 0) {
        /*
         * Tier 4 / Tier 3 Fallback: No active PPS lock ever acquired.
         */
#if SYN_TIMESYNC_HAS_RTC
        if (tsync->rtc_synced) {
            SYN_RTC_DateTime dt;
            if (syn_rtc_get(&dt) == SYN_OK) {
                out_utc->sec = syn_rtc_to_epoch(&dt);
                out_utc->nsec = 0;
                out_utc->uncertainty_ns = 1000000000U; /* ±1 second RTC precision */
                out_utc->source = SYN_TIME_SOURCE_RTC_SYNCED;
                return SYN_OK;
            }
        }
#endif
        /* Unsynced fallback: Monotonic ticks converted to nanoseconds */
        uint64_t total_ns = syn_hpclock_ticks_to_ns(event_ticks);
        out_utc->sec = total_ns / 1000000000ULL;
        out_utc->nsec = (uint32_t)(total_ns % 1000000000ULL);
        out_utc->uncertainty_ns = 0xFFFFFFFFU; /* Infinite / undefined error */
        out_utc->source = SYN_TIME_SOURCE_UNSYNCED;
        return SYN_OK;
    }

    /* Compute tick offset relative to last PPS anchor */
    int64_t tick_delta = (int64_t)event_ticks - (int64_t)tsync->last_pps_ticks;
    int64_t ns_delta;

    if (tick_delta >= 0) {
        ns_delta = (int64_t)syn_hpclock_ticks_to_ns((uint64_t)tick_delta);
    } else {
        ns_delta = -(int64_t)syn_hpclock_ticks_to_ns((uint64_t)(-tick_delta));
    }

    /* Compute total seconds and sub-second nanoseconds */
    int64_t sec_offset = ns_delta / 1000000000LL;
    int64_t nsec_rem = ns_delta % 1000000000LL;

    if (nsec_rem < 0) {
        sec_offset -= 1;
        nsec_rem += 1000000000LL;
    }

    out_utc->sec = (uint64_t)((int64_t)tsync->last_utc_sec + sec_offset);
    out_utc->nsec = (uint32_t)nsec_rem;

    /*
     * Quality Tier & Uncertainty Evaluation
     */
    uint64_t elapsed_s = (tick_delta >= 0) ? (uint64_t)tick_delta / freq_hz : 0;

    if (elapsed_s <= 1) {
        /* Tier 1: Active PPS lock within 1 second */
        out_utc->source = SYN_TIME_SOURCE_GPS_PPS;
        out_utc->uncertainty_ns = tsync->base_jitter_ns;
    } else if (elapsed_s <= tsync->max_holdover_s) {
        /* Tier 2: PPS holdover — error accumulates with drift */
        out_utc->source = SYN_TIME_SOURCE_GPS_HOLDOVER;
        uint32_t abs_drift = (uint32_t)abs(tsync->drift_ppm);
        out_utc->uncertainty_ns = tsync->base_jitter_ns + (abs_drift * (uint32_t)elapsed_s * 1000U);
    } else {
        /* Tier 3 or 4: Holdover expired */
#if SYN_TIMESYNC_HAS_RTC
        if (tsync->rtc_synced) {
            out_utc->source = SYN_TIME_SOURCE_RTC_SYNCED;
            out_utc->uncertainty_ns = 1000000000U;
        } else {
            out_utc->source = SYN_TIME_SOURCE_UNSYNCED;
            out_utc->uncertainty_ns = 0xFFFFFFFFU;
        }
#else
        out_utc->source = SYN_TIME_SOURCE_UNSYNCED;
        out_utc->uncertainty_ns = 0xFFFFFFFFU;
#endif
    }

    return SYN_OK;
}

/* ── Epoch Nanoseconds Helper ───────────────────────────────────────────── */

uint64_t syn_timesync_to_epoch_ns(const SYN_TimeSync *tsync, const SYN_HPTimestamp *event_ts)
{
    SYN_UTCTimestamp utc;
    if (syn_timesync_resolve_utc(tsync, event_ts, &utc) != SYN_OK) {
        return 0;
    }
    return utc.sec * 1000000000ULL + (uint64_t)utc.nsec;
}

/* ── PPS Lock Check ─────────────────────────────────────────────────────── */

bool syn_timesync_is_pps_locked(const SYN_TimeSync *tsync)
{
    if (tsync == NULL || !tsync->has_pps_lock) {
        return false;
    }
    /* Simple sanity check: has at least one PPS been bound */
    return true;
}

#endif /* SYN_USE_TIMESYNC */
```


