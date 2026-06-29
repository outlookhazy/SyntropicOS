/**
 * @file test_hwwdt.c
 * @brief Unity tests for syn_hwwdt (Hardware Watchdog Timer driver).
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/system/syn_hwwdt.h"

static void test_hwwdt_init_ok(void)
{
    mock_wdt_init_ok = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_hwwdt_init(2000u));
    TEST_ASSERT_EQUAL_UINT32(2000u, mock_wdt_timeout_ms);
}

static void test_hwwdt_init_fail(void)
{
    mock_wdt_init_ok = false;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_hwwdt_init(1000u));
}

static void test_hwwdt_feed_increments_count(void)
{
    mock_wdt_init_ok = true;
    syn_hwwdt_init(1000u);
    TEST_ASSERT_EQUAL_UINT32(0u, mock_wdt_feed_count);
    syn_hwwdt_feed();
    TEST_ASSERT_EQUAL_UINT32(1u, mock_wdt_feed_count);
    syn_hwwdt_feed();
    syn_hwwdt_feed();
    TEST_ASSERT_EQUAL_UINT32(3u, mock_wdt_feed_count);
}

static void test_hwwdt_timeout_stored(void)
{
    mock_wdt_init_ok = true;
    syn_hwwdt_init(500u);
    TEST_ASSERT_EQUAL_UINT32(500u, mock_wdt_timeout_ms);
    syn_hwwdt_init(8000u);
    TEST_ASSERT_EQUAL_UINT32(8000u, mock_wdt_timeout_ms);
}

void run_hwwdt_tests(void)
{
    RUN_TEST(test_hwwdt_init_ok);
    RUN_TEST(test_hwwdt_init_fail);
    RUN_TEST(test_hwwdt_feed_increments_count);
    RUN_TEST(test_hwwdt_timeout_stored);
}
