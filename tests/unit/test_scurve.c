#include "syntropic/util/syn_scurve.h"
#include "unity/unity.h"

static SYN_SCurve sc;

void test_scurve_init(void)
{
    syn_scurve_init(&sc, 100);
    TEST_ASSERT_EQUAL(100, sc.p);
    TEST_ASSERT_EQUAL(0, sc.v);
    TEST_ASSERT_EQUAL(0, sc.a);
    TEST_ASSERT_TRUE(sc.done);
}

void test_scurve_motion(void)
{
    syn_scurve_init(&sc, 0);
    syn_scurve_set_constraints(&sc, 100, 10, 2);
    syn_scurve_set_target(&sc, 1000);

    TEST_ASSERT_FALSE(sc.done);

    /* Simulate motion */
    int ticks = 0;
    while (!sc.done && ticks < 2000) {
        syn_scurve_update(&sc);
        ticks++;
    }

    if (!sc.done) {
        /* Failed to finish */
    }
    TEST_ASSERT_TRUE(sc.done);
    TEST_ASSERT_EQUAL(1000, sc.p);
    TEST_ASSERT_EQUAL(0, sc.v);
    TEST_ASSERT_EQUAL(0, sc.a);
    TEST_ASSERT_TRUE(ticks > 0 && ticks < 2000);
}

void test_scurve_reverse(void)
{
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

/** set_target with dist==0 — exercises early-return done=true path */
void test_scurve_target_same(void)
{
    syn_scurve_init(&sc, 42);
    syn_scurve_set_constraints(&sc, 100, 10, 2);
    /* Set target to current position */
    syn_scurve_set_target(&sc, 42);
    TEST_ASSERT_TRUE(sc.done);
    TEST_ASSERT_EQUAL(7, sc.current_phase);

    /* update when done — returns current position */
    int32_t p = syn_scurve_update(&sc);
    TEST_ASSERT_EQUAL(42, p);
}

/** Long motion with cruise phase — exercises syn_isqrt, Tv>0, case 3 in switch */
void test_scurve_cruise_phase(void)
{
    /* Use large target so D_ad < d → Tv > 0 (cruise phase executes) */
    syn_scurve_init(&sc, 0);
    syn_scurve_set_constraints(&sc, 100, 10, 2);
    /* Tj=5, vj=50, Ta=5, vmax=100, D_ad=100*(10+5)=1500; d=5000>1500 → Tv>0 */
    syn_scurve_set_target(&sc, 5000);
    TEST_ASSERT_FALSE(sc.done);
    TEST_ASSERT_TRUE(sc.phase_ticks[3] > 0); /* cruise ticks */

    int ticks = 0;
    bool cruise_seen = false;
    while (!sc.done && ticks < 10000) {
        if (sc.current_phase == 3)
            cruise_seen = true;
        syn_scurve_update(&sc);
        ticks++;
    }
    TEST_ASSERT_TRUE(sc.done);
    TEST_ASSERT_TRUE(cruise_seen); /* case 3 was executed */
    TEST_ASSERT_EQUAL(5000, sc.p);
}

/** Small v_max relative to j_max — forces syn_isqrt branch (v_max <= vj) */
void test_scurve_isqrt_branch(void)
{
    /* j_max=10, a_max=100, v_max=5 → Tj=a_max/j_max=10, vj=j_max*10^2=1000 >> v_max=5
     * So v_max < vj → Tj = isqrt(v_max/j_max) = isqrt(0) → forced to 1.
     * BUT for isqrt to exercise the loop body (lines 33-35), we need n>0.
     * Use j_max=2, a_max=100, v_max=8 → Tj=50, vj=2*2500=5000 >> v_max=8
     * → isqrt(v_max/j_max) = isqrt(4) = 2 (exercises loop body) */
    syn_scurve_init(&sc, 0);
    syn_scurve_set_constraints(&sc, 8, 100, 2);
    syn_scurve_set_target(&sc, 200);
    TEST_ASSERT_FALSE(sc.done);

    int ticks = 0;
    while (!sc.done && ticks < 5000) {
        syn_scurve_update(&sc);
        ticks++;
    }
    TEST_ASSERT_TRUE(sc.done);
    TEST_ASSERT_EQUAL(200, sc.p);
}

/** Short motion that forces Tj-- reduction (Tj > 1, Ta == 0 path) */
void test_scurve_tj_reduction(void)
{
    /* Small distance — D_ad > d, so reduction loop runs.
     * With v_max=100, a_max=10, j_max=2, d=10:
     *   Tj=5, Ta=5, D_ad = 100 * 15 = 1500 >> 10 → loops many times
     *   Eventually Ta=0, then Tj decreases (line 91) */
    syn_scurve_init(&sc, 0);
    syn_scurve_set_constraints(&sc, 100, 10, 2);
    syn_scurve_set_target(&sc, 10); /* very short — forces Tj reduction */
    TEST_ASSERT_FALSE(sc.done);

    int ticks = 0;
    while (!sc.done && ticks < 500) {
        syn_scurve_update(&sc);
        ticks++;
    }
    TEST_ASSERT_TRUE(sc.done);
    TEST_ASSERT_EQUAL(10, sc.p);
}

void test_scurve3d_path_planning(void)
{
    SYN_SCurve3D sc3d;
    syn_scurve3d_plan(&sc3d, 0, 0, 0, 30, 40, 0, 50, 10, 2); /* 3D vector length 50 */
    TEST_ASSERT_EQUAL_UINT32(50, sc3d.total_dist);

    int32_t x = 0, y = 0, z = 0;
    int ticks = 0;
    while (syn_scurve3d_update(&sc3d, &x, &y, &z) && ticks < 2000) {
        ticks++;
    }

    /* Verify 3D target reached */
    syn_scurve3d_update(&sc3d, &x, &y, &z);
    TEST_ASSERT_EQUAL_INT(30, x);
    TEST_ASSERT_EQUAL_INT(40, y);
    TEST_ASSERT_EQUAL_INT(0, z);

    /* Zero movement 3D plan */
    syn_scurve3d_plan(&sc3d, 10, 20, 30, 10, 20, 30, 50, 10, 2);
    TEST_ASSERT_FALSE(syn_scurve3d_update(&sc3d, &x, &y, &z));
    TEST_ASSERT_EQUAL_INT(10, x);
    TEST_ASSERT_EQUAL_INT(20, y);
    TEST_ASSERT_EQUAL_INT(30, z);
}

static void test_scurve_null_and_bounds(void)
{
    syn_scurve_init(NULL, 0);
    syn_scurve_set_constraints(NULL, 0, 0, 0);
    syn_scurve_set_target(NULL, 0);

    SYN_SCurve sc_local;
    syn_scurve_init(&sc_local, 0);
    syn_scurve_set_constraints(&sc_local, -10, -5, 0);
    TEST_ASSERT_EQUAL_INT(1, sc_local.v_max);
    TEST_ASSERT_EQUAL_INT(1, sc_local.a_max);
    TEST_ASSERT_EQUAL_INT(1, sc_local.j_max);

    /* Zero constraints when setting target (lines 54, 56, 58 of syn_scurve.c) */
    sc_local.v_max = 0;
    sc_local.a_max = 0;
    sc_local.j_max = 0;
    syn_scurve_set_target(&sc_local, 100);
    TEST_ASSERT_EQUAL_INT(1, sc_local.v_max);

    /* Zero distance 3D update branch (lines 233-235 of syn_scurve.c) */
    SYN_SCurve3D sc3d;
    syn_scurve3d_plan(&sc3d, 5, 5, 5, 5, 5, 5, 10, 10, 10); /* total_dist == 0 */
    sc3d.sc_master.done = false;
    sc3d.sc_master.current_phase = 1;
    int32_t ox, oy, oz;
    syn_scurve3d_update(&sc3d, &ox, &oy, &oz);
    TEST_ASSERT_EQUAL_INT(5, ox);
}

static void test_r5_scurve3d_large_delta_no_overflow(void)
{
    SYN_SCurve3D sc3d;
    /* 60000^2 + 80000^2 = 10^10 > INT32_MAX; exact 3D Euclidean distance is 100000 */
    syn_scurve3d_plan(&sc3d, 0, 0, 0, 60000, 80000, 0, 5000, 500, 50);
    TEST_ASSERT_EQUAL_UINT32(100000U, sc3d.total_dist);

    /* Negative large deltas and INT32_MAX total_dist clamp */
    syn_scurve3d_plan(&sc3d, 800000000, 800000000, 800000000, -800000000, -800000000, -800000000,
                      10000000, 1000000, 100000);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)INT32_MAX, sc3d.total_dist);
}

void run_scurve_tests(void)
{
    RUN_TEST(test_scurve_init);
    RUN_TEST(test_scurve_motion);
    RUN_TEST(test_scurve_reverse);
    RUN_TEST(test_scurve_target_same);
    RUN_TEST(test_scurve_cruise_phase);
    RUN_TEST(test_scurve_isqrt_branch);
    RUN_TEST(test_scurve_tj_reduction);
    RUN_TEST(test_scurve3d_path_planning);
    RUN_TEST(test_scurve_null_and_bounds);
    RUN_TEST(test_r5_scurve3d_large_delta_no_overflow);
}
