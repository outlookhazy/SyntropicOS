/**
 * @file test_timesync.c
 * @brief Unity tests for syn_timesync — high-precision time discipline service.
 */

#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_hpclock.h"
#include "syntropic/drivers/syn_rtc.h"
#include "syntropic/drivers/syn_timesync.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

/* ── Test Suite ────────────────────────────────────────────────────────── */

static void test_timesync_init(void)
{
    SYN_TimeSync tsync;
    syn_timesync_init(&tsync);

    TEST_ASSERT_FALSE(tsync.has_pps_lock);
    TEST_ASSERT_EQUAL_UINT32(50, tsync.base_jitter_ns);
    TEST_ASSERT_EQUAL_UINT32(60, tsync.max_holdover_s);
}

static void test_timesync_unsynced_fallback(void)
{
    SYN_TimeSync tsync;
    syn_timesync_init(&tsync);

    SYN_HPTimestamp event_ts;
    event_ts.msb_1 = 0;
    event_ts.lsb = 16000; /* 16000 ticks = 1 ms */
    event_ts.msb_2 = 0;

    SYN_UTCTimestamp utc;
    SYN_Status st = syn_timesync_resolve_utc(&tsync, &event_ts, &utc);

    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(SYN_TIME_SOURCE_UNSYNCED, utc.source);
    TEST_ASSERT_EQUAL_UINT64(0, utc.sec);
    TEST_ASSERT_EQUAL_UINT32(1000000, utc.nsec); /* 1 ms = 1,000,000 ns */
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFU, utc.uncertainty_ns);
}

static void test_timesync_locked_resolution(void)
{
    SYN_TimeSync tsync;
    syn_timesync_init(&tsync);

    /* Bind PPS #1 at tick 1,000,000, UTC second 1,700,000,000 */
    SYN_HPTimestamp pps_ts;
    pps_ts.msb_1 = 0;
    pps_ts.lsb = 1000000;
    pps_ts.msb_2 = 0;

    uint64_t epoch_sec = 1700000000ULL;
    SYN_Status st = syn_timesync_bind_pps(&tsync, &pps_ts, epoch_sec);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_TRUE(tsync.has_pps_lock);

    /* Event occurs 8,000,000 ticks later (0.5 seconds at 16 MHz) */
    SYN_HPTimestamp event_ts;
    event_ts.msb_1 = 0;
    event_ts.lsb = 9000000;
    event_ts.msb_2 = 0;

    SYN_UTCTimestamp utc;
    st = syn_timesync_resolve_utc(&tsync, &event_ts, &utc);

    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(SYN_TIME_SOURCE_GPS_PPS, utc.source);
    TEST_ASSERT_EQUAL_UINT64(1700000000ULL, utc.sec);
    TEST_ASSERT_EQUAL_UINT32(500000000U, utc.nsec); /* 0.5s = 500,000,000 ns */
    TEST_ASSERT_EQUAL_UINT32(50, utc.uncertainty_ns);
}

static void test_timesync_event_before_pps(void)
{
    SYN_TimeSync tsync;
    syn_timesync_init(&tsync);

    /* Bind PPS at tick 10,000,000, UTC second 100 */
    SYN_HPTimestamp pps_ts;
    pps_ts.msb_1 = 0;
    pps_ts.lsb = 10000000;
    pps_ts.msb_2 = 0;

    syn_timesync_bind_pps(&tsync, &pps_ts, 100);

    /* Event occurs 1,600,000 ticks (100 ms) BEFORE the PPS anchor */
    SYN_HPTimestamp event_ts;
    event_ts.msb_1 = 0;
    event_ts.lsb = 8400000;
    event_ts.msb_2 = 0;

    SYN_UTCTimestamp utc;
    syn_timesync_resolve_utc(&tsync, &event_ts, &utc);

    /* Expected: UTC sec 99, 900,000,000 ns (99.9 seconds) */
    TEST_ASSERT_EQUAL_UINT64(99, utc.sec);
    TEST_ASSERT_EQUAL_UINT32(900000000U, utc.nsec);
    TEST_ASSERT_EQUAL(SYN_TIME_SOURCE_GPS_PPS, utc.source);
}

static void test_timesync_drift_ppm_measurement(void)
{
    SYN_TimeSync tsync;
    syn_timesync_init(&tsync);

    /* PPS 1 at tick 0, UTC sec 100 */
    SYN_HPTimestamp pps1 = {.msb_1 = 0, .lsb = 0, .msb_2 = 0};
    syn_timesync_bind_pps(&tsync, &pps1, 100);

    /* PPS 2 at tick 16,000,160 (+10 PPM instantaneous). First update sets 10 PPM directly. */
    SYN_HPTimestamp pps2 = {.msb_1 = 0, .lsb = 16000160, .msb_2 = 0};
    syn_timesync_bind_pps(&tsync, &pps2, 101);
    TEST_ASSERT_EQUAL_INT32(10, tsync.drift_ppm);

    /* PPS 3 at tick 32,000,640 (+30 PPM instantaneous).
     * EMA formula: (10 * 7 + 30) / 8 = 100 / 8 = 12 PPM.
     */
    SYN_HPTimestamp pps3 = {.msb_1 = 0, .lsb = 32000640, .msb_2 = 0};
    syn_timesync_bind_pps(&tsync, &pps3, 102);
    TEST_ASSERT_EQUAL_INT32(12, tsync.drift_ppm);
}

static void test_timesync_holdover_uncertainty_growth(void)
{
    SYN_TimeSync tsync;
    syn_timesync_init(&tsync);

    /* Bind PPS 1 & 2 to establish 10 PPM drift */
    SYN_HPTimestamp pps1, pps2;
    pps1.msb_1 = 0;
    pps1.lsb = 0;
    pps1.msb_2 = 0;
    pps2.msb_1 = 0;
    pps2.lsb = 16000160;
    pps2.msb_2 = 0;

    syn_timesync_bind_pps(&tsync, &pps1, 100);
    syn_timesync_bind_pps(&tsync, &pps2, 101);

    /* Event 5 seconds into holdover (16MHz * 5 = 80,000,000 ticks past pps2) */
    SYN_HPTimestamp event_ts;
    event_ts.msb_1 = 0;
    event_ts.lsb = 16000160 + 80000000;
    event_ts.msb_2 = 0;

    SYN_UTCTimestamp utc;
    syn_timesync_resolve_utc(&tsync, &event_ts, &utc);

    TEST_ASSERT_EQUAL(SYN_TIME_SOURCE_GPS_HOLDOVER, utc.source);
    TEST_ASSERT_EQUAL_UINT64(106, utc.sec);

    /* Uncertainty: base (50ns) + drift (10 PPM * 5s * 1000ns = 50,000ns) = 50,050ns */
    TEST_ASSERT_EQUAL_UINT32(50050U, utc.uncertainty_ns);
}

static void test_timesync_holdover_expiration(void)
{
    SYN_TimeSync tsync;
    syn_timesync_init(&tsync);
    tsync.max_holdover_s = 10; /* Shorten holdover to 10s for test */

    SYN_HPTimestamp pps;
    pps.msb_1 = 0;
    pps.lsb = 0;
    pps.msb_2 = 0;
    syn_timesync_bind_pps(&tsync, &pps, 1700000100ULL);

    /* Event 15 seconds after PPS (exceeds 10s max holdover) */
    SYN_HPTimestamp event_ts;
    event_ts.msb_1 = 0;
    event_ts.lsb = 16000000UL * 15;
    event_ts.msb_2 = 0;

    SYN_UTCTimestamp utc;
    syn_timesync_resolve_utc(&tsync, &event_ts, &utc);

#if SYN_TIMESYNC_HAS_RTC
    /* With RTC disciplined during bind_pps, expired holdover degrades to Tier 3 (RTC_SYNCED) */
    TEST_ASSERT_EQUAL(SYN_TIME_SOURCE_RTC_SYNCED, utc.source);
    TEST_ASSERT_EQUAL_UINT32(1000000000U, utc.uncertainty_ns);

    /* Without RTC sync, expired holdover degrades to Tier 4 (UNSYNCED) */
    tsync.rtc_synced = false;
    syn_timesync_resolve_utc(&tsync, &event_ts, &utc);
#endif
    TEST_ASSERT_EQUAL(SYN_TIME_SOURCE_UNSYNCED, utc.source);
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFU, utc.uncertainty_ns);
}

#if SYN_TIMESYNC_HAS_RTC
static void test_timesync_rtc_fallback(void)
{
    /* Pre-populate valid RTC date/time prior to syn_timesync_init */
    mock_rtc_time.year = 2025;
    mock_rtc_time.month = 6;
    mock_rtc_time.day = 15;
    mock_rtc_time.weekday = 0;
    mock_rtc_time.hour = 12;
    mock_rtc_time.minute = 0;
    mock_rtc_time.second = 0;

    uint32_t expected_epoch = syn_rtc_to_epoch(&mock_rtc_time);

    SYN_TimeSync tsync;
    syn_timesync_init(&tsync);
    TEST_ASSERT_TRUE(tsync.rtc_synced);
    TEST_ASSERT_FALSE(tsync.has_pps_lock);

    SYN_HPTimestamp event_ts = {.msb_1 = 0, .lsb = 16000, .msb_2 = 0};
    SYN_UTCTimestamp utc;

    /* Before PPS lock, valid RTC resolves at Tier 3 (SYN_TIME_SOURCE_RTC_SYNCED) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_timesync_resolve_utc(&tsync, &event_ts, &utc));
    TEST_ASSERT_EQUAL(SYN_TIME_SOURCE_RTC_SYNCED, utc.source);
    TEST_ASSERT_EQUAL_UINT64(expected_epoch, utc.sec);
    TEST_ASSERT_EQUAL_UINT32(0U, utc.nsec);
    TEST_ASSERT_EQUAL_UINT32(1000000000U, utc.uncertainty_ns);

    /* Simulate RTC hardware read failure: degrades cleanly to Tier 4 (UNSYNCED) */
    mock_rtc_init_ok = false;
    TEST_ASSERT_EQUAL(SYN_OK, syn_timesync_resolve_utc(&tsync, &event_ts, &utc));
    TEST_ASSERT_EQUAL(SYN_TIME_SOURCE_UNSYNCED, utc.source);
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFU, utc.uncertainty_ns);

    /* Also verify bind_pps handles RTC set failure gracefully */
    SYN_HPTimestamp pps = {.msb_1 = 0, .lsb = 32000, .msb_2 = 0};
    tsync.rtc_synced = false;
    TEST_ASSERT_EQUAL(SYN_OK, syn_timesync_bind_pps(&tsync, &pps, 1700000200ULL));
    TEST_ASSERT_FALSE(tsync.rtc_synced);
    mock_rtc_init_ok = true;
}
#endif

static void test_timesync_to_epoch_ns_helper(void)
{
    SYN_TimeSync tsync;
    syn_timesync_init(&tsync);

    SYN_HPTimestamp pps;
    pps.msb_1 = 0;
    pps.lsb = 0;
    pps.msb_2 = 0;
    syn_timesync_bind_pps(&tsync, &pps, 10); /* UTC sec = 10 */

    SYN_HPTimestamp event_ts;
    event_ts.msb_1 = 0;
    event_ts.lsb = 16000; /* 1 ms = 1,000,000 ns */
    event_ts.msb_2 = 0;

    uint64_t epoch_ns = syn_timesync_to_epoch_ns(&tsync, &event_ts);
    /* 10 seconds * 1e9 + 1,000,000 ns = 10,001,000,000 ns */
    TEST_ASSERT_EQUAL_UINT64(10001000000ULL, epoch_ns);
}

static void test_timesync_null_params(void)
{
    syn_timesync_init(NULL);

    SYN_TimeSync tsync;
    syn_timesync_init(&tsync);

    SYN_HPTimestamp ts;
    ts.msb_1 = 0;
    ts.lsb = 100;
    ts.msb_2 = 0;
    SYN_UTCTimestamp utc;

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_timesync_bind_pps(NULL, &ts, 100));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_timesync_bind_pps(&tsync, NULL, 100));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_timesync_resolve_utc(NULL, &ts, &utc));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_timesync_resolve_utc(&tsync, NULL, &utc));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_timesync_resolve_utc(&tsync, &ts, NULL));

    TEST_ASSERT_EQUAL_UINT64(0, syn_timesync_to_epoch_ns(NULL, &ts));
}

static void test_timesync_is_pps_locked(void)
{
    TEST_ASSERT_FALSE(syn_timesync_is_pps_locked(NULL));

    SYN_TimeSync tsync;
    syn_timesync_init(&tsync);
    TEST_ASSERT_FALSE(syn_timesync_is_pps_locked(&tsync));

    SYN_HPTimestamp pps;
    pps.msb_1 = 0;
    pps.lsb = 0;
    pps.msb_2 = 0;
    syn_timesync_bind_pps(&tsync, &pps, 100);

    TEST_ASSERT_TRUE(syn_timesync_is_pps_locked(&tsync));
}

void run_timesync_tests(void)
{
    syn_hpclock_msb = 0;
    RUN_TEST(test_timesync_init);
    RUN_TEST(test_timesync_unsynced_fallback);
    RUN_TEST(test_timesync_locked_resolution);
    RUN_TEST(test_timesync_event_before_pps);
    RUN_TEST(test_timesync_drift_ppm_measurement);
    RUN_TEST(test_timesync_holdover_uncertainty_growth);
    RUN_TEST(test_timesync_holdover_expiration);
#if SYN_TIMESYNC_HAS_RTC
    RUN_TEST(test_timesync_rtc_fallback);
#endif
    RUN_TEST(test_timesync_to_epoch_ns_helper);
    RUN_TEST(test_timesync_null_params);
    RUN_TEST(test_timesync_is_pps_locked);
}
