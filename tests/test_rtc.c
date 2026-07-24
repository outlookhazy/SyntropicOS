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

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_rtc.h"

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
    SYN_RTC_DateTime set_dt = { 2025u, 6u, 15u, 10u, 30u, 45u };
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_set(&set_dt));

    SYN_RTC_DateTime got_dt;
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_get(&got_dt));
    TEST_ASSERT_EQUAL_UINT16(2025u, got_dt.year);
    TEST_ASSERT_EQUAL_UINT8(6u,     got_dt.month);
    TEST_ASSERT_EQUAL_UINT8(15u,    got_dt.day);
    TEST_ASSERT_EQUAL_UINT8(10u,    got_dt.hour);
    TEST_ASSERT_EQUAL_UINT8(30u,    got_dt.minute);
    TEST_ASSERT_EQUAL_UINT8(45u,    got_dt.second);
}

/* ── test 3: set rejects invalid dates ──────────────────────────────────── */

static void test_rtc_set_invalid_month(void)
{
    SYN_RTC_DateTime bad = { 2025u, 13u, 1u, 0u, 0u, 0u };
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_rtc_set(&bad));
}

static void test_rtc_set_invalid_day(void)
{
    SYN_RTC_DateTime bad = { 2025u, 2u, 29u, 0u, 0u, 0u }; /* 2025 not leap */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_rtc_set(&bad));
}

static void test_rtc_set_invalid_hour(void)
{
    SYN_RTC_DateTime bad = { 2025u, 1u, 1u, 24u, 0u, 0u };
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_rtc_set(&bad));
}

/* ── test 4: is_valid ───────────────────────────────────────────────────── */

static void test_rtc_is_valid_ok(void)
{
    SYN_RTC_DateTime dt = { 2024u, 2u, 29u, 23u, 59u, 59u }; /* 2024 = leap */
    TEST_ASSERT_TRUE(syn_rtc_is_valid(&dt));
}

static void test_rtc_is_valid_reject_zero_month(void)
{
    SYN_RTC_DateTime dt = { 2024u, 0u, 1u, 0u, 0u, 0u };
    TEST_ASSERT_FALSE(syn_rtc_is_valid(&dt));
}

/* ── test 5: epoch conversion — known values ────────────────────────────── */

static void test_rtc_epoch_unix_origin(void)
{
    SYN_RTC_DateTime dt = { 1970u, 1u, 1u, 0u, 0u, 0u };
    TEST_ASSERT_EQUAL_UINT32(0u, syn_rtc_to_epoch(&dt));
}

static void test_rtc_epoch_y2k(void)
{
    /* 2000-01-01 00:00:00 UTC = 946684800 */
    SYN_RTC_DateTime dt = { 2000u, 1u, 1u, 0u, 0u, 0u };
    TEST_ASSERT_EQUAL_UINT32(946684800UL, syn_rtc_to_epoch(&dt));
}

static void test_rtc_epoch_leap_day(void)
{
    /* 2000-02-29 is a valid date (2000 is divisible by 400 -> leap) */
    SYN_RTC_DateTime dt = { 2000u, 2u, 29u, 0u, 0u, 0u };
    /* Days from epoch: 10957 (to 2000) + 31 (Jan) + 28 (Feb 1-28) = 11016
     * -> 11016 * 86400 = 951782400 */
    TEST_ASSERT_EQUAL_UINT32(951782400UL, syn_rtc_to_epoch(&dt));
}

/* ── test 6: from_epoch roundtrip ───────────────────────────────────────── */

static void test_rtc_from_epoch_roundtrip(void)
{
    SYN_RTC_DateTime orig = { 2025u, 6u, 27u, 23u, 15u, 30u };
    uint32_t epoch = syn_rtc_to_epoch(&orig);

    SYN_RTC_DateTime back;
    syn_rtc_from_epoch(epoch, &back);

    TEST_ASSERT_EQUAL_UINT16(orig.year,   back.year);
    TEST_ASSERT_EQUAL_UINT8( orig.month,  back.month);
    TEST_ASSERT_EQUAL_UINT8( orig.day,    back.day);
    TEST_ASSERT_EQUAL_UINT8( orig.hour,   back.hour);
    TEST_ASSERT_EQUAL_UINT8( orig.minute, back.minute);
    TEST_ASSERT_EQUAL_UINT8( orig.second, back.second);
}

static void test_rtc_from_epoch_zero(void)
{
    SYN_RTC_DateTime dt;
    syn_rtc_from_epoch(0u, &dt);
    TEST_ASSERT_EQUAL_UINT16(1970u, dt.year);
    TEST_ASSERT_EQUAL_UINT8(1u, dt.month);
    TEST_ASSERT_EQUAL_UINT8(1u, dt.day);
    TEST_ASSERT_EQUAL_UINT8(0u, dt.hour);
    TEST_ASSERT_EQUAL_UINT8(0u, dt.minute);
    TEST_ASSERT_EQUAL_UINT8(0u, dt.second);
}

/* ── Registration ────────────────────────────────────────────────────────── */

static void test_rtc_drift_ppm(void)
{
    syn_rtc_init();
    syn_rtc_set_drift_ppm(10); /* +10 ppm drift compensation */

    SYN_RTC_DateTime set_dt = { 2025u, 6u, 15u, 10u, 30u, 45u };
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_set(&set_dt));

    SYN_RTC_DateTime got_dt;
    TEST_ASSERT_EQUAL(SYN_OK, syn_rtc_get(&got_dt));

    SYN_RTC_DateTime bad_year = { 1969u, 1u, 1u, 0u, 0u, 0u };
    TEST_ASSERT_FALSE(syn_rtc_is_valid(&bad_year));
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
}
