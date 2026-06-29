/**
 * @file test_sensor.c
 * @brief Unity tests for syn_sensor.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/sensor/syn_sensor.h"

static int16_t mock_sensor_value = 0;
static int sensor_high_count = 0;
static int sensor_low_count = 0;

static int16_t mock_sensor_read(void *ctx)
{
    (void)ctx;
    return mock_sensor_value;
}

static void sensor_on_high(SYN_Sensor *s, int16_t v, void *ctx)
{
    (void)s; (void)v; (void)ctx;
    sensor_high_count++;
}

static void sensor_on_low(SYN_Sensor *s, int16_t v, void *ctx)
{
    (void)s; (void)v; (void)ctx;
    sensor_low_count++;
}

static void test_sensor(void)
{

    mock_tick_ms = 0;
    sensor_high_count = 0;
    sensor_low_count  = 0;

    SYN_Sensor sensor;
    SYN_FilterEMA ema;
    syn_filter_ema_init(&ema, 255);  /* alpha=1.0 (no filtering for testing) */

    syn_sensor_init(&sensor, "test", mock_sensor_read, NULL);
    syn_sensor_set_interval(&sensor, 100);
    syn_sensor_set_filter_ema(&sensor, &ema);
    syn_sensor_set_threshold(&sensor, 500, 50, sensor_on_high, sensor_on_low, NULL);

    /* Not time yet */
    mock_sensor_value = 0;
    TEST_ASSERT_FALSE(syn_sensor_update(&sensor));

    /* Time elapsed */
    mock_tick_advance(100);
    TEST_ASSERT_TRUE(syn_sensor_update(&sensor));
    TEST_ASSERT_EQUAL_INT(0, syn_sensor_value(&sensor));
    TEST_ASSERT_EQUAL_INT(0, sensor_high_count);

    /* Value crosses high threshold */
    mock_sensor_value = 600;
    mock_tick_advance(100);
    syn_sensor_update(&sensor);
    TEST_ASSERT_EQUAL_INT(1, sensor_high_count);

    /* Value drops below low threshold */
    mock_sensor_value = 400;
    mock_tick_advance(100);
    syn_sensor_update(&sensor);
    TEST_ASSERT_EQUAL_INT(1, sensor_low_count);

    /* Disable/enable */
    syn_sensor_enable(&sensor, false);
    mock_tick_advance(100);
    TEST_ASSERT_FALSE(syn_sensor_update(&sensor));

    /* Force read (clear filter first so EMA state doesn't interfere) */
    syn_sensor_clear_filter(&sensor);
    mock_sensor_value = 123;
    int16_t v = syn_sensor_read_now(&sensor);
    TEST_ASSERT_EQUAL_INT(123, v);
}

void run_sensor_tests(void)
{
    RUN_TEST(test_sensor);
}
