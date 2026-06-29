/**
 * @file test_ramp.c
 * @brief Unity tests for syn_ramp.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"

#include "syntropic/util/syn_ramp.h"

static void test_ramp(void)
{
    SYN_Ramp r;
    syn_ramp_init(&r, 0);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 0);
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
    syn_ramp_set_target(&r, 100, 10);
    TEST_ASSERT_TRUE(!syn_ramp_done(&r));
    int32_t val = 0;
    for (int i = 0; i < 10; i++) val = syn_ramp_update(&r);
    TEST_ASSERT_EQUAL_INT(100, val);
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
    syn_ramp_set_target(&r, 50, 25);
    syn_ramp_update(&r); syn_ramp_update(&r);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 50);
    syn_ramp_jump(&r, 999);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 999);
    syn_ramp_set_target(&r, 1002, 10);
    syn_ramp_update(&r);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 1002);
    syn_ramp_init(&r, 0);
    syn_ramp_set_target_scurve(&r, 1000, 100, 10);
    int iters = 0;
    while (!syn_ramp_done(&r) && iters < 500) { syn_ramp_update(&r); iters++; }
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 1000);
    syn_ramp_init(&r, 100);
    syn_ramp_set_target(&r, -100, 50);
    for (int i = 0; i < 4; i++) syn_ramp_update(&r);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == -100);
}

void run_ramp_tests(void)
{
    RUN_TEST(test_ramp);
}
