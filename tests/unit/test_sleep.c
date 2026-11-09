/**
 * @file test_sleep.c
 * @brief Unity tests for syn_sleep.
 */

#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/system/syn_sleep.h"
#include "unity/unity.h"

static void test_sleep(void)
{
    mock_sleep_count = 0;

    SYN_Sleep sl;
    syn_sleep_init(&sl, SYN_SLEEP_LIGHT);

    TEST_ASSERT_FALSE(syn_sleep_any_locked(&sl));

    /* Sleep should work with no locks */
    TEST_ASSERT_TRUE(syn_sleep_enter(&sl));
    TEST_ASSERT_EQUAL_INT(1, mock_sleep_count);

    /* Acquire lock → sleep should be vetoed */
    syn_sleep_lock(&sl, SYN_SLEEP_LOCK_UART);
    TEST_ASSERT_TRUE(syn_sleep_any_locked(&sl));
    TEST_ASSERT_TRUE(syn_sleep_is_locked(&sl, SYN_SLEEP_LOCK_UART));

    TEST_ASSERT_FALSE(syn_sleep_enter(&sl));
    TEST_ASSERT_EQUAL_INT(1, mock_sleep_count);

    /* Release lock → sleep works again */
    syn_sleep_unlock(&sl, SYN_SLEEP_LOCK_UART);
    TEST_ASSERT_FALSE(syn_sleep_any_locked(&sl));
    TEST_ASSERT_TRUE(syn_sleep_enter(&sl));
    TEST_ASSERT_EQUAL_INT(2, mock_sleep_count);

    /* Multiple locks */
    syn_sleep_lock(&sl, SYN_SLEEP_LOCK_UART);
    syn_sleep_lock(&sl, SYN_SLEEP_LOCK_SPI);
    TEST_ASSERT_TRUE(syn_sleep_is_locked(&sl, SYN_SLEEP_LOCK_UART));
    TEST_ASSERT_TRUE(syn_sleep_any_locked(&sl));
    TEST_ASSERT_FALSE(syn_sleep_enter(&sl));

    syn_sleep_unlock(&sl, SYN_SLEEP_LOCK_UART);
    TEST_ASSERT_FALSE(syn_sleep_is_locked(&sl, SYN_SLEEP_LOCK_UART));
    TEST_ASSERT_TRUE(syn_sleep_any_locked(&sl));
    TEST_ASSERT_FALSE(syn_sleep_enter(&sl));

    syn_sleep_unlock(&sl, SYN_SLEEP_LOCK_SPI);
    TEST_ASSERT_TRUE(syn_sleep_enter(&sl));

    /* Disable */
    syn_sleep_enable(&sl, false);
    TEST_ASSERT_FALSE(syn_sleep_enter(&sl));

    /* Stats */
    TEST_ASSERT_EQUAL_INT(3, sl.enter_count);
    TEST_ASSERT_TRUE(sl.veto_count > 0);
    TEST_ASSERT_EQUAL_INT(0, syn_sleep_locks(&sl));

    /* Test SYN_SLEEP_NONE and SYN_SLEEP_DEEP modes */
    int count_before_none = mock_sleep_count;
    syn_sleep_init(&sl, SYN_SLEEP_NONE);
    TEST_ASSERT_FALSE(syn_sleep_enter(&sl));
    TEST_ASSERT_EQUAL_INT(count_before_none, mock_sleep_count);

    syn_sleep_init(&sl, SYN_SLEEP_DEEP);
    TEST_ASSERT_TRUE(syn_sleep_enter(&sl));
    TEST_ASSERT_EQUAL_INT(count_before_none + 1, mock_sleep_count);
}

/**
 * Regression: syn_sleep_lock/unlock are documented as ISR-callable, so
 * the lock_mask read-modify-write must run inside a critical section —
 * a raw |= / &= could lose a preempting ISR's lock update and let the
 * CPU sleep through an active transfer.
 */
static void test_sleep_lock_unlock_atomic(void)
{
    SYN_Sleep sl;
    syn_sleep_init(&sl, SYN_SLEEP_LIGHT);

    int enters_before = mock_critical_enter_count;

    syn_sleep_lock(&sl, SYN_SLEEP_LOCK_DMA);
    TEST_ASSERT_EQUAL_INT(enters_before + 1, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth); /* balanced exit */
    TEST_ASSERT_TRUE(syn_sleep_is_locked(&sl, SYN_SLEEP_LOCK_DMA));

    syn_sleep_unlock(&sl, SYN_SLEEP_LOCK_DMA);
    TEST_ASSERT_EQUAL_INT(enters_before + 2, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
    TEST_ASSERT_FALSE(syn_sleep_any_locked(&sl));
}

static void test_r5_sleep_none_veto_and_null(void)
{
    mock_sleep_count = 0;

    TEST_ASSERT_FALSE(syn_sleep_enter(NULL));
    TEST_ASSERT_EQUAL_INT(0, mock_sleep_count);

    SYN_Sleep sl;
    syn_sleep_init(&sl, SYN_SLEEP_NONE);
    TEST_ASSERT_FALSE(syn_sleep_enter(&sl));
    TEST_ASSERT_EQUAL_INT(0, mock_sleep_count);
    TEST_ASSERT_EQUAL_UINT32(0U, sl.enter_count);
    TEST_ASSERT_EQUAL_UINT32(1U, sl.veto_count);
}

static void test_sleep_enter_critical_section_guard(void)
{
    SYN_Sleep sl;
    syn_sleep_init(&sl, SYN_SLEEP_LIGHT);

    int enters_before = mock_critical_enter_count;
    TEST_ASSERT_TRUE(syn_sleep_enter(&sl));
    TEST_ASSERT_EQUAL_INT(enters_before + 1, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    syn_sleep_lock(&sl, SYN_SLEEP_LOCK_UART);
    enters_before = mock_critical_enter_count;
    TEST_ASSERT_FALSE(syn_sleep_enter(&sl));
    TEST_ASSERT_EQUAL_INT(enters_before + 1, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
    syn_sleep_unlock(&sl, SYN_SLEEP_LOCK_UART);
}

void run_sleep_tests(void)
{
    RUN_TEST(test_sleep);
    RUN_TEST(test_sleep_lock_unlock_atomic);
    RUN_TEST(test_r5_sleep_none_veto_and_null);
    RUN_TEST(test_sleep_enter_critical_section_guard);
}
