/**
 * @file test_button.c
 * @brief Unity tests for syn_button.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/input/syn_button.h"

static int btn_press_count = 0;
static int btn_release_count = 0;
static int btn_long_count = 0;

static void btn_on_press(SYN_Button *b, void *ctx)
    { (void)b; (void)ctx; btn_press_count++; }
static void btn_on_release(SYN_Button *b, void *ctx)
    { (void)b; (void)ctx; btn_release_count++; }
static void btn_on_long(SYN_Button *b, void *ctx)
    { (void)b; (void)ctx; btn_long_count++; }

static void test_button(void)
{

    mock_tick_ms = 0;
    btn_press_count = 0;
    btn_release_count = 0;
    btn_long_count = 0;

    /* Pin 0 — we control it via gpio_states (from mock GPIO port) */
    mock_gpio_states[0] = 0; /* not pressed */

    SYN_Button btn;
    syn_button_init(&btn, 0, SYN_BUTTON_ACTIVE_HIGH, 50);
    syn_button_on_press(&btn, btn_on_press, NULL);
    syn_button_on_release(&btn, btn_on_release, NULL);
    syn_button_on_long_press(&btn, btn_on_long, 500, NULL);

    /* No press — should stay idle */
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(0, btn_press_count);
    TEST_ASSERT_FALSE(syn_button_is_pressed(&btn));

    /* Press the button */
    mock_gpio_states[0] = 1;
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(0, btn_press_count);

    /* Advance past debounce window */
    mock_tick_advance(60);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_press_count);
    TEST_ASSERT_TRUE(syn_button_is_pressed(&btn));

    /* Hold for long press */
    mock_tick_advance(500);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_long_count);

    /* Release */
    mock_gpio_states[0] = 0;
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(1, btn_release_count);
    TEST_ASSERT_FALSE(syn_button_is_pressed(&btn));

    /* Bounce rejection: press then release before debounce */
    btn_press_count = 0;
    mock_gpio_states[0] = 1;
    mock_tick_advance(10);
    syn_button_update(&btn);
    mock_gpio_states[0] = 0; /* bounce off */
    mock_tick_advance(10);
    syn_button_update(&btn);
    TEST_ASSERT_EQUAL_INT(0, btn_press_count);

    /* Event polling */
    btn.events = 0;
    mock_gpio_states[0] = 1;
    syn_button_update(&btn);
    mock_tick_advance(60);
    syn_button_update(&btn);
    uint8_t evts = syn_button_poll_events(&btn);
    TEST_ASSERT_TRUE(evts & SYN_BUTTON_EVT_PRESS);
    TEST_ASSERT_EQUAL_INT(0, syn_button_poll_events(&btn));
}

void run_button_tests(void)
{
    RUN_TEST(test_button);
}
