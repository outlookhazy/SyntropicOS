/**
 * @file test_boot.c
 * @brief Unity tests for syn_boot — full coverage (adds set_reset_reason,
 *        set_errlog, log_events paths).
 */

#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/system/syn_boot.h"
#include "syntropic/system/syn_errlog.h"
#include "unity/unity.h"

static void test_boot(void)
{
    /* Fresh flash */
    memset(mock_flash, 0xFF, sizeof(mock_flash));

    SYN_ParamStore boot_store;
    SYN_Status st = syn_param_init(&boot_store, 0, 2, sizeof(SYN_BootData));
    (void)st;
    /* No data yet */

    SYN_Boot boot;
    st = syn_boot_init(&boot, &boot_store, 3);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_EQUAL_INT(1, syn_boot_count(&boot));
    TEST_ASSERT_FALSE(syn_boot_is_safe_mode(&boot));

    /* Mark healthy */
    st = syn_boot_mark_healthy(&boot);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Simulate reboot (re-init) */
    SYN_ParamStore boot_store2;
    syn_param_init(&boot_store2, 0, 2, sizeof(SYN_BootData));

    SYN_Boot boot2;
    st = syn_boot_init(&boot2, &boot_store2, 3);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(2, syn_boot_count(&boot2));
    TEST_ASSERT_EQUAL_INT(0, syn_boot_fail_count(&boot2));

    /* Simulate crash loops (don't mark healthy) */
    int i;
    for (i = 0; i < 3; i++) {
        SYN_ParamStore bs;
        syn_param_init(&bs, 0, 2, sizeof(SYN_BootData));
        SYN_Boot b;
        syn_boot_init(&b, &bs, 3);
        /* don't mark healthy */
    }

    /* Next boot should be safe mode */
    SYN_ParamStore bs;
    syn_param_init(&bs, 0, 2, sizeof(SYN_BootData));
    SYN_Boot bsafe;
    syn_boot_init(&bsafe, &bs, 3);
    TEST_ASSERT_TRUE(syn_boot_is_safe_mode(&bsafe));

    /* Clear safe mode */
    st = syn_boot_clear_safe_mode(&bsafe);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_FALSE(syn_boot_is_safe_mode(&bsafe));
}

/** syn_boot_set_reset_reason — exercises last_reset field write */
static void test_boot_set_reset_reason(void)
{
    memset(mock_flash, 0xFF, sizeof(mock_flash));

    SYN_ParamStore store;
    syn_param_init(&store, 0, 2, sizeof(SYN_BootData));

    SYN_Boot boot;
    syn_boot_init(&boot, &store, 3);

    syn_boot_set_reset_reason(&boot, 0x5A);
    TEST_ASSERT_EQUAL_HEX8(0x5A, boot.data.last_reset);
}

/** syn_boot_set_errlog + syn_boot_log_events (no errlog) */
static void test_boot_log_events_no_errlog(void)
{
    memset(mock_flash, 0xFF, sizeof(mock_flash));

    SYN_ParamStore store;
    syn_param_init(&store, 0, 2, sizeof(SYN_BootData));

    SYN_Boot boot;
    syn_boot_init(&boot, &store, 3);

    /* log_events with NULL errlog — should be a no-op */
    syn_boot_log_events(&boot);
    /* No crash = pass */
    TEST_ASSERT_NULL(boot.errlog);
}

/** syn_boot_set_errlog + syn_boot_log_events with crash-loop and safe mode */
static void test_boot_log_events_with_errlog(void)
{
    /* Simulate enough failures to trigger safe mode */
    memset(mock_flash, 0xFF, sizeof(mock_flash));
    int i;
    for (i = 0; i < 4; i++) {
        SYN_ParamStore bs;
        syn_param_init(&bs, 0, 2, sizeof(SYN_BootData));
        SYN_Boot b;
        syn_boot_init(&b, &bs, 3);
        /* don't mark healthy */
    }

    SYN_ParamStore store;
    syn_param_init(&store, 0, 2, sizeof(SYN_BootData));
    SYN_Boot boot;
    syn_boot_init(&boot, &store, 3);
    /* At this point we should be in safe mode, and fail_count > 0 */

    /* Set an errlog */
    static SYN_ErrEntry errlog_buf[4];
    static SYN_ErrLog errlog;
    syn_errlog_init(&errlog, errlog_buf, 4, 1);

    syn_boot_set_errlog(&boot, &errlog);
    TEST_ASSERT_EQUAL_PTR(&errlog, boot.errlog);

    /* log_events should record the crash loop (fail_count > 0) */
    /* and possibly safe mode entry */
    syn_boot_log_events(&boot);

    /* At least one errlog entry should have been made */
    TEST_ASSERT_TRUE(syn_errlog_count(&errlog) > 0);
}

static void test_boot_null_checks(void)
{
    SYN_Boot uninit_boot;
    memset(&uninit_boot, 0, sizeof(uninit_boot));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_boot_init(NULL, NULL, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_boot_mark_healthy(NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_boot_mark_healthy(&uninit_boot));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_boot_clear_safe_mode(NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_boot_clear_safe_mode(&uninit_boot));
    syn_boot_set_reset_reason(NULL, SYN_RESET_POWER_ON);
    syn_boot_set_errlog(NULL, NULL);
    syn_boot_log_events(NULL);
    TEST_ASSERT_FALSE(syn_boot_is_safe_mode(NULL));
}

/**
 * @brief Verify fail_count saturates at UINT16_MAX instead of wrapping to 0.
 */
static void test_tier_a_boot_fail_count_saturation(void)
{
    memset(mock_flash, 0xFF, sizeof(mock_flash));

    SYN_ParamStore store;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_param_init(&store, 0U, 2U, sizeof(SYN_BootData)));

    SYN_BootData seeded = {
        .boot_count = 500U,
        .fail_count = UINT16_MAX,
        .was_healthy = 0U,
        .last_reset = SYN_RESET_WATCHDOG,
    };
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_save(&store, &seeded));

    SYN_ParamStore reboot_store;
    TEST_ASSERT_EQUAL(SYN_OK, syn_param_init(&reboot_store, 0U, 2U, sizeof(SYN_BootData)));

    SYN_Boot boot;
    TEST_ASSERT_EQUAL(SYN_OK, syn_boot_init(&boot, &reboot_store, 3U));
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, syn_boot_fail_count(&boot));
    TEST_ASSERT_TRUE(syn_boot_is_safe_mode(&boot));
}

void run_boot_tests(void)
{
    RUN_TEST(test_boot);
    RUN_TEST(test_boot_set_reset_reason);
    RUN_TEST(test_boot_log_events_no_errlog);
    RUN_TEST(test_boot_log_events_with_errlog);
    RUN_TEST(test_boot_null_checks);
    RUN_TEST(test_tier_a_boot_fail_count_saturation);
}
