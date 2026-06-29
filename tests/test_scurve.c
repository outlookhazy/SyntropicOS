#include "unity/unity.h"
#include "syntropic/util/syn_scurve.h"

static SYN_SCurve sc;

void test_scurve_init(void) {
    syn_scurve_init(&sc, 100);
    TEST_ASSERT_EQUAL(100, sc.p);
    TEST_ASSERT_EQUAL(0, sc.v);
    TEST_ASSERT_EQUAL(0, sc.a);
    TEST_ASSERT_TRUE(sc.done);
}

void test_scurve_motion(void) {
    syn_scurve_init(&sc, 0);
    syn_scurve_set_constraints(&sc, 100, 10, 2);
    syn_scurve_set_target(&sc, 1000);
    
    TEST_ASSERT_FALSE(sc.done);
    
    // Simulate motion
    int ticks = 0;
    while (!sc.done && ticks < 2000) {
        syn_scurve_update(&sc);
        ticks++;
    }
    
    if (!sc.done) {
        printf("Failed to finish! p=%d, v=%d, a=%d, target=%d\n", sc.p, sc.v, sc.a, sc.target_p);
    }
    TEST_ASSERT_TRUE(sc.done);
    TEST_ASSERT_EQUAL(1000, sc.p);
    TEST_ASSERT_EQUAL(0, sc.v);
    TEST_ASSERT_EQUAL(0, sc.a);
    TEST_ASSERT_TRUE(ticks > 0 && ticks < 2000);
}

void test_scurve_reverse(void) {
    syn_scurve_init(&sc, 500);
    syn_scurve_set_constraints(&sc, 50, 5, 1);
    syn_scurve_set_target(&sc, 0);
    
    int ticks = 0;
    while (!sc.done && ticks < 2000) {
        syn_scurve_update(&sc);
        ticks++;
    }
    
    TEST_ASSERT_TRUE(sc.done);
    TEST_ASSERT_EQUAL(0, sc.p);
    TEST_ASSERT_EQUAL(0, sc.v);
}

void run_scurve_tests(void) {
    RUN_TEST(test_scurve_init);
    RUN_TEST(test_scurve_motion);
    RUN_TEST(test_scurve_reverse);
}
