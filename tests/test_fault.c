/**
 * @file test_fault.c
 * @brief Unity tests for Post-Mortem Hard Fault diagnostics.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"

static void test_fault_check_and_log(void)
{
    SYN_ErrEntry err_buf[4];
    SYN_ErrLog elog;
    syn_errlog_init(&elog, err_buf, 4, 1);

    /* Simulate a Hard Fault capture */
    SYN_FaultContext ctx = {
        .pc = 0x08001234,
        .lr = 0x08005678,
        .sp = 0x20001000,
        .r0 = 0,
        .r1 = 1,
        .r2 = 2,
        .r3 = 3,
        .r12 = 12,
        .xpsr = 0x01000000
    };

    syn_fault_capture(&ctx);

    /* Check and log the fault */
    bool detected = syn_fault_check_and_log(&elog);
    TEST_ASSERT_TRUE(detected);

    /* Verify it is correctly logged in the error log */
    TEST_ASSERT_EQUAL_INT(1, syn_errlog_count(&elog));
    SYN_ErrEntry latest;
    bool read_ok = syn_errlog_read(&elog, 0, &latest);
    TEST_ASSERT_TRUE(read_ok);
    TEST_ASSERT_EQUAL_UINT16(0xFA17, latest.code);
    TEST_ASSERT_EQUAL_UINT8(SYN_ERR_FATAL, latest.severity);
    TEST_ASSERT_EQUAL_UINT32(0x08001234, latest.context); /* PC recorded as context */

    /* Second check must find nothing (signature cleared) */
    detected = syn_fault_check_and_log(&elog);
    TEST_ASSERT_FALSE(detected);
    TEST_ASSERT_EQUAL_INT(1, syn_errlog_count(&elog));
}

static void test_fault_null_log(void)
{
    SYN_FaultContext ctx = { .pc = 0x08005555 };
    syn_fault_capture(&ctx);

    /* Check with NULL errlog should handle safely (not crash) and clear signature */
    bool detected = syn_fault_check_and_log(NULL);
    TEST_ASSERT_TRUE(detected);

    detected = syn_fault_check_and_log(NULL);
    TEST_ASSERT_FALSE(detected);
}

static void test_fault_capture_null(void)
{
    /* Capture NULL should bypass copy safely and not modify dump */
    syn_fault_capture(NULL);
}

void run_fault_tests(void)
{
    RUN_TEST(test_fault_check_and_log);
    RUN_TEST(test_fault_null_log);
    RUN_TEST(test_fault_capture_null);
}

