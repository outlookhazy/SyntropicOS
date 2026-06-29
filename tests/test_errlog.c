/**
 * @file test_errlog.c
 * @brief Unity tests for syn_errlog.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/system/syn_errlog.h"

static void test_errlog(void)
{

    mock_tick_ms = 0;

    SYN_ErrEntry entries[4];
    SYN_ErrLog elog;
    syn_errlog_init(&elog, entries, 4, 5); /* boot count 5 */

    TEST_ASSERT_EQUAL_INT(0, syn_errlog_count(&elog));
    TEST_ASSERT_EQUAL(NULL, syn_errlog_latest(&elog));

    /* Record some errors */
    mock_tick_ms = 100;
    syn_errlog_record(&elog, 0x0001, SYN_ERR_WARNING, 42);
    mock_tick_ms = 200;
    syn_errlog_record(&elog, 0x0002, SYN_ERR_ERROR, 99);

    TEST_ASSERT_EQUAL_INT(2, syn_errlog_count(&elog));
    TEST_ASSERT_EQUAL_INT(2, syn_errlog_available(&elog));

    /* Read back */
    SYN_ErrEntry e;
    TEST_ASSERT_TRUE(syn_errlog_read(&elog, 0, &e));
    TEST_ASSERT_EQUAL_HEX16(0x0001, e.code);
    TEST_ASSERT_EQUAL(SYN_ERR_WARNING, e.severity);
    TEST_ASSERT_EQUAL_INT(42, e.context);
    TEST_ASSERT_EQUAL_INT(100, e.timestamp);
    TEST_ASSERT_EQUAL_INT(5, e.boot_count);

    /* Latest */
    const SYN_ErrEntry *latest = syn_errlog_latest(&elog);
    TEST_ASSERT_TRUE(latest != NULL);
    TEST_ASSERT_EQUAL_HEX16(0x0002, latest->code);

    /* Severity count */
    TEST_ASSERT_EQUAL_INT(1, syn_errlog_count_severity(&elog, SYN_ERR_WARNING));
    TEST_ASSERT_EQUAL_INT(1, syn_errlog_count_severity(&elog, SYN_ERR_ERROR));
    TEST_ASSERT_EQUAL_INT(0, syn_errlog_count_severity(&elog, SYN_ERR_FATAL));

    /* Overflow wraps (capacity=4) */
    syn_errlog_record(&elog, 0x0003, SYN_ERR_INFO, 0);
    syn_errlog_record(&elog, 0x0004, SYN_ERR_INFO, 0);
    syn_errlog_record(&elog, 0x0005, SYN_ERR_FATAL, 0); /* wraps */

    TEST_ASSERT_EQUAL_INT(5, syn_errlog_count(&elog));
    TEST_ASSERT_EQUAL_INT(4, syn_errlog_available(&elog));

    /* Oldest should now be 0x0002 */
    TEST_ASSERT_TRUE(syn_errlog_read(&elog, 0, &e));
    TEST_ASSERT_EQUAL_HEX16(0x0002, e.code);

    /* Disable */
    syn_errlog_enable(&elog, false);
    syn_errlog_record(&elog, 0xFFFF, SYN_ERR_FATAL, 0);
    TEST_ASSERT_EQUAL_INT(5, syn_errlog_count(&elog));

    /* Clear */
    syn_errlog_enable(&elog, true);
    syn_errlog_clear(&elog);
    TEST_ASSERT_EQUAL_INT(0, syn_errlog_count(&elog));
}

void run_errlog_tests(void)
{
    RUN_TEST(test_errlog);
}
