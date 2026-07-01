/**
 * @file test_led.c
 * @brief Unity tests for syn_led.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/output/syn_led.h"

static void test_led_basic(void)
{
    mock_tick_ms = 0;
    mock_gpio_states[1] = 0;

    SYN_LED led;
    syn_led_init(&led, 1, SYN_LED_ACTIVE_HIGH);
    TEST_ASSERT_FALSE(syn_led_is_on(&led));

    syn_led_on(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[1]);
    syn_led_update(&led); /* covers case SYN_LED_MODE_ON */

    syn_led_off(&led);
    TEST_ASSERT_FALSE(syn_led_is_on(&led));
    syn_led_update(&led); /* covers case SYN_LED_MODE_OFF */

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

static void test_led_active_low(void)
{
    mock_tick_ms = 0;
    mock_gpio_states[2] = 1;

    SYN_LED led;
    syn_led_init(&led, 2, SYN_LED_ACTIVE_LOW);
    TEST_ASSERT_FALSE(syn_led_is_on(&led));
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[2]); // Initial off

    syn_led_on(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[2]); // Active low on

    syn_led_off(&led);
    TEST_ASSERT_FALSE(syn_led_is_on(&led));
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[2]); // Active low off
}

static void test_led_service(void)
{
    mock_tick_ms = 0;
    SYN_LED leds[2];
    syn_led_init(&leds[0], 1, SYN_LED_ACTIVE_HIGH);
    syn_led_init(&leds[1], 2, SYN_LED_ACTIVE_HIGH);

    syn_led_blink(&leds[0], 100, 100);
    syn_led_blink(&leds[1], 50, 50);

    syn_led_service(leds, 2);
    TEST_ASSERT_TRUE(syn_led_is_on(&leds[0]));
    TEST_ASSERT_TRUE(syn_led_is_on(&leds[1]));

    mock_tick_advance(50);
    syn_led_service(leds, 2);
    TEST_ASSERT_TRUE(syn_led_is_on(&leds[0]));
    TEST_ASSERT_FALSE(syn_led_is_on(&leds[1]));

    mock_tick_advance(50);
    syn_led_service(leds, 2);
    TEST_ASSERT_FALSE(syn_led_is_on(&leds[0]));
    TEST_ASSERT_TRUE(syn_led_is_on(&leds[1]));
}

static void test_led_pattern(void)
{
    mock_tick_ms = 0;
    SYN_LED led;
    syn_led_init(&led, 1, SYN_LED_ACTIVE_HIGH);

    /* 1. Empty and NULL pattern handling */
    syn_led_pattern(&led, NULL, 10);
    TEST_ASSERT_FALSE(syn_led_is_on(&led));
    TEST_ASSERT_EQUAL(SYN_LED_MODE_OFF, led.mode);

    syn_led_pattern(&led, "", 10);
    TEST_ASSERT_FALSE(syn_led_is_on(&led));
    TEST_ASSERT_EQUAL(SYN_LED_MODE_OFF, led.mode);

    /* 2. Play pattern ".- |?" with unit = 10ms */
    syn_led_pattern(&led, ".- |?", 10);
    TEST_ASSERT_EQUAL(SYN_LED_MODE_PATTERN, led.mode);
    TEST_ASSERT_FALSE(syn_led_is_on(&led));

    /* '.' - Short on */
    syn_led_update(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));

    mock_tick_advance(5);
    syn_led_update(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));

    mock_tick_advance(5);
    syn_led_update(&led);
    TEST_ASSERT_FALSE(syn_led_is_on(&led)); /* Turned off, index -> 1 */

    /* '-' - Long on */
    syn_led_update(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));

    mock_tick_advance(25);
    syn_led_update(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));

    mock_tick_advance(5);
    syn_led_update(&led);
    TEST_ASSERT_FALSE(syn_led_is_on(&led)); /* Turned off, index -> 2 */

    /* ' ' - Short pause */
    mock_tick_advance(5);
    syn_led_update(&led);
    TEST_ASSERT_EQUAL(2, led.pattern_idx);

    mock_tick_advance(5);
    syn_led_update(&led); /* elapsed >= unit, index -> 3 */
    TEST_ASSERT_EQUAL(3, led.pattern_idx);

    /* '|' - Long pause */
    mock_tick_advance(25);
    syn_led_update(&led);
    TEST_ASSERT_EQUAL(3, led.pattern_idx);

    mock_tick_advance(5);
    syn_led_update(&led); /* elapsed >= 3*unit, index -> 4 */
    TEST_ASSERT_EQUAL(4, led.pattern_idx);

    /* '?' - Unknown char (should skip) */
    syn_led_update(&led); /* skipped, index -> 5 */
    TEST_ASSERT_EQUAL(5, led.pattern_idx);

    /* '\0' - Loop back to start */
    syn_led_update(&led); /* looped, index -> 0 */
    TEST_ASSERT_EQUAL(0, led.pattern_idx);
}

static void test_led_next_ms(void)
{
    mock_tick_ms = 0;
    SYN_LED led;
    syn_led_init(&led, 1, SYN_LED_ACTIVE_HIGH);

    /* 1. OFF mode → UINT32_MAX (static, no transition) */
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, syn_led_next_ms(&led));

    /* 2. ON mode → UINT32_MAX */
    syn_led_on(&led);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, syn_led_next_ms(&led));

    /* 3. BLINK — lit, mid-phase */
    syn_led_blink(&led, 100, 200);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));
    mock_tick_advance(40);
    TEST_ASSERT_EQUAL_UINT32(60, syn_led_next_ms(&led));  /* 100 - 40 */

    /* 4. BLINK — lit, exactly on boundary → overdue */
    mock_tick_advance(60);
    TEST_ASSERT_EQUAL_UINT32(0, syn_led_next_ms(&led));

    /* 5. BLINK — transition to off, then check off-phase timing */
    syn_led_update(&led);  /* transitions to off */
    TEST_ASSERT_FALSE(syn_led_is_on(&led));
    mock_tick_advance(50);
    TEST_ASSERT_EQUAL_UINT32(150, syn_led_next_ms(&led));  /* 200 - 50 */

    /* 6. FLASH mode */
    syn_led_flash(&led, 80, 120, 3);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));
    mock_tick_advance(30);
    TEST_ASSERT_EQUAL_UINT32(50, syn_led_next_ms(&led));  /* 80 - 30 */

    /* Transition to off phase */
    mock_tick_advance(50);
    syn_led_update(&led);
    TEST_ASSERT_FALSE(syn_led_is_on(&led));
    mock_tick_advance(40);
    TEST_ASSERT_EQUAL_UINT32(80, syn_led_next_ms(&led));  /* 120 - 40 */

    /* 7. PATTERN mode — '.' (short on, 1 unit) */
    syn_led_pattern(&led, ".- |?", 50);
    TEST_ASSERT_EQUAL(SYN_LED_MODE_PATTERN, led.mode);
    /* Not yet lit, first update turns on '.' */
    syn_led_update(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));
    mock_tick_advance(20);
    TEST_ASSERT_EQUAL_UINT32(30, syn_led_next_ms(&led));  /* 50 - 20 */

    /* Advance past '.' and transition to '-' */
    mock_tick_advance(30);
    syn_led_update(&led);  /* turns off after '.' */
    syn_led_update(&led);  /* starts '-' (long on, 3 units = 150ms) */
    TEST_ASSERT_TRUE(syn_led_is_on(&led));
    mock_tick_advance(100);
    TEST_ASSERT_EQUAL_UINT32(50, syn_led_next_ms(&led));  /* 150 - 100 */

    /* Advance past '-' into ' ' (short pause, 1 unit = 50ms) */
    mock_tick_advance(50);
    syn_led_update(&led);  /* turns off after '-' */
    syn_led_update(&led);  /* enters ' ' pause */
    TEST_ASSERT_EQUAL(2, led.pattern_idx);
    mock_tick_advance(20);
    TEST_ASSERT_EQUAL_UINT32(30, syn_led_next_ms(&led));  /* 50 - 20 */

    /* Advance past ' ' into '|' (long pause, 3 units = 150ms) */
    mock_tick_advance(30);
    syn_led_update(&led);  /* exits ' ', enters '|' */
    TEST_ASSERT_EQUAL(3, led.pattern_idx);
    mock_tick_advance(60);
    TEST_ASSERT_EQUAL_UINT32(90, syn_led_next_ms(&led));  /* 150 - 60 */

    /* Advance past '|' into '?' (unknown char → 0) */
    mock_tick_advance(90);
    syn_led_update(&led);
    TEST_ASSERT_EQUAL(4, led.pattern_idx);
    TEST_ASSERT_EQUAL_UINT32(0, syn_led_next_ms(&led));

    /* Skip '?' into '\0' (end of pattern → 0) */
    syn_led_update(&led);
    TEST_ASSERT_EQUAL(5, led.pattern_idx);
    TEST_ASSERT_EQUAL_UINT32(0, syn_led_next_ms(&led));
}

void run_led_tests(void)
{
    RUN_TEST(test_led_basic);
    RUN_TEST(test_led_active_low);
    RUN_TEST(test_led_service);
    RUN_TEST(test_led_pattern);
    RUN_TEST(test_led_next_ms);
}

