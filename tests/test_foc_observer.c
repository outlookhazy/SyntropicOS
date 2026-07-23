/**
 * @file test_foc_observer.c
 * @brief Unity unit tests for Sensorless FOC Rotor Position & Speed Observer.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/motor/syn_foc_observer.h"

static void test_foc_observer_init(void)
{
    SYN_FOCObserver obs;
    SYN_FOCObserverConfig cfg = {
        .R = Q16_HALF,                       /* 0.5 Ohm */
        .L = Q16_FROM_FLOAT(0.001),          /* 1 mH */
        .G = Q16_FROM_INT(50),               /* Observer gain */
        .dt = Q16_FROM_FLOAT(0.0001),        /* 100 us (10 kHz) */
        .Kp_pll = Q16_FROM_INT(100),
        .Ki_pll = Q16_FROM_INT(1000)
    };

    TEST_ASSERT_EQUAL(SYN_OK, syn_foc_observer_init(&obs, &cfg));
    TEST_ASSERT_EQUAL(0, syn_foc_observer_get_angle(&obs));
    TEST_ASSERT_EQUAL(0, syn_foc_observer_get_speed(&obs));
}

static void test_foc_observer_tracking(void)
{
    SYN_FOCObserver obs;
    SYN_FOCObserverConfig cfg = {
        .R = Q16_HALF,
        .L = Q16_FROM_FLOAT(0.001),
        .G = Q16_FROM_INT(10),
        .dt = Q16_FROM_FLOAT(0.0001), /* 10 kHz */
        .Kp_pll = Q16_FROM_INT(150),
        .Ki_pll = Q16_FROM_INT(6000)
    };

    syn_foc_observer_init(&obs, &cfg);

    /* Simulated motor parameters */
    q16_t dt = cfg.dt;
    q16_t target_omega = Q16_FROM_INT(100); /* 100 rad/s */
    q16_t Ke = Q16_FROM_FLOAT(0.05);        /* Back-EMF constant */

    q16_t actual_theta = 0;
    q16_t I_alpha = 0;
    q16_t I_beta  = 0;

    /* Run 6000 steps (0.6 s) of simulated motor rotation */
    for (int step = 0; step < 6000; step++) {
        actual_theta += q16_mul(target_omega, dt);
        while (actual_theta >= Q16_2_PI) actual_theta -= Q16_2_PI;

        /* Simulated Back-EMF: E_alpha = -Ke*omega*sin(theta), E_beta = Ke*omega*cos(theta) */
        q16_t E_mag = q16_mul(Ke, target_omega);
        q16_t E_alpha = -q16_mul(E_mag, q16_sin(actual_theta));
        q16_t E_beta  =  q16_mul(E_mag, q16_cos(actual_theta));

        /* Applied voltage V = 0 (un-driven back-EMF current generator) */
        q16_t V_alpha = 0;
        q16_t V_beta  = 0;

        /* Update phase current simulation: dI/dt = (V - R*I - E) / L */
        q16_t dI_a = q16_div(V_alpha - q16_mul(cfg.R, I_alpha) - E_alpha, cfg.L);
        q16_t dI_b = q16_div(V_beta  - q16_mul(cfg.R, I_beta)  - E_beta,  cfg.L);

        I_alpha += q16_mul(dI_a, dt);
        I_beta  += q16_mul(dI_b, dt);

        syn_foc_observer_update(&obs, V_alpha, V_beta, I_alpha, I_beta);
    }

    /* Check estimated speed and angle convergence */
    q16_t est_speed = syn_foc_observer_get_speed(&obs);
    q16_t est_angle = syn_foc_observer_get_angle(&obs);

    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(20.0), target_omega, est_speed);

    /* Angle tracking error should be small (within observer phase lag tolerance) */
    q16_t angle_err = q16_abs(est_angle - actual_theta);
    if (angle_err > Q16_PI) angle_err = Q16_2_PI - angle_err;
    TEST_ASSERT_TRUE(angle_err < Q16_FROM_FLOAT(0.50));
}

static void test_foc_observer_reset_and_invalid(void)
{
    SYN_FOCObserver obs;
    SYN_FOCObserverConfig cfg = {
        .R = Q16_HALF,
        .L = Q16_FROM_FLOAT(0.001),
        .G = Q16_FROM_INT(50),
        .dt = Q16_FROM_FLOAT(0.0001),
        .Kp_pll = Q16_FROM_INT(100),
        .Ki_pll = Q16_FROM_INT(1000)
    };

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
    SYN_FOCObserverConfig cfg = {
        .R = Q16_HALF,
        .L = Q16_FROM_FLOAT(0.001),
        .G = Q16_FROM_INT(10),
        .dt = Q16_FROM_FLOAT(0.0001),
        .Kp_pll = Q16_FROM_INT(150),
        .Ki_pll = Q16_FROM_INT(6000)
    };

    syn_foc_observer_init(&obs, &cfg);

    q16_t dt = cfg.dt;
    q16_t target_omega = -Q16_FROM_INT(100); /* -100 rad/s */
    q16_t Ke = Q16_FROM_FLOAT(0.05);

    q16_t actual_theta = 0;
    q16_t I_alpha = 0;
    q16_t I_beta  = 0;

    for (int step = 0; step < 6000; step++) {
        actual_theta += q16_mul(target_omega, dt);
        while (actual_theta < 0) actual_theta += Q16_2_PI;

        q16_t E_mag = q16_mul(Ke, q16_abs(target_omega));
        q16_t E_alpha = -q16_mul(E_mag, q16_sin(actual_theta));
        q16_t E_beta  =  q16_mul(E_mag, q16_cos(actual_theta));

        q16_t V_alpha = 0;
        q16_t V_beta  = 0;

        q16_t dI_a = q16_div(V_alpha - q16_mul(cfg.R, I_alpha) - E_alpha, cfg.L);
        q16_t dI_b = q16_div(V_beta  - q16_mul(cfg.R, I_beta)  - E_beta,  cfg.L);

        I_alpha += q16_mul(dI_a, dt);
        I_beta  += q16_mul(dI_b, dt);

        syn_foc_observer_update(&obs, V_alpha, V_beta, I_alpha, I_beta);
    }

    q16_t est_speed = syn_foc_observer_get_speed(&obs);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(20.0), target_omega, est_speed);
}

void run_foc_observer_tests(void)
{
    RUN_TEST(test_foc_observer_init);
    RUN_TEST(test_foc_observer_tracking);
    RUN_TEST(test_foc_observer_reverse);
    RUN_TEST(test_foc_observer_reset_and_invalid);
}
