/**
 * @file test_bldc_6step.c
 * @brief Unity tests for Zero-Heap 6-Step BLDC Motor Commutation Driver.
 */

#include "syntropic/motor/syn_bldc_6step.h"
#include "unity/unity.h"

void test_bldc_6step_init_defaults(void)
{
    SYN_BLDC_6Step bldc;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_init(&bldc, NULL));

    TEST_ASSERT_EQUAL_INT(SYN_BLDC_STATE_STOPPED, bldc.state);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_DIR_CW, bldc.direction);
    TEST_ASSERT_EQUAL_UINT8(4, bldc.config.pole_pairs);
    TEST_ASSERT_EQUAL_UINT16(0, bldc.duty);
    TEST_ASSERT_EQUAL_UINT32(0, bldc.rpm);
}

void test_bldc_6step_hall_commutation_cw(void)
{
    SYN_BLDC_6Step bldc;
    syn_bldc_6step_init(&bldc, NULL);

    SYN_BLDC_PhaseOutputs out;
    syn_bldc_6step_start(&bldc);
    syn_bldc_6step_set_duty(&bldc, 500);

    /* Step 1: Hall 0b101(5) -> U+ V- */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_set_hall(&bldc, 5, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_PWM, out.u);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_LOW, out.v);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.w);
    TEST_ASSERT_EQUAL_UINT16(500, out.duty);

    /* Step 2: Hall 0b001(1) -> U+ W- */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_set_hall(&bldc, 1, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_PWM, out.u);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.v);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_LOW, out.w);

    /* Step 3: Hall 0b011(3) -> V+ W- */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_set_hall(&bldc, 3, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.u);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_PWM, out.v);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_LOW, out.w);

    /* Step 4: Hall 0b010(2) -> V+ U- */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_set_hall(&bldc, 2, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_LOW, out.u);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_PWM, out.v);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.w);

    /* Step 5: Hall 0b110(6) -> W+ U- */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_set_hall(&bldc, 6, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_LOW, out.u);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.v);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_PWM, out.w);

    /* Step 6: Hall 0b100(4) -> W+ V- */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_set_hall(&bldc, 4, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.u);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_LOW, out.v);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_PWM, out.w);
}

void test_bldc_6step_invalid_hall_fault(void)
{
    SYN_BLDC_6Step bldc;
    syn_bldc_6step_init(&bldc, NULL);

    SYN_BLDC_PhaseOutputs out;
    syn_bldc_6step_start(&bldc);

    /* Invalid Hall 0b000 */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_bldc_6step_set_hall(&bldc, 0, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_STATE_FAULT, bldc.state);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.u);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.v);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.w);

    /* Invalid Hall 0b111 */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_bldc_6step_set_hall(&bldc, 7, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_STATE_FAULT, bldc.state);
}

void test_bldc_6step_direction_and_stop(void)
{
    SYN_BLDC_6Step bldc;
    syn_bldc_6step_init(&bldc, NULL);

    SYN_BLDC_PhaseOutputs out;
    syn_bldc_6step_start(&bldc);
    syn_bldc_6step_set_direction(&bldc, SYN_BLDC_DIR_CCW);
    syn_bldc_6step_set_duty(&bldc, 750);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_set_hall(&bldc, 5, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_LOW, out.u);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_PWM, out.v);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.w);

    syn_bldc_6step_stop(&bldc, &out);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_STATE_STOPPED, bldc.state);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.u);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.v);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.w);
}

void test_bldc_6step_speed_calculation(void)
{
    SYN_BLDC_6Step bldc;
    syn_bldc_6step_init(&bldc, NULL);
    syn_bldc_6step_start(&bldc);

    /* Initial tick */
    syn_bldc_6step_update_speed(&bldc, 1000, 0);

    /* Simulate 48 Hall transitions in 100ms (1000ms -> 1100ms) */
    SYN_BLDC_PhaseOutputs out;
    for (int i = 0; i < 48; i++) {
        syn_bldc_6step_set_hall(&bldc, 5, &out);
    }

    uint32_t rpm = syn_bldc_6step_update_speed(&bldc, 1100, 0);
    /* 48 transitions / (6 * 4 pole pairs) = 2 mechanical revs in 100ms = 1200 RPM */
    TEST_ASSERT_EQUAL_UINT32(1200, rpm);
}

void test_bldc_6step_null_params_and_edge_cases(void)
{
    /* Null check validations */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_bldc_6step_init(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_bldc_6step_set_hall(NULL, 1, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_bldc_6step_set_duty(NULL, 500));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_bldc_6step_set_direction(NULL, SYN_BLDC_DIR_CW));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_bldc_6step_start(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_bldc_6step_stop(NULL, NULL));
    TEST_ASSERT_EQUAL_UINT32(0, syn_bldc_6step_update_speed(NULL, 100, 1000));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_bldc_6step_get_phase_outputs(NULL, NULL));

    SYN_BLDC_6Step bldc;
    SYN_BLDC_PhaseOutputs out;

    /* Custom configuration initialization */
    SYN_BLDC_Config cfg = {.pole_pairs = 2, .pwm_frequency = 15000};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_init(&bldc, &cfg));
    TEST_ASSERT_EQUAL_UINT8(2, bldc.config.pole_pairs);
    TEST_ASSERT_EQUAL_UINT16(15000, bldc.config.pwm_frequency);

    /* Duty clamping test */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_set_duty(&bldc, 1500));
    TEST_ASSERT_EQUAL_UINT16(1000, bldc.duty);

    /* Set hall when STOPPED (out != NULL) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_set_hall(&bldc, 5, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.u);

    /* Stop with NULL out */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_stop(&bldc, NULL));

    /* Get phase outputs when STOPPED */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_get_phase_outputs(&bldc, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.u);

    /* Get phase outputs when RUNNING (CW & CCW) */
    syn_bldc_6step_start(&bldc);
    syn_bldc_6step_set_hall(&bldc, 5, NULL);

    syn_bldc_6step_set_direction(&bldc, SYN_BLDC_DIR_CW);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_get_phase_outputs(&bldc, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_PWM, out.u);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_LOW, out.v);

    syn_bldc_6step_set_direction(&bldc, SYN_BLDC_DIR_CCW);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_get_phase_outputs(&bldc, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_LOW, out.u);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_PWM, out.v);

    /* Speed update with dt < 100ms */
    syn_bldc_6step_update_speed(&bldc, 1000, 0);
    uint32_t rpm_short = syn_bldc_6step_update_speed(&bldc, 1050, 0);
    TEST_ASSERT_EQUAL_UINT32(0, rpm_short);

    /* Speed update with closed-loop PID active and pole_pairs = 0 fallback */
    bldc.config.pole_pairs = 0;
    bldc.speed_pid_active = true;
    for (int i = 0; i < 24; i++) {
        syn_bldc_6step_set_hall(&bldc, 5, NULL);
    }
    uint32_t rpm_pid = syn_bldc_6step_update_speed(&bldc, 1200, 1500);
    /* 24 transitions / (6 * 4 default pole pairs) = 1 rev in 200ms -> 300 RPM */
    TEST_ASSERT_EQUAL_UINT32(300, rpm_pid);
}

void test_bldc_6step_fault_latch_high_rpm_and_pid_clamp(void)
{
    SYN_BLDC_6Step bldc;
    SYN_BLDC_PhaseOutputs out;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_init(&bldc, NULL));
    syn_bldc_6step_start(&bldc);
    syn_bldc_6step_set_duty(&bldc, 600);

    /* Fault state must keep gates OFF on subsequent valid Hall edges until explicit restart */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_bldc_6step_set_hall(&bldc, 0, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_STATE_FAULT, bldc.state);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_set_hall(&bldc, 5, &out));
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.u);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.v);
    TEST_ASSERT_EQUAL_INT(SYN_BLDC_GATE_OFF, out.w);
    TEST_ASSERT_EQUAL_UINT16(0U, out.duty);

    /* Boot at tick 0 + high transition count (> 71582 overflows 32-bit transitions * 60000) */
    syn_bldc_6step_init(&bldc, NULL);
    syn_bldc_6step_start(&bldc);
    TEST_ASSERT_EQUAL_UINT32(0U, syn_bldc_6step_update_speed(&bldc, 0U, 0U));
    bldc.hall_transitions = 100000U;
    uint32_t rpm = syn_bldc_6step_update_speed(&bldc, 1000U, 0U);
    /* 100000 / 24 * 60 = 250000 RPM */
    TEST_ASSERT_EQUAL_UINT32(250000U, rpm);

    /* Negative PID output must clamp duty to 0 rather than wrapping uint16_t to 1000 */
    bldc.speed_pid.cfg.out_min = -500;
    bldc.speed_pid_active = true;
    bldc.hall_transitions = 480U; /* 12000 RPM > target 100 RPM -> negative PID output */
    (void)syn_bldc_6step_update_speed(&bldc, 1100U, 100U);
    TEST_ASSERT_EQUAL_UINT16(0U, bldc.duty);

    /* current_step tracks Hall 1..6 sequence, resets on fault/stop, and huge RPM clamps to
     * INT32_MAX */
    syn_bldc_6step_init(&bldc, NULL);
    syn_bldc_6step_start(&bldc);
    const uint8_t halls[6] = {5U, 1U, 3U, 2U, 6U, 4U};
    for (uint8_t s = 0U; s < 6U; s++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_set_hall(&bldc, halls[s], &out));
        TEST_ASSERT_EQUAL_UINT8((uint8_t)(s + 1U), bldc.current_step);
    }

    bldc.speed_pid_active = true;
    bldc.speed_pid.cfg.out_min = -500;
    bldc.duty = 800U;
    (void)syn_bldc_6step_update_speed(&bldc, 0U, 1000U);
    bldc.hall_transitions = UINT32_MAX; /* > INT32_MAX RPM: must not wrap negative into PID */
    (void)syn_bldc_6step_update_speed(&bldc, 100U, 1000U);
    TEST_ASSERT_EQUAL_UINT16(0U, bldc.duty);

    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_bldc_6step_set_hall(&bldc, 7U, &out));
    TEST_ASSERT_EQUAL_UINT8(0U, bldc.current_step);
    TEST_ASSERT_EQUAL_UINT32(0U, bldc.rpm);

    syn_bldc_6step_start(&bldc);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_set_hall(&bldc, 5U, &out));
    TEST_ASSERT_EQUAL_UINT8(1U, bldc.current_step);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_bldc_6step_stop(&bldc, &out));
    TEST_ASSERT_EQUAL_UINT8(0U, bldc.current_step);
    TEST_ASSERT_EQUAL_INT64(0, bldc.speed_pid.integral);
}
