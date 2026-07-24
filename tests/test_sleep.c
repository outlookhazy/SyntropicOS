/**
 * @file test_sleep.c
 * @brief Unity tests for syn_sleep.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/system/syn_sleep.h"

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
    TEST_ASSERT_FALSE(syn_sleep_enter(&sl));

    syn_sleep_unlock(&sl, SYN_SLEEP_LOCK_UART);
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
    syn_sleep_init(&sl, SYN_SLEEP_NONE);
    TEST_ASSERT_TRUE(syn_sleep_enter(&sl));

    syn_sleep_init(&sl, SYN_SLEEP_DEEP);
    TEST_ASSERT_TRUE(syn_sleep_enter(&sl));
}

void run_sleep_tests(void)
{
    RUN_TEST(test_sleep);
}
