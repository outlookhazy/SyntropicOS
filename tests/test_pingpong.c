/**
 * @file test_pingpong.c
 * @brief Unity tests for syn_pingpong.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_pingpong.h"

static void test_pingpong(void)
{

    uint8_t a[8], b[8];
    SYN_PingPong pp;
    syn_pingpong_init(&pp, a, b, 8);

    TEST_ASSERT_EQUAL(a, syn_pingpong_active(&pp));
    TEST_ASSERT_FALSE(syn_pingpong_ready(&pp));
    TEST_ASSERT_EQUAL_INT(8, syn_pingpong_size(&pp));

    /* Write to active, swap */
    memset(syn_pingpong_active(&pp), 0xAA, 8);
    syn_pingpong_swap(&pp);

    TEST_ASSERT_TRUE(syn_pingpong_ready(&pp));
    TEST_ASSERT_EQUAL(b, syn_pingpong_active(&pp));
    TEST_ASSERT_EQUAL(a, syn_pingpong_ready_buf(&pp));
    TEST_ASSERT_EQUAL_HEX8(0xAA, syn_pingpong_ready_buf(&pp)[0]);

    /* Consume */
    syn_pingpong_consume(&pp);
    TEST_ASSERT_FALSE(syn_pingpong_ready(&pp));

    /* Write to b, swap back */
    memset(syn_pingpong_active(&pp), 0xBB, 8);
    syn_pingpong_swap(&pp);
    TEST_ASSERT_EQUAL(a, syn_pingpong_active(&pp));
    TEST_ASSERT_EQUAL_HEX8(0xBB, syn_pingpong_ready_buf(&pp)[0]);
}

void run_pingpong_tests(void)
{
    RUN_TEST(test_pingpong);
}
