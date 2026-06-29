/**
 * @file test_led.c
 * @brief Unity tests for syn_led.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/output/syn_led.h"

static void test_led(void)
{

    mock_tick_ms = 0;
    mock_gpio_states[1] = 0;

    SYN_LED led;
    syn_led_init(&led, 1, SYN_LED_ACTIVE_HIGH);
    TEST_ASSERT_FALSE(syn_led_is_on(&led));

    syn_led_on(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[1]);

    syn_led_off(&led);
    TEST_ASSERT_FALSE(syn_led_is_on(&led));

    syn_led_toggle(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));
    syn_led_toggle(&led);
    TEST_ASSERT_FALSE(syn_led_is_on(&led));

    /* Blink */
    syn_led_blink(&led, 100, 100);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));

    mock_tick_advance(100);
    syn_led_update(&led);
    TEST_ASSERT_FALSE(syn_led_is_on(&led));

    mock_tick_advance(100);
    syn_led_update(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));

    /* Flash N times */
    syn_led_flash(&led, 50, 50, 2);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));

    mock_tick_advance(50);
    syn_led_update(&led);  /* on->off, remain=1 */
    TEST_ASSERT_FALSE(syn_led_is_on(&led));

    mock_tick_advance(50);
    syn_led_update(&led);  /* off->on for flash 2 */
    TEST_ASSERT_TRUE(syn_led_is_on(&led));

    mock_tick_advance(50);
    syn_led_update(&led);  /* on->off, remain=0, mode=OFF */
    TEST_ASSERT_FALSE(syn_led_is_on(&led));
}

void run_led_tests(void)
{
    RUN_TEST(test_led);
}
