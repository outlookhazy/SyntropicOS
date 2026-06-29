/**
 * @file test_trace.c
 * @brief Unity tests for syn_trace.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/debug/syn_trace.h"

static void test_trace(void)
{

    mock_tick_ms = 0;

    SYN_TraceEntry entries[4];
    SYN_Trace trace;
    syn_trace_init(&trace, entries, 4);

    TEST_ASSERT_EQUAL_INT(0, syn_trace_count(&trace));

    /* Record some events */
    mock_tick_ms = 100;
    syn_trace_record(&trace, 0x01, 42);
    mock_tick_ms = 200;
    syn_trace_record(&trace, 0x02, 99);
    TEST_ASSERT_EQUAL_INT(2, syn_trace_count(&trace));

    /* Read back */
    SYN_TraceEntry e;
    TEST_ASSERT_TRUE(syn_trace_read(&trace, 0, &e));
    TEST_ASSERT_EQUAL_HEX8(0x01, e.event_id);
    TEST_ASSERT_EQUAL_INT(42, e.value);
    TEST_ASSERT_EQUAL_INT(100, e.timestamp);

    TEST_ASSERT_TRUE(syn_trace_read(&trace, 1, &e));
    TEST_ASSERT_EQUAL_HEX8(0x02, e.event_id);

    /* Overflow wraps (capacity=4, add 4 more → wraps) */
    mock_tick_ms = 300;
    syn_trace_record(&trace, 0x03, 0);
    syn_trace_record(&trace, 0x04, 0);
    syn_trace_record(&trace, 0x05, 0);  /* wraps, overwrites slot 0 */
    syn_trace_record(&trace, 0x06, 0);

    TEST_ASSERT_EQUAL_INT(6, syn_trace_count(&trace));
    /* Oldest available should now be event 0x03 */
    TEST_ASSERT_TRUE(syn_trace_read(&trace, 0, &e));
    TEST_ASSERT_EQUAL_HEX8(0x03, e.event_id);

    /* Disable */
    syn_trace_enable(&trace, false);
    syn_trace_record(&trace, 0xFF, 0);
    TEST_ASSERT_EQUAL_INT(6, syn_trace_count(&trace));

    /* Clear */
    syn_trace_enable(&trace, true);
    syn_trace_clear(&trace);
    TEST_ASSERT_EQUAL_INT(0, syn_trace_count(&trace));
}

void run_trace_tests(void)
{
    RUN_TEST(test_trace);
}
