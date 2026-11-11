/**
 * @file test_foc_observer.c
 * @brief Unity unit tests for Sensorless FOC Rotor Position & Speed Observer.
 */

#include "mocks/mock_port.h"
#include "syntropic/motor/syn_foc_observer.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static void test_foc_observer_init(void)
{
    SYN_FOCObserver obs;
    SYN_FOCObserverConfig cfg = {.R = Q16_HALF,                /* 0.5 Ohm */
                                 .L = Q16_FROM_FLOAT(0.001),   /* 1 mH */
                                 .G = Q16_FROM_INT(50),        /* Observer gain */
                                 .dt = Q16_FROM_FLOAT(0.0001), /* 100 us (10 kHz) */
                                 .Kp_pll = Q16_FROM_INT(100),
                                 .Ki_pll = Q16_FROM_INT(1000)};

    TEST_ASSERT_EQUAL(SYN_OK, syn_foc_observer_init(&obs, &cfg));
    TEST_ASSERT_EQUAL(0, syn_foc_observer_get_angle(&obs));
    TEST_ASSERT_EQUAL(0, syn_foc_observer_get_speed(&obs));
}

static void test_foc_observer_tracking(void)
{
    SYN_FOCObserver obs;
    SYN_FOCObserverConfig cfg = {.R = Q16_HALF,
                                 .L = Q16_FROM_FLOAT(0.001),
                                 .G = Q16_FROM_INT(10),
                                 .dt = Q16_FROM_FLOAT(0.0001), /* 10 kHz */
                                 .Kp_pll = Q16_FROM_INT(150),
                                 .Ki_pll = Q16_FROM_INT(6000)};

    syn_foc_observer_init(&obs, &cfg);

    /* Simulated motor parameters */
    q16_t dt = cfg.dt;
    q16_t target_omega = Q16_FROM_INT(100); /* 100 rad/s */
    q16_t Ke = Q16_FROM_FLOAT(0.05);        /* Back-EMF constant */

    q16_t actual_theta = 0;
    q16_t I_alpha = 0;
    q16_t I_beta = 0;

    /* Run 6000 steps (0.6 s) of simulated motor rotation */
    for (int step = 0; step < 6000; step++) {
        actual_theta += q16_mul(target_omega, dt);
        while (actual_theta >= Q16_2_PI)
            actual_theta -= Q16_2_PI;

        /* Simulated Back-EMF: E_alpha = -Ke*omega*sin(theta), E_beta = Ke*omega*cos(theta) */
        q16_t E_mag = q16_mul(Ke, target_omega);
        q16_t E_alpha = -q16_mul(E_mag, q16_sin(actual_theta));
        q16_t E_beta = q16_mul(E_mag, q16_cos(actual_theta));

        /* Applied voltage V = 0 (un-driven back-EMF current generator) */
        q16_t V_alpha = 0;
        q16_t V_beta = 0;

        /* Update phase current simulation: dI/dt = (V - R*I - E) / L */
        q16_t dI_a = q16_div(V_alpha - q16_mul(cfg.R, I_alpha) - E_alpha, cfg.L);
        q16_t dI_b = q16_div(V_beta - q16_mul(cfg.R, I_beta) - E_beta, cfg.L);

        I_alpha += q16_mul(dI_a, dt);
        I_beta += q16_mul(dI_b, dt);

        syn_foc_observer_update(&obs, V_alpha, V_beta, I_alpha, I_beta);
    }

    /* Check estimated speed and angle convergence */
    q16_t est_speed = syn_foc_observer_get_speed(&obs);
    q16_t est_angle = syn_foc_observer_get_angle(&obs);

    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(20.0), target_omega, est_speed);

    /* Angle tracking error should be small (within observer phase lag tolerance) */
    q16_t angle_err = q16_abs(est_angle - actual_theta);
    if (angle_err > Q16_PI)
        angle_err = Q16_2_PI - angle_err;
    TEST_ASSERT_TRUE(angle_err < Q16_FROM_FLOAT(0.50));
}

static void test_foc_observer_reset_and_invalid(void)
{
    SYN_FOCObserver obs;
    SYN_FOCObserverConfig cfg = {.R = Q16_HALF,
                                 .L = Q16_FROM_FLOAT(0.001),
                                 .G = Q16_FROM_INT(50),
                                 .dt = Q16_FROM_FLOAT(0.0001),
                                 .Kp_pll = Q16_FROM_INT(100),
                                 .Ki_pll = Q16_FROM_INT(1000)};

    syn_foc_observer_init(&obs, &cfg);
    syn_foc_observer_update(&obs, Q16_ONE, Q16_ONE, Q16_HALF, Q16_HALF);

    syn_foc_observer_reset(&obs);
    TEST_ASSERT_EQUAL(0, syn_foc_observer_get_angle(&obs));
    TEST_ASSERT_EQUAL(0, syn_foc_observer_get_speed(&obs));

    /* Invalid parameters */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_foc_observer_init(NULL, &cfg));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_foc_observer_init(&obs, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_foc_observer_update(NULL, 0, 0, 0, 0));

    cfg.R = 0;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_foc_observer_init(&obs, &cfg));

    syn_foc_observer_reset(NULL);
    TEST_ASSERT_EQUAL(0, syn_foc_observer_get_angle(NULL));
    TEST_ASSERT_EQUAL(0, syn_foc_observer_get_speed(NULL));
}

static void test_foc_observer_reverse(void)
{
    SYN_FOCObserver obs;
    SYN_FOCObserverConfig cfg = {.R = Q16_HALF,
                                 .L = Q16_FROM_FLOAT(0.001),
                                 .G = Q16_FROM_INT(10),
                                 .dt = Q16_FROM_FLOAT(0.0001),
                                 .Kp_pll = Q16_FROM_INT(150),
                                 .Ki_pll = Q16_FROM_INT(6000)};

    syn_foc_observer_init(&obs, &cfg);

    q16_t dt = cfg.dt;
    q16_t target_omega = -Q16_FROM_INT(100); /* -100 rad/s */
    q16_t Ke = Q16_FROM_FLOAT(0.05);

    q16_t actual_theta = 0;
    q16_t I_alpha = 0;
    q16_t I_beta = 0;

    for (int step = 0; step < 6000; step++) {
        actual_theta += q16_mul(target_omega, dt);
        while (actual_theta < 0)
            actual_theta += Q16_2_PI;

        q16_t E_mag = q16_mul(Ke, q16_abs(target_omega));
        q16_t E_alpha = -q16_mul(E_mag, q16_sin(actual_theta));
        q16_t E_beta = q16_mul(E_mag, q16_cos(actual_theta));

        q16_t V_alpha = 0;
        q16_t V_beta = 0;

        q16_t dI_a = q16_div(V_alpha - q16_mul(cfg.R, I_alpha) - E_alpha, cfg.L);
        q16_t dI_b = q16_div(V_beta - q16_mul(cfg.R, I_beta) - E_beta, cfg.L);

        I_alpha += q16_mul(dI_a, dt);
        I_beta += q16_mul(dI_b, dt);

        syn_foc_observer_update(&obs, V_alpha, V_beta, I_alpha, I_beta);
    }

    q16_t est_speed = syn_foc_observer_get_speed(&obs);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(20.0), target_omega, est_speed);
}

static void test_foc_observer_stall_decay_and_restart(void)
{
    SYN_FOCObserver obs;
    SYN_FOCObserverConfig cfg = {.R = Q16_HALF,
                                 .L = Q16_FROM_FLOAT(0.001),
                                 .G = Q16_FROM_INT(10),
                                 .dt = Q16_FROM_FLOAT(0.0001),
                                 .Kp_pll = Q16_FROM_INT(150),
                                 .Ki_pll = Q16_FROM_INT(6000)};

    TEST_ASSERT_EQUAL(SYN_OK, syn_foc_observer_init(&obs, &cfg));

    q16_t dt = cfg.dt;
    q16_t Ke = Q16_FROM_FLOAT(0.05);
    q16_t actual_theta = 0;
    q16_t I_alpha = 0;
    q16_t I_beta = 0;

    /* 1. Spin forward at +100 rad/s until converged */
    q16_t target_omega = Q16_FROM_INT(100);
    for (int step = 0; step < 6000; step++) {
        actual_theta += q16_mul(target_omega, dt);
        while (actual_theta >= Q16_2_PI) {
            actual_theta -= Q16_2_PI;
        }
        q16_t E_mag = q16_mul(Ke, target_omega);
        q16_t E_alpha = -q16_mul(E_mag, q16_sin(actual_theta));
        q16_t E_beta = q16_mul(E_mag, q16_cos(actual_theta));
        I_alpha += q16_mul(q16_div(-q16_mul(cfg.R, I_alpha) - E_alpha, cfg.L), dt);
        I_beta += q16_mul(q16_div(-q16_mul(cfg.R, I_beta) - E_beta, cfg.L), dt);
        syn_foc_observer_update(&obs, 0, 0, I_alpha, I_beta);
    }
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(20.0), target_omega, syn_foc_observer_get_speed(&obs));

    /* 2. Rotor stalls (V = 0, I = 0): estimated speed must decay to 0 and stop integrating theta */
    for (int step = 0; step < 1000; step++) {
        syn_foc_observer_update(&obs, 0, 0, 0, 0);
    }
    TEST_ASSERT_EQUAL_INT32(0, syn_foc_observer_get_speed(&obs));
    q16_t stalled_angle = syn_foc_observer_get_angle(&obs);
    syn_foc_observer_update(&obs, 0, 0, 0, 0);
    TEST_ASSERT_EQUAL_INT32(stalled_angle, syn_foc_observer_get_angle(&obs));

    /* 3. Restart in reverse (-100 rad/s) without reset, then stall again */
    target_omega = -Q16_FROM_INT(100);
    I_alpha = 0;
    I_beta = 0;
    for (int step = 0; step < 6000; step++) {
        actual_theta += q16_mul(target_omega, dt);
        while (actual_theta < 0) {
            actual_theta += Q16_2_PI;
        }
        q16_t E_mag = q16_mul(Ke, q16_abs(target_omega));
        q16_t E_alpha = -q16_mul(E_mag, q16_sin(actual_theta));
        q16_t E_beta = q16_mul(E_mag, q16_cos(actual_theta));
        I_alpha += q16_mul(q16_div(-q16_mul(cfg.R, I_alpha) - E_alpha, cfg.L), dt);
        I_beta += q16_mul(q16_div(-q16_mul(cfg.R, I_beta) - E_beta, cfg.L), dt);
        syn_foc_observer_update(&obs, 0, 0, I_alpha, I_beta);
    }
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(20.0), target_omega, syn_foc_observer_get_speed(&obs));

    for (int step = 0; step < 1000; step++) {
        syn_foc_observer_update(&obs, 0, 0, 0, 0);
    }
    TEST_ASSERT_EQUAL_INT32(0, syn_foc_observer_get_speed(&obs));
}

static void test_foc_observer_hv_bemf_and_extreme_saturation(void)
{
    /* 1. Step 120 -> 220 V back-EMF (220^2 = 48400 wraps (q16_t)E_sq < 0 and freezes PLL) */
    SYN_FOCObserver obs;
    SYN_FOCObserverConfig cfg = {.R = Q16_HALF,
                                 .L = Q16_FROM_FLOAT(0.002),
                                 .G = Q16_FROM_INT(400),
                                 .dt = Q16_FROM_FLOAT(0.0001),
                                 .Kp_pll = Q16_FROM_INT(50),
                                 .Ki_pll = Q16_FROM_INT(2000)};
    TEST_ASSERT_EQUAL(SYN_OK, syn_foc_observer_init(&obs, &cfg));

    q16_t dt_over_L = q16_div(cfg.dt, cfg.L);
    q16_t target_omega = Q16_FROM_INT(120);
    q16_t actual_theta = 0;
    q16_t I_alpha = 0;
    q16_t I_beta = 0;
    for (int step = 0; step < 8000; step++) {
        if (step == 3000) {
            target_omega = Q16_FROM_INT(220);
        }
        actual_theta += q16_mul(target_omega, cfg.dt);
        while (actual_theta >= Q16_2_PI) {
            actual_theta -= Q16_2_PI;
        }
        q16_t E_alpha = -q16_mul(target_omega, q16_sin(actual_theta));
        q16_t E_beta = q16_mul(target_omega, q16_cos(actual_theta));
        I_alpha += q16_mul(-q16_mul(cfg.R, I_alpha) - E_alpha, dt_over_L);
        I_beta += q16_mul(-q16_mul(cfg.R, I_beta) - E_beta, dt_over_L);
        TEST_ASSERT_EQUAL(SYN_OK, syn_foc_observer_update(&obs, 0, 0, I_alpha, I_beta));
    }
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(15), target_omega, syn_foc_observer_get_speed(&obs));

    /* 2. Extreme full-scale inputs & large dt wrap without signed overflow or hang */
    SYN_FOCObserverConfig ext_cfg = {.R = Q16_FROM_INT(100),
                                     .L = Q16_ONE,
                                     .G = INT32_MAX,
                                     .dt = Q16_FROM_INT(100),
                                     .Kp_pll = INT32_MAX,
                                     .Ki_pll = INT32_MAX};
    TEST_ASSERT_EQUAL(SYN_OK, syn_foc_observer_init(&obs, &ext_cfg));
    obs.E_alpha_est = INT32_MIN;
    obs.E_beta_est = INT32_MAX;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_foc_observer_update(&obs, INT32_MAX, INT32_MIN, INT32_MIN, INT32_MAX));
    TEST_ASSERT_TRUE(syn_foc_observer_get_angle(&obs) >= 0 &&
                     syn_foc_observer_get_angle(&obs) < Q16_2_PI);
}

void run_foc_observer_tests(void)
{
    RUN_TEST(test_foc_observer_init);
    RUN_TEST(test_foc_observer_tracking);
    RUN_TEST(test_foc_observer_reverse);
    RUN_TEST(test_foc_observer_reset_and_invalid);
    RUN_TEST(test_foc_observer_stall_decay_and_restart);
    RUN_TEST(test_foc_observer_hv_bemf_and_extreme_saturation);
}
