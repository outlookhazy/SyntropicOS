/**
 * @file test_hysteresis.c
 * @brief Unity tests for syn_hysteresis.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_hysteresis.h"

static void test_hysteresis(void)
{

    SYN_Hysteresis h;
    syn_hyst_init(&h, 1000, 50, false);  /* threshold=1000, band=±50 */

    TEST_ASSERT_FALSE(syn_hyst_state(&h));

    /* Value below threshold — no change */
    syn_hyst_update(&h, 900);
    TEST_ASSERT_FALSE(syn_hyst_state(&h));

    /* Value in deadband — no change */
    syn_hyst_update(&h, 1030);
    TEST_ASSERT_FALSE(syn_hyst_state(&h));

    /* Value crosses high trip (1050) */
    syn_hyst_update(&h, 1051);
    TEST_ASSERT_TRUE(syn_hyst_state(&h));

    /* Value drops but stays above low trip (950) */
    syn_hyst_update(&h, 980);
    TEST_ASSERT_TRUE(syn_hyst_state(&h));

    /* Value drops below low trip */
    syn_hyst_update(&h, 940);
    TEST_ASSERT_FALSE(syn_hyst_state(&h));
}

void run_hysteresis_tests(void)
{
    RUN_TEST(test_hysteresis);
}
