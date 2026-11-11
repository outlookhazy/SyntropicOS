/**
 * @file test_sensor.c
 * @brief Unity tests for syn_sensor.
 */

#include "mocks/mock_port.h"
#include "syntropic/sensor/syn_sensor.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

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
    (void)s;
    (void)v;
    (void)ctx;
    sensor_high_count++;
}

static void sensor_on_low(SYN_Sensor *s, int16_t v, void *ctx)
{
    (void)s;
    (void)v;
    (void)ctx;
    sensor_low_count++;
}

static void test_sensor(void)
{
    mock_tick_ms = 0;
    sensor_high_count = 0;
    sensor_low_count = 0;

    SYN_Sensor sensor;
    SYN_FilterEMA ema;
    syn_filter_ema_init(&ema, 255); /* alpha=1.0 (no filtering for testing) */

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

static void test_sensor_filters_and_edge_cases(void)
{
    mock_tick_ms = 0;
    sensor_high_count = 0;
    sensor_low_count = 0;

    SYN_Sensor s;
    syn_sensor_init(&s, "temp", mock_sensor_read, NULL);

    /* 1. Moving Average (MA) filter */
    SYN_FilterMA ma;
    syn_filter_ma_init(&ma, 4);
    syn_sensor_set_filter_ma(&s, &ma);

    mock_sensor_value = 100;
    syn_sensor_read_now(&s);
    TEST_ASSERT_EQUAL_INT(100, s.filtered);

    mock_sensor_value = 200;
    syn_sensor_read_now(&s);
    TEST_ASSERT_EQUAL_INT(150, s.filtered);

    /* 2. Median filter */
    SYN_FilterMedian median;
    syn_filter_median_init(&median, 3);
    syn_sensor_set_filter_median(&s, &median);

    mock_sensor_value = 10;
    syn_sensor_read_now(&s);
    TEST_ASSERT_EQUAL_INT(10, s.filtered);

    mock_sensor_value = 30;
    syn_sensor_read_now(&s);
    TEST_ASSERT_EQUAL_INT(30, s.filtered);

    mock_sensor_value = 20;
    syn_sensor_read_now(&s);
    TEST_ASSERT_EQUAL_INT(20, s.filtered); // median of 10, 30, 20 is 20

    /* 3. Threshold Callbacks NULL Safety */
    syn_sensor_clear_filter(&s);
    syn_sensor_set_threshold(&s, 500, 50, NULL, NULL, NULL);

    /* Cross high threshold */
    mock_sensor_value = 600;
    syn_sensor_read_now(&s);
    TEST_ASSERT_TRUE(syn_hyst_state(&s.hyst));

    /* Cross low threshold */
    mock_sensor_value = 400;
    syn_sensor_read_now(&s);
    TEST_ASSERT_FALSE(syn_hyst_state(&s.hyst));

    /* 4. Threshold Callback Triggering & Clearing */
    syn_sensor_set_threshold(&s, 500, 50, sensor_on_high, sensor_on_low, NULL);

    mock_sensor_value = 600;
    syn_sensor_read_now(&s);
    TEST_ASSERT_EQUAL_INT(1, sensor_high_count);

    mock_sensor_value = 400;
    syn_sensor_read_now(&s);
    TEST_ASSERT_EQUAL_INT(1, sensor_low_count);

    /* Clear threshold */
    syn_sensor_clear_threshold(&s);
    sensor_high_count = 0;
    sensor_low_count = 0;
    mock_sensor_value = 600;
    syn_sensor_read_now(&s);
    TEST_ASSERT_EQUAL_INT(0, sensor_high_count);

    /* 5. Statistics Window integration */
    SYN_Signal stats;
    int32_t stats_buf[8];
    syn_signal_init(&stats, stats_buf, 8);
    syn_sensor_set_stats(&s, &stats);

    mock_sensor_value = 50;
    syn_sensor_read_now(&s);
    TEST_ASSERT_EQUAL_INT(50, (int)syn_signal_latest(&stats));

    syn_sensor_set_stats(&s, NULL);

    /* 6. Enable / Disable resetting poll tick */
    syn_sensor_enable(&s, false);
    TEST_ASSERT_FALSE(s.enabled);

    mock_tick_advance(100);
    syn_sensor_enable(&s, true);
    TEST_ASSERT_TRUE(s.enabled);
    TEST_ASSERT_EQUAL_UINT32(mock_tick_ms, s.last_poll_tick);

    /* 7. Bulk Service update */
    SYN_Sensor sensors[2];
    syn_sensor_init(&sensors[0], "s0", mock_sensor_read, NULL);
    syn_sensor_init(&sensors[1], "s1", mock_sensor_read, NULL);
    syn_sensor_set_interval(&sensors[0], 100);
    syn_sensor_set_interval(&sensors[1], 100);

    mock_tick_advance(100);
    mock_sensor_value = 99;
    syn_sensor_service(sensors, 2);
    TEST_ASSERT_EQUAL_INT(99, sensors[0].filtered);
    TEST_ASSERT_EQUAL_INT(99, sensors[1].filtered);

    syn_sensor_service(NULL, 0);

    /* 8. Default filter type case (filter != NULL, but filter_type is NONE) */
    s.filter = &ma;
    s.filter_type = SYN_SENSOR_FILTER_NONE;
    mock_sensor_value = 42;
    syn_sensor_read_now(&s);
    TEST_ASSERT_EQUAL_INT(42, s.filtered);
}

typedef struct {
    int32_t latest_in_cb;
    size_t count_in_cb;
} SensorStatsCbCapture;

static void sensor_on_high_check_stats(SYN_Sensor *s, int16_t v, void *ctx)
{
    (void)v;
    SensorStatsCbCapture *cap = (SensorStatsCbCapture *)ctx;
    cap->count_in_cb = s->stats->count;
    cap->latest_in_cb = syn_signal_latest(s->stats);
}

static void test_sensor_stats_updated_before_threshold_callback(void)
{
    SYN_Sensor s;
    SYN_Signal stats;
    int32_t stats_buf[4];
    SensorStatsCbCapture cap = {0, 0};

    syn_sensor_init(&s, "volt", mock_sensor_read, NULL);
    syn_signal_init(&stats, stats_buf, 4);
    syn_sensor_set_stats(&s, &stats);
    syn_sensor_set_threshold(&s, 500, 20, sensor_on_high_check_stats, NULL, &cap);

    mock_sensor_value = 625;
    (void)syn_sensor_read_now(&s);

    TEST_ASSERT_EQUAL_UINT32(1U, (uint32_t)cap.count_in_cb);
    TEST_ASSERT_EQUAL_INT32(625, cap.latest_in_cb);
}

void run_sensor_tests(void)
{
    RUN_TEST(test_sensor);
    RUN_TEST(test_sensor_filters_and_edge_cases);
    RUN_TEST(test_sensor_stats_updated_before_threshold_callback);
}
