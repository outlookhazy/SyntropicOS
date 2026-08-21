

# File syn\_timesync.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_timesync.h**](syn__timesync_8h.md)

[Go to the documentation of this file](syn__timesync_8h.md)


```C++

#ifndef SYN_TIMESYNC_H
#define SYN_TIMESYNC_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_TIMESYNC) || SYN_USE_TIMESYNC

#include "../common/syn_defs.h"
#include "syn_hpclock.h"

#if defined(SYN_USE_RTC) && SYN_USE_RTC
#include "syn_rtc.h"
#define SYN_TIMESYNC_HAS_RTC 1 
#else
#define SYN_TIMESYNC_HAS_RTC 0 
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Time Quality Source Tiers ─────────────────────────────────────────── */

typedef enum {
    SYN_TIME_SOURCE_UNSYNCED = 0,     
    SYN_TIME_SOURCE_RTC_SYNCED = 1,   
    SYN_TIME_SOURCE_GPS_HOLDOVER = 2, 
    SYN_TIME_SOURCE_GPS_PPS = 3,      
} SYN_TimeSource;

/* ── Structured UTC Output ──────────────────────────────────────────────── */

typedef struct {
    uint64_t sec;            
    uint32_t nsec;           
    uint32_t uncertainty_ns; 
    SYN_TimeSource source;   
} SYN_UTCTimestamp;

/* ── Context ────────────────────────────────────────────────────────────── */

typedef struct {
    /* Anchors */
    SYN_HPTimestamp last_pps_ts; 
    uint64_t last_pps_ticks;     
    uint64_t last_utc_sec;       
    /* Drift calculation */
    uint64_t prev_pps_ticks; 
    int32_t drift_ppm;       
    uint32_t pps_count;      
    /* Tuning / Config */
    uint32_t base_jitter_ns; 
    uint32_t max_holdover_s; 
    /* Status flags */
    bool has_pps_lock; 
    bool rtc_synced;   
} SYN_TimeSync;

/* ── Default Configuration Constants ───────────────────────────────────── */

#ifndef SYN_TIMESYNC_DEFAULT_JITTER_NS
#define SYN_TIMESYNC_DEFAULT_JITTER_NS 50U
#endif

#ifndef SYN_TIMESYNC_DEFAULT_HOLDOVER_S
#define SYN_TIMESYNC_DEFAULT_HOLDOVER_S 60U
#endif

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_timesync_init(SYN_TimeSync *tsync);

SYN_Status syn_timesync_bind_pps(SYN_TimeSync *tsync, const SYN_HPTimestamp *pps_ts,
                                 uint64_t utc_sec);

SYN_Status syn_timesync_resolve_utc(const SYN_TimeSync *tsync, const SYN_HPTimestamp *event_ts,
                                    SYN_UTCTimestamp *out_utc);

uint64_t syn_timesync_to_epoch_ns(const SYN_TimeSync *tsync, const SYN_HPTimestamp *event_ts);

bool syn_timesync_is_pps_locked(const SYN_TimeSync *tsync);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_TIMESYNC */

#endif /* SYN_TIMESYNC_H */
```


