/**
 * @file test_exti.c
 * @brief Unity tests for syn_exti.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/drivers/syn_exti.h"

static int exti_fire_count = 0;
static SYN_GPIO_Pin exti_last_pin = 255;

static void exti_callback(SYN_GPIO_Pin pin, void *ctx)
{
    (void)ctx;
    exti_fire_count++;
    exti_last_pin = pin;
}

static int exti_ctx_value = 0;
static void exti_callback_ctx(SYN_GPIO_Pin pin, void *ctx)
{
    (void)pin;
    exti_ctx_value = *(int *)ctx;
}

static void test_exti(void)
{

    exti_fire_count = 0;
    exti_last_pin = 255;
    exti_ctx_value = 0;

    syn_exti_init();
    TEST_ASSERT_EQUAL_INT(0, syn_exti_count());

    /* Register pin 3 */
    SYN_Status st = syn_exti_register(3, SYN_EXTI_FALLING,
                                        exti_callback, NULL);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(1, syn_exti_count());

    /* Simulate ISR firing */
    syn_exti_irq_handler(3);
    TEST_ASSERT_EQUAL_INT(1, exti_fire_count);
    TEST_ASSERT_EQUAL_INT(3, exti_last_pin);

    /* Fire again */
    syn_exti_irq_handler(3);
    TEST_ASSERT_EQUAL_INT(2, exti_fire_count);

    /* Unregistered pin — no crash */
    syn_exti_irq_handler(99);
    TEST_ASSERT_EQUAL_INT(2, exti_fire_count);

    /* Register with context */
    static int val = 42;
    syn_exti_register(7, SYN_EXTI_RISING, exti_callback_ctx, &val);
    syn_exti_irq_handler(7);
    TEST_ASSERT_EQUAL_INT(42, exti_ctx_value);

    /* Unregister */
    syn_exti_unregister(3);
    syn_exti_irq_handler(3);
    TEST_ASSERT_EQUAL_INT(2, exti_fire_count);

    /* Re-register same pin (update in place) */
    syn_exti_register(3, SYN_EXTI_BOTH, exti_callback, NULL);
    syn_exti_irq_handler(3);
    TEST_ASSERT_EQUAL_INT(3, exti_fire_count);
}

void run_exti_tests(void)
{
    RUN_TEST(test_exti);
}
