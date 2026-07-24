/**
 * @file test_event_flags.c
 * @brief Unit test suite for Event Flag Groups (syn_event_flags).
 */

#include "unity/unity.h"
#include "syntropic/sched/syn_event_flags.h"

void test_event_flags_set_clear_wait(void)
{
    SYN_EventFlags ef;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_init(&ef));
    TEST_ASSERT_EQUAL_UINT32(0, syn_event_flags_get(&ef));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_set(&ef, 0x05U)); /* Bits 0 & 2 */
    TEST_ASSERT_EQUAL_UINT32(0x05U, syn_event_flags_get(&ef));

    uint32_t matched = 0;

    /* WAIT_ALL - unsatisfiable (requires 0x07U) */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_event_flags_wait(&ef, 0x07U, SYN_EVENT_FLAGS_WAIT_ALL, &matched));

    /* WAIT_ANY - satisfied (bit 0 or 2 present) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_wait(&ef, 0x07U, SYN_EVENT_FLAGS_WAIT_ANY, &matched));
    TEST_ASSERT_EQUAL_UINT32(0x05U, matched);

    /* WAIT_ALL with AUTO_CLEAR */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_set(&ef, 0x02U)); /* Bit 1 set -> now 0x07U */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_wait(&ef, 0x07U, SYN_EVENT_FLAGS_WAIT_ALL | SYN_EVENT_FLAGS_AUTO_CLEAR, &matched));
    TEST_ASSERT_EQUAL_UINT32(0x07U, matched);
    TEST_ASSERT_EQUAL_UINT32(0, syn_event_flags_get(&ef)); /* Flags auto-cleared */

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_clear(&ef, 0xFFFFFFFFU));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_event_flags_init(NULL));
}

void run_event_flags_tests(void)
{
    RUN_TEST(test_event_flags_set_clear_wait);
}
