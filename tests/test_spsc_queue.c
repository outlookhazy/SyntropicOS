/**
 * @file test_spsc_queue.c
 * @brief Unit test suite for Lock-Free SPSC Queue (syn_spsc_queue).
 */

#include "unity/unity.h"
#include "syntropic/util/syn_spsc_queue.h"

typedef struct {
    uint32_t id;
    uint16_t val;
} TestMsg;

void test_spsc_queue_push_pop_count(void)
{
    TestMsg buffer[8];
    SYN_SPSC_Queue q;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_init(&q, buffer, sizeof(TestMsg), 8));
    TEST_ASSERT_TRUE(syn_spsc_queue_is_empty(&q));
    TEST_ASSERT_FALSE(syn_spsc_queue_is_full(&q));
    TEST_ASSERT_EQUAL_UINT32(0, syn_spsc_queue_count(&q));

    TestMsg msg1 = {.id = 100, .val = 0xAA};
    TestMsg msg2 = {.id = 101, .val = 0xBB};

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_push(&q, &msg1));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_push(&q, &msg2));
    TEST_ASSERT_FALSE(syn_spsc_queue_is_empty(&q));
    TEST_ASSERT_EQUAL_UINT32(2, syn_spsc_queue_count(&q));

    TestMsg out;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL_UINT32(100, out.id);
    TEST_ASSERT_EQUAL_UINT16(0xAA, out.val);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL_UINT32(101, out.id);
    TEST_ASSERT_EQUAL_UINT16(0xBB, out.val);

    TEST_ASSERT_TRUE(syn_spsc_queue_is_empty(&q));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_spsc_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spsc_queue_init(NULL, buffer, sizeof(TestMsg), 8));
}

void run_spsc_queue_tests(void)
{
    RUN_TEST(test_spsc_queue_push_pop_count);
}
