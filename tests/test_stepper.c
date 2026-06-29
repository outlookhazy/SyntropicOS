/**
 * @file test_stepper.c
 * @brief Unity tests for syn_stepper.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/motor/syn_stepper.h"

static void test_stepper(void)
{

    mock_tick_ms = 0;

    SYN_Stepper stepper;
    syn_stepper_init(&stepper, 5, 6);
    syn_stepper_set_speed(&stepper, 100, 1000);

    TEST_ASSERT_FALSE(syn_stepper_is_moving(&stepper));
    TEST_ASSERT_EQUAL_INT(0, syn_stepper_position(&stepper));

    /* Move 10 steps forward */
    syn_stepper_move(&stepper, 10);
    TEST_ASSERT_TRUE(syn_stepper_is_moving(&stepper));

    /* Tick until complete */
    int ticks = 0;
    while (syn_stepper_is_moving(&stepper) && ticks < 10000) {
        mock_tick_advance(1);
        syn_stepper_tick(&stepper);
        ticks++;
    }
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&stepper));
    TEST_ASSERT_EQUAL_INT(10, syn_stepper_position(&stepper));

    /* Move to absolute position */
    syn_stepper_move_to(&stepper, 0);
    while (syn_stepper_is_moving(&stepper) && ticks < 20000) {
        mock_tick_advance(1);
        syn_stepper_tick(&stepper);
        ticks++;
    }
    TEST_ASSERT_EQUAL_INT(0, syn_stepper_position(&stepper));

    /* Emergency stop */
    syn_stepper_move(&stepper, 1000);
    mock_tick_advance(5);
    syn_stepper_tick(&stepper);
    syn_stepper_stop(&stepper);
    TEST_ASSERT_FALSE(syn_stepper_is_moving(&stepper));

    /* Set position */
    syn_stepper_set_position(&stepper, 500);
    TEST_ASSERT_EQUAL_INT(500, syn_stepper_position(&stepper));
}

void run_stepper_tests(void)
{
    RUN_TEST(test_stepper);
}
