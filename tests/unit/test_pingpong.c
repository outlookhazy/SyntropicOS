/**
 * @file test_pingpong.c
 * @brief Unity tests for syn_pingpong.
 */

#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_pingpong.h"
#include "unity/unity.h"

static void test_pingpong(void)
{
    uint8_t a[8], b[8];
    SYN_PingPong pp;
    syn_pingpong_init(&pp, a, b, 8);

    TEST_ASSERT_EQUAL(a, syn_pingpong_active(&pp));
    TEST_ASSERT_FALSE(syn_pingpong_ready(&pp));
    TEST_ASSERT_EQUAL_INT(8, syn_pingpong_size(&pp));
    TEST_ASSERT_EQUAL_UINT32(0U, syn_pingpong_overruns(&pp));

    /* Write to active, swap */
    memset(syn_pingpong_active(&pp), 0xAA, 8);
    TEST_ASSERT_TRUE(syn_pingpong_swap(&pp));

    TEST_ASSERT_TRUE(syn_pingpong_ready(&pp));
    TEST_ASSERT_EQUAL(b, syn_pingpong_active(&pp));
    TEST_ASSERT_EQUAL(a, syn_pingpong_ready_buf(&pp));
    TEST_ASSERT_EQUAL_HEX8(0xAA, syn_pingpong_ready_buf(&pp)[0]);

    /* D-CORE-03: Swap while unconsumed must NOT flip active onto consumer's ready buffer a */
    memset(syn_pingpong_active(&pp), 0xCC, 8);
    TEST_ASSERT_FALSE(syn_pingpong_swap(&pp));
    TEST_ASSERT_EQUAL_UINT32(1U, syn_pingpong_overruns(&pp));
    TEST_ASSERT_EQUAL(b, syn_pingpong_active(&pp));
    TEST_ASSERT_EQUAL(a, syn_pingpong_ready_buf(&pp));
    TEST_ASSERT_EQUAL_HEX8(0xAA, syn_pingpong_ready_buf(&pp)[0]);

    /* Consume */
    syn_pingpong_consume(&pp);
    TEST_ASSERT_FALSE(syn_pingpong_ready(&pp));

    /* Write to b, swap back */
    memset(syn_pingpong_active(&pp), 0xBB, 8);
    TEST_ASSERT_TRUE(syn_pingpong_swap(&pp));
    TEST_ASSERT_EQUAL_UINT32(1U, syn_pingpong_overruns(&pp));
    TEST_ASSERT_EQUAL(a, syn_pingpong_active(&pp));
    TEST_ASSERT_EQUAL_HEX8(0xBB, syn_pingpong_ready_buf(&pp)[0]);
}

void run_pingpong_tests(void)
{
    RUN_TEST(test_pingpong);
}
