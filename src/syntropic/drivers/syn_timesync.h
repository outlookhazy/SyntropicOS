/**
 * @file syn_timesync.h
 * @brief High-precision time discipline and clock synchronization service.
 *
 * `syn_timesync` converts raw high-precision hardware timestamps
 * (`SYN_HPTimestamp` from `syn_hpclock`) into universal UTC wall-clock
 * time with quantified uncertainty bounds (± nanoseconds).
 *
 * It combines an asynchronous 1 Hz pulse reference (such as a GPS PPS
 * signal) with an epoch date/time source (such as NMEA UTC sentences),
 * estimates hardware crystal oscillator drift in Parts-Per-Million (PPM),
 * and provides a 4-tier quality fallback hierarchy:
 *
 *   1. SYN_TIME_SOURCE_GPS_PPS       Active PPS lock (sub-microsecond UTC)
 *   2. SYN_TIME_SOURCE_GPS_HOLDOVER  PPS pulse lost; extrapolating with drift PPM
 *   3. SYN_TIME_SOURCE_RTC_SYNCED    Long outage; using hardware RTC (if enabled)
 *   4. SYN_TIME_SOURCE_UNSYNCED      No time reference; raw monotonic uptime
 *
 * ## Usage Example
 * @code
 *   static SYN_TimeSync tsync;
 *   syn_timesync_init(&tsync);
 *
 *   // In PPS ISR:
 *   SYN_HPCLOCK_CAPTURE(pps_ts);
 *
 *   // In GPS UART task (after NMEA UTC second parsed):
 *   syn_timesync_bind_pps(&tsync, &pps_ts, utc_epoch_seconds);
 *
 *   // Resolving an event timestamp:
 *   SYN_UTCTimestamp utc;
 *   syn_timesync_resolve_utc(&tsync, &event_ts, &utc);
 *   if (utc.source == SYN_TIME_SOURCE_GPS_PPS && utc.uncertainty_ns < 1000) {
 *       // High-precision event processing
 *   }
 * @endcode
 * @ingroup syn_drivers
 */

#ifndef SYN_TIMESYNC_H
#define SYN_TIMESYNC_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_TIMESYNC) || SYN_USE_TIMESYNC

#include "../common/syn_defs.h"
#include "syn_hpclock.h"

#if !defined(SYN_USE_RTC) || SYN_USE_RTC
#include "syn_rtc.h"
#define SYN_TIMESYNC_HAS_RTC 1 /**< RTC driver presence flag (1) */
#else
#define SYN_TIMESYNC_HAS_RTC 0 /**< RTC driver presence flag (0) */
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Time Quality Source Tiers ─────────────────────────────────────────── */

/**
 * @brief Quality tier of a resolved UTC timestamp.
 */
typedef enum {
    SYN_TIME_SOURCE_UNSYNCED = 0,     /**< No time reference; arbitrary/monotonic epoch */
    SYN_TIME_SOURCE_RTC_SYNCED = 1,   /**< Hardware RTC (coarse wall-clock date)        */
    SYN_TIME_SOURCE_GPS_HOLDOVER = 2, /**< PPS lost; extrapolating with crystal drift PPM */
    SYN_TIME_SOURCE_GPS_PPS = 3,      /**< Active PPS lock (sub-microsecond accuracy)   */
} SYN_TimeSource;

/* ── Structured UTC Output ──────────────────────────────────────────────── */

/**
 * @brief Universal UTC timestamp with error bound and quality tier.
 */
typedef struct {
    uint64_t sec;            /**< UTC epoch seconds (1970 base)          */
    uint32_t nsec;           /**< Sub-second nanoseconds (0..999999999)  */
    uint32_t uncertainty_ns; /**< Quantified error bound (± nanoseconds) */
    SYN_TimeSource source;   /**< Quality tier at resolution instant     */
} SYN_UTCTimestamp;

/* ── Context ────────────────────────────────────────────────────────────── */

/**
 * @brief Time sync discipline state context (caller-allocated).
 */
typedef struct {
    /* Anchors */
    SYN_HPTimestamp last_pps_ts; /**< Raw timestamp at last PPS pulse         */
    uint64_t last_pps_ticks;     /**< Resolved 64-bit ticks at last PPS       */
    uint64_t last_utc_sec;       /**< Universal UTC second at last PPS       */

    /* Drift calculation */
    uint64_t prev_pps_ticks; /**< Ticks at previous PPS for PPM calculation */
    int32_t drift_ppm;       /**< Measured crystal drift in Parts-Per-Million */
    uint32_t pps_count;      /**< Total valid PPS updates received         */

    /* Tuning / Config */
    uint32_t base_jitter_ns; /**< Base reference jitter (default: 50 ns)  */
    uint32_t max_holdover_s; /**< Max holdover before degrading to RTC    */

    /* Status flags */
    bool has_pps_lock; /**< true if at least one PPS is bound       */
    bool rtc_synced;   /**< true if RTC was previously set by GPS   */
} SYN_TimeSync;

/* ── Default Configuration Constants ───────────────────────────────────── */

/** Default assumed PPS reference jitter (50 nanoseconds). */
#ifndef SYN_TIMESYNC_DEFAULT_JITTER_NS
#define SYN_TIMESYNC_DEFAULT_JITTER_NS 50U
#endif

/** Default max holdover duration before degrading to RTC (60 seconds). */
#ifndef SYN_TIMESYNC_DEFAULT_HOLDOVER_S
#define SYN_TIMESYNC_DEFAULT_HOLDOVER_S 60U
#endif

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a time discipline context.
 *
 * Sets default jitter (50 ns) and max holdover (60 s).
 * If hardware RTC is enabled (`SYN_USE_RTC`), checks if the RTC is valid.
 *
 * @param tsync Context to initialize. Must not be NULL.
 */
void syn_timesync_init(SYN_TimeSync *tsync);

/**
 * @brief Bind a captured PPS timestamp to its corresponding UTC second.
 *
 * Call this when an asynchronous epoch time sentence (e.g. NMEA $GPZDA) is
 * received, pairing the integer UTC second with the PPS timestamp captured
 * at the top of that second.
 *
 * Measures crystal drift PPM against nominal syn_port_hpclock_freq_hz().
 *
 * @param tsync   TimeSync context. Must not be NULL.
 * @param pps_ts  Timestamp snapshot captured by ISR at PPS rising edge.
 * @param utc_sec Universal UTC epoch second corresponding to this PPS pulse.
 * @return SYN_OK on success, SYN_INVALID_PARAM if pps_ts is NULL.
 */
SYN_Status syn_timesync_bind_pps(SYN_TimeSync *tsync, const SYN_HPTimestamp *pps_ts,
                                 uint64_t utc_sec);

/**
 * @brief Resolve an arbitrary high-precision timestamp into UTC.
 *
 * Converts a raw SYN_HPTimestamp (captured in an ISR or main thread) into
 * absolute UTC time, calculating the sub-second offset, active quality tier,
 * and linear error accumulation bound (uncertainty_ns).
 *
 * Handles event timestamps occurring before or after the reference PPS pulse.
 *
 * @param tsync     TimeSync context. Must not be NULL.
 * @param event_ts  Event timestamp to resolve. Must not be NULL.
 * @param out_utc   Output UTC timestamp. Must not be NULL.
 * @return SYN_OK on success, SYN_INVALID_PARAM if args are NULL.
 */
SYN_Status syn_timesync_resolve_utc(const SYN_TimeSync *tsync, const SYN_HPTimestamp *event_ts,
                                    SYN_UTCTimestamp *out_utc);

/**
 * @brief Helper: resolve event timestamp directly to total 64-bit UTC nanoseconds.
 *
 * @param tsync     TimeSync context.
 * @param event_ts  Event timestamp to resolve.
 * @return Total nanoseconds since Unix epoch (1970-01-01 00:00:00 UTC).
 */
uint64_t syn_timesync_to_epoch_ns(const SYN_TimeSync *tsync, const SYN_HPTimestamp *event_ts);

/**
 * @brief Check if active PPS lock is present.
 * @param tsync  TimeSync context.
 * @return true if PPS lock is active (received within last 1.1s).
 */
bool syn_timesync_is_pps_locked(const SYN_TimeSync *tsync);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_TIMESYNC */

#endif /* SYN_TIMESYNC_H */
