/**
 * @file test_rtc.c
 * @brief Unity tests for syn_rtc (Real-Time Clock driver).
 *
 * Epoch verification:
 *   1970-01-01 00:00:00 UTC -> 0
 *   2000-01-01 00:00:00 UTC -> 946684800
 *      (30*365 + 7 leap years) * 86400 = 10957 * 86400 = 946684800
 *   2000-02-29 00:00:00 UTC -> 946684800 + (31+28)*86400 = 946684800 + 5097600
 *      Leap day: 2000 is leap (div 400), so Feb has 29 days.
 *      2000-02-29 = epoch 951782400  (946684800 + 59*86400)
 */

#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_rtc.h"
#include "unity/unity.h"

/* ── test 1: init delegates to port ─────────────────────────────────────── */

static void test_rtc_init_ok(void)
{
    mock_rtc_init_ok = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_init());
}

static void test_rtc_init_fail(void)
{
    mock_rtc_init_ok = false;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_rtc_init());
}

/* ── test 2: set / get roundtrip ─────────────────────────────────────────── */

static void test_rtc_set_get(void)
{
    SYN_RTC_DateTime set_dt = {.year = 2025u,
                               .month = 6u,
                               .day = 15u,
                               .weekday = 7u,
                               .hour = 10u,
                               .minute = 30u,
                               .second = 45u};
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_set(&set_dt));

    SYN_RTC_DateTime got_dt;
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_get(&got_dt));
    TEST_ASSERT_EQUAL_UINT16(2025u, got_dt.year);
    TEST_ASSERT_EQUAL_UINT8(6u, got_dt.month);
    TEST_ASSERT_EQUAL_UINT8(15u, got_dt.day);
    TEST_ASSERT_EQUAL_UINT8(10u, got_dt.hour);
    TEST_ASSERT_EQUAL_UINT8(30u, got_dt.minute);
    TEST_ASSERT_EQUAL_UINT8(45u, got_dt.second);
}

/* ── test 3: set rejects invalid dates ──────────────────────────────────── */

static void test_rtc_set_invalid_month(void)
{
    SYN_RTC_DateTime bad = {
        .year = 2025u, .month = 13u, .day = 1u, .hour = 0u, .minute = 0u, .second = 0u};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_rtc_set(&bad));
}

static void test_rtc_set_invalid_day(void)
{
    SYN_RTC_DateTime bad = {.year = 2025u,
                            .month = 2u,
                            .day = 29u,
                            .hour = 0u,
                            .minute = 0u,
                            .second = 0u}; /* 2025 not leap */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_rtc_set(&bad));
}

static void test_rtc_set_invalid_hour(void)
{
    SYN_RTC_DateTime bad = {
        .year = 2025u, .month = 1u, .day = 1u, .hour = 24u, .minute = 0u, .second = 0u};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_rtc_set(&bad));
}

/* ── test 4: is_valid ───────────────────────────────────────────────────── */

static void test_rtc_is_valid_ok(void)
{
    SYN_RTC_DateTime dt = {.year = 2024u,
                           .month = 2u,
                           .day = 29u,
                           .hour = 23u,
                           .minute = 59u,
                           .second = 59u}; /* 2024 = leap */
    TEST_ASSERT_TRUE(syn_rtc_is_valid(&dt));
}

static void test_rtc_is_valid_reject_zero_month(void)
{
    SYN_RTC_DateTime dt = {
        .year = 2024u, .month = 0u, .day = 1u, .hour = 0u, .minute = 0u, .second = 0u};
    TEST_ASSERT_FALSE(syn_rtc_is_valid(&dt));

    SYN_RTC_DateTime bad_min = {
        .year = 2024u, .month = 1u, .day = 1u, .hour = 0u, .minute = 60u, .second = 0u};
    TEST_ASSERT_FALSE(syn_rtc_is_valid(&bad_min));

    SYN_RTC_DateTime bad_sec = {
        .year = 2024u, .month = 1u, .day = 1u, .hour = 0u, .minute = 0u, .second = 60u};
    TEST_ASSERT_FALSE(syn_rtc_is_valid(&bad_sec));

    SYN_RTC_DateTime bad_weekday = {.year = 2024u,
                                    .month = 1u,
                                    .day = 1u,
                                    .weekday = 8u,
                                    .hour = 0u,
                                    .minute = 0u,
                                    .second = 0u};
    TEST_ASSERT_FALSE(syn_rtc_is_valid(&bad_weekday));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_rtc_set_alarm(NULL));
}

/* ── test 5: epoch conversion — known values ────────────────────────────── */

static void test_rtc_epoch_unix_origin(void)
{
    SYN_RTC_DateTime dt = {
        .year = 1970u, .month = 1u, .day = 1u, .hour = 0u, .minute = 0u, .second = 0u};
    TEST_ASSERT_EQUAL_UINT32(0u, syn_rtc_to_epoch(&dt));
}

static void test_rtc_epoch_y2k(void)
{
    /* 2000-01-01 00:00:00 UTC = 946684800 */
    SYN_RTC_DateTime dt = {
        .year = 2000u, .month = 1u, .day = 1u, .hour = 0u, .minute = 0u, .second = 0u};
    TEST_ASSERT_EQUAL_UINT32(946684800UL, syn_rtc_to_epoch(&dt));
}

static void test_rtc_epoch_leap_day(void)
{
    /* 2000-02-29 is a valid date (2000 is divisible by 400 -> leap) */
    SYN_RTC_DateTime dt = {
        .year = 2000u, .month = 2u, .day = 29u, .hour = 0u, .minute = 0u, .second = 0u};
    /* Days from epoch: 10957 (to 2000) + 31 (Jan) + 28 (Feb 1-28) = 11016
     * -> 11016 * 86400 = 951782400 */
    TEST_ASSERT_EQUAL_UINT32(951782400UL, syn_rtc_to_epoch(&dt));
}

/* ── test 6: from_epoch roundtrip ───────────────────────────────────────── */

static void test_rtc_from_epoch_roundtrip(void)
{
    SYN_RTC_DateTime orig = {.year = 2025u,
                             .month = 6u,
                             .day = 27u,
                             .weekday = 5u,
                             .hour = 23u,
                             .minute = 15u,
                             .second = 30u};
    uint32_t epoch = syn_rtc_to_epoch(&orig);

    SYN_RTC_DateTime back;
    syn_rtc_from_epoch(epoch, &back);

    TEST_ASSERT_EQUAL_UINT16(orig.year, back.year);
    TEST_ASSERT_EQUAL_UINT8(orig.month, back.month);
    TEST_ASSERT_EQUAL_UINT8(orig.day, back.day);
    TEST_ASSERT_EQUAL_UINT8(orig.weekday, back.weekday);
    TEST_ASSERT_EQUAL_UINT8(orig.hour, back.hour);
    TEST_ASSERT_EQUAL_UINT8(orig.minute, back.minute);
    TEST_ASSERT_EQUAL_UINT8(orig.second, back.second);
}

static void test_rtc_from_epoch_zero(void)
{
    SYN_RTC_DateTime dt;
    syn_rtc_from_epoch(0u, &dt);
    TEST_ASSERT_EQUAL_UINT16(1970u, dt.year);
    TEST_ASSERT_EQUAL_UINT8(1u, dt.month);
    TEST_ASSERT_EQUAL_UINT8(1u, dt.day);
    TEST_ASSERT_EQUAL_UINT8(4u, dt.weekday); /* 1970-01-01 = Thursday */
    TEST_ASSERT_EQUAL_UINT8(0u, dt.hour);
    TEST_ASSERT_EQUAL_UINT8(0u, dt.minute);
    TEST_ASSERT_EQUAL_UINT8(0u, dt.second);
}

static void test_rtc_drift_ppm(void)
{
    syn_rtc_init();
    syn_rtc_set_drift_ppm(-1000000); /* Must be rejected (no division by zero) */
    syn_rtc_set_drift_ppm(2000000);  /* Must be rejected */
    syn_rtc_set_drift_ppm(10);       /* +10 ppm drift compensation */

    SYN_RTC_DateTime set_dt = {
        .year = 2025u, .month = 6u, .day = 15u, .hour = 10u, .minute = 30u, .second = 45u};
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_set(&set_dt));

    SYN_RTC_DateTime got_dt;
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_get(&got_dt));

    /* Test get failure when port rtc fails */
    mock_rtc_init_ok = false;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_rtc_get(&got_dt));
    mock_rtc_init_ok = true;

    SYN_RTC_DateTime bad_year = {
        .year = 1969u, .month = 1u, .day = 1u, .hour = 0u, .minute = 0u, .second = 0u};
    TEST_ASSERT_FALSE(syn_rtc_is_valid(&bad_year));
}

static void test_rtc_schedule_alarm(void)
{
    mock_rtc_init_ok = true;
    syn_rtc_init();
    SYN_RTC_DateTime now = {
        .year = 2026u, .month = 7u, .day = 25u, .hour = 10u, .minute = 0u, .second = 0u};
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_set(&now));

    SYN_RTC_DateTime alarm_dt;
    /* Schedule alarm 3600 seconds (1 hour) from now: 10:00 -> 11:00 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_schedule_alarm_seconds(3600, &alarm_dt));

    TEST_ASSERT_EQUAL_UINT16(2026u, alarm_dt.year);
    TEST_ASSERT_EQUAL_UINT8(7u, alarm_dt.month);
    TEST_ASSERT_EQUAL_UINT8(25u, alarm_dt.day);
    TEST_ASSERT_EQUAL_UINT8(11u, alarm_dt.hour);
    TEST_ASSERT_EQUAL_UINT8(0u, alarm_dt.minute);
    TEST_ASSERT_EQUAL_UINT8(0u, alarm_dt.second);
}

static void test_rtc_schedule_alarm_fail(void)
{
    mock_rtc_init_ok = false;
    SYN_RTC_DateTime alarm_dt;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_rtc_schedule_alarm_seconds(60, &alarm_dt));
    mock_rtc_init_ok = true;
}

static void test_r5_rtc_drift_epoch_anchor(void)
{
    mock_rtc_init_ok = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_init());

    /* NULL and hardware-fail paths */
    TEST_ASSERT_FALSE(syn_rtc_is_valid(NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_rtc_set(NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_rtc_get(NULL));

    SYN_RTC_DateTime dt2026 = {
        .year = 2026u, .month = 7u, .day = 25u, .hour = 12u, .minute = 0u, .second = 0u};
    uint32_t epoch_2026 = syn_rtc_to_epoch(&dt2026);

    mock_rtc_init_ok = false;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_rtc_set(&dt2026));
    /* Lazy anchor path when drift_ppm set before valid RTC read */
    syn_rtc_set_drift_ppm(2);
    mock_rtc_init_ok = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_port_rtc_set(&dt2026));

    SYN_RTC_DateTime got;
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_get(&got));
    TEST_ASSERT_EQUAL_UINT32(epoch_2026, syn_rtc_to_epoch(&got));

    /* R5-S13: Explicit syn_rtc_set + +2 ppm must NOT jump 56.5 years of drift at t=0 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_init());
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_set(&dt2026));
    syn_rtc_set_drift_ppm(2);
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_get(&got));
    TEST_ASSERT_EQUAL_UINT32(epoch_2026, syn_rtc_to_epoch(&got));

    /* Simulate 1,000,002 raw hardware ticks (+2 ppm fast over 1,000,000 real seconds) */
    SYN_RTC_DateTime raw_advanced;
    syn_rtc_from_epoch(epoch_2026 + 1000002U, &raw_advanced);
    TEST_ASSERT_EQUAL(SYN_OK, syn_port_rtc_set(&raw_advanced));
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_get(&got));
    TEST_ASSERT_EQUAL_UINT32(epoch_2026 + 1000000U, syn_rtc_to_epoch(&got));

    /* Clamp to 0 when raw hardware time precedes anchor near Unix origin */
    SYN_RTC_DateTime dt_origin_plus5 = {
        .year = 1970u, .month = 1u, .day = 1u, .hour = 0u, .minute = 0u, .second = 5u};
    SYN_RTC_DateTime dt_origin = {
        .year = 1970u, .month = 1u, .day = 1u, .hour = 0u, .minute = 0u, .second = 0u};
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_set(&dt_origin_plus5));
    syn_rtc_set_drift_ppm(-500000);
    TEST_ASSERT_EQUAL(SYN_OK, syn_port_rtc_set(&dt_origin));
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_get(&got));
    TEST_ASSERT_EQUAL_UINT32(0U, syn_rtc_to_epoch(&got));

    /* Schedule alarm with NULL alarm_dt output pointer */
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_init());
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_set(&dt2026));
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_schedule_alarm_seconds(120U, NULL));

    /* Hardware RTC already valid before syn_rtc_init -> syn_rtc_set_drift_ppm snapshots anchor */
    TEST_ASSERT_EQUAL(SYN_OK, syn_port_rtc_set(&dt2026));
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_init());
    syn_rtc_set_drift_ppm(5);
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_get(&got));
    TEST_ASSERT_EQUAL_UINT32(epoch_2026, syn_rtc_to_epoch(&got));
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_init());
}

void run_rtc_tests(void)
{
    RUN_TEST(test_rtc_init_ok);
    RUN_TEST(test_rtc_init_fail);
    RUN_TEST(test_rtc_set_get);
    RUN_TEST(test_rtc_set_invalid_month);
    RUN_TEST(test_rtc_set_invalid_day);
    RUN_TEST(test_rtc_set_invalid_hour);
    RUN_TEST(test_rtc_is_valid_ok);
    RUN_TEST(test_rtc_is_valid_reject_zero_month);
    RUN_TEST(test_rtc_epoch_unix_origin);
    RUN_TEST(test_rtc_epoch_y2k);
    RUN_TEST(test_rtc_epoch_leap_day);
    RUN_TEST(test_rtc_from_epoch_roundtrip);
    RUN_TEST(test_rtc_drift_ppm);
    RUN_TEST(test_rtc_from_epoch_zero);
    RUN_TEST(test_rtc_schedule_alarm);
    RUN_TEST(test_rtc_schedule_alarm_fail);
    RUN_TEST(test_r5_rtc_drift_epoch_anchor);
}
