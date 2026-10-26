

# File syn\_bldc\_6step.c

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_bldc\_6step.c**](syn__bldc__6step_8c.md)

[Go to the documentation of this file](syn__bldc__6step_8c.md)


```C++

#include "syntropic/motor/syn_bldc_6step.h"

/*
 * 6-Step Trapezoidal Phase Commutation Table
 *
 * Standard 120-degree Hall sensor mapping:
 * Hall State | CW Step | Phase U | Phase V | Phase W
 * -----------+---------+---------+---------+--------
 *   0b101(5) | Step 1  | PWM (H) | LOW (L) | OFF (Z)
 *   0b001(1) | Step 2  | PWM (H) | OFF (Z) | LOW (L)
 *   0b011(3) | Step 3  | OFF (Z) | PWM (H) | LOW (L)
 *   0b010(2) | Step 4  | LOW (L) | PWM (H) | OFF (Z)
 *   0b110(6) | Step 5  | LOW (L) | OFF (Z) | PWM (H)
 *   0b100(4) | Step 6  | OFF (Z) | LOW (L) | PWM (H)
 */

typedef struct {
    SYN_BLDC_GateState u; 
    SYN_BLDC_GateState v; 
    SYN_BLDC_GateState w; 
} PhaseGatePattern;

static const PhaseGatePattern s_step_table_cw[7] = {
    {SYN_BLDC_GATE_OFF, SYN_BLDC_GATE_OFF, SYN_BLDC_GATE_OFF}, /* 0: Invalid */
    {SYN_BLDC_GATE_PWM, SYN_BLDC_GATE_OFF, SYN_BLDC_GATE_LOW}, /* 1: Hall 0b001 (Step 2: U+ W-) */
    {SYN_BLDC_GATE_LOW, SYN_BLDC_GATE_PWM, SYN_BLDC_GATE_OFF}, /* 2: Hall 0b010 (Step 4: V+ U-) */
    {SYN_BLDC_GATE_OFF, SYN_BLDC_GATE_PWM, SYN_BLDC_GATE_LOW}, /* 3: Hall 0b011 (Step 3: V+ W-) */
    {SYN_BLDC_GATE_OFF, SYN_BLDC_GATE_LOW, SYN_BLDC_GATE_PWM}, /* 4: Hall 0b100 (Step 6: W+ V-) */
    {SYN_BLDC_GATE_PWM, SYN_BLDC_GATE_LOW, SYN_BLDC_GATE_OFF}, /* 5: Hall 0b101 (Step 1: U+ V-) */
    {SYN_BLDC_GATE_LOW, SYN_BLDC_GATE_OFF, SYN_BLDC_GATE_PWM}, /* 6: Hall 0b110 (Step 5: W+ U-) */
};

static const PhaseGatePattern s_step_table_ccw[7] = {
    {SYN_BLDC_GATE_OFF, SYN_BLDC_GATE_OFF, SYN_BLDC_GATE_OFF}, /* 0: Invalid */
    {SYN_BLDC_GATE_LOW, SYN_BLDC_GATE_OFF, SYN_BLDC_GATE_PWM}, /* 1: Hall 0b001 */
    {SYN_BLDC_GATE_PWM, SYN_BLDC_GATE_LOW, SYN_BLDC_GATE_OFF}, /* 2: Hall 0b010 */
    {SYN_BLDC_GATE_OFF, SYN_BLDC_GATE_LOW, SYN_BLDC_GATE_PWM}, /* 3: Hall 0b011 */
    {SYN_BLDC_GATE_OFF, SYN_BLDC_GATE_PWM, SYN_BLDC_GATE_LOW}, /* 4: Hall 0b100 */
    {SYN_BLDC_GATE_LOW, SYN_BLDC_GATE_PWM, SYN_BLDC_GATE_OFF}, /* 5: Hall 0b101 */
    {SYN_BLDC_GATE_PWM, SYN_BLDC_GATE_OFF, SYN_BLDC_GATE_LOW}, /* 6: Hall 0b110 */
};

SYN_Status syn_bldc_6step_init(SYN_BLDC_6Step *bldc, const SYN_BLDC_Config *cfg)
{
    if (!bldc) {
        return SYN_INVALID_PARAM;
    }

    if (cfg) {
        bldc->config = *cfg;
    } else {
        bldc->config.pole_pairs = 4;
        bldc->config.pwm_frequency = 20000;
    }

    bldc->state = SYN_BLDC_STATE_STOPPED;
    bldc->direction = SYN_BLDC_DIR_CW;
    bldc->current_step = 0;
    bldc->hall_state = 0;
    bldc->duty = 0;
    bldc->hall_transitions = 0;
    bldc->last_hall_tick_ms = 0;
    bldc->rpm = 0;
    bldc->speed_pid_active = false;

    /* Initialize speed PID controller defaults */
    SYN_PID_Config pid_cfg = SYN_PID_GAINS(0.5f, 0.1f, 0.01f, 100, 0, 1000);
    syn_pid_init(&bldc->speed_pid, &pid_cfg);

    return SYN_OK;
}

SYN_Status syn_bldc_6step_set_hall(SYN_BLDC_6Step *bldc, uint8_t hall_state,
                                   SYN_BLDC_PhaseOutputs *out)
{
    if (!bldc) {
        return SYN_INVALID_PARAM;
    }

    uint8_t state = hall_state & 0x07U;

    /* Check for invalid Hall sensor readings (0b000 or 0b111) */
    if (state == 0U || state == 7U) {
        bldc->state = SYN_BLDC_STATE_FAULT;
        if (out) {
            out->u = SYN_BLDC_GATE_OFF;
            out->v = SYN_BLDC_GATE_OFF;
            out->w = SYN_BLDC_GATE_OFF;
            out->duty = 0;
        }
        return SYN_ERROR;
    }

    bldc->hall_state = state;
    bldc->hall_transitions++;

    if (bldc->state == SYN_BLDC_STATE_STOPPED) {
        if (out) {
            out->u = SYN_BLDC_GATE_OFF;
            out->v = SYN_BLDC_GATE_OFF;
            out->w = SYN_BLDC_GATE_OFF;
            out->duty = 0;
        }
        return SYN_OK;
    }

    const PhaseGatePattern *pattern =
        (bldc->direction == SYN_BLDC_DIR_CW) ? &s_step_table_cw[state] : &s_step_table_ccw[state];

    if (out) {
        out->u = pattern->u;
        out->v = pattern->v;
        out->w = pattern->w;
        out->duty = bldc->duty;
    }

    return SYN_OK;
}

SYN_Status syn_bldc_6step_set_duty(SYN_BLDC_6Step *bldc, uint16_t duty_0_to_1000)
{
    if (!bldc) {
        return SYN_INVALID_PARAM;
    }

    if (duty_0_to_1000 > 1000U) {
        duty_0_to_1000 = 1000U;
    }

    bldc->duty = duty_0_to_1000;
    return SYN_OK;
}

SYN_Status syn_bldc_6step_set_direction(SYN_BLDC_6Step *bldc, SYN_BLDC_Direction dir)
{
    if (!bldc) {
        return SYN_INVALID_PARAM;
    }

    bldc->direction = dir;
    return SYN_OK;
}

SYN_Status syn_bldc_6step_start(SYN_BLDC_6Step *bldc)
{
    if (!bldc) {
        return SYN_INVALID_PARAM;
    }

    bldc->state = SYN_BLDC_STATE_RUNNING;
    return SYN_OK;
}

SYN_Status syn_bldc_6step_stop(SYN_BLDC_6Step *bldc, SYN_BLDC_PhaseOutputs *out)
{
    if (!bldc) {
        return SYN_INVALID_PARAM;
    }

    bldc->state = SYN_BLDC_STATE_STOPPED;
    bldc->rpm = 0;

    if (out) {
        out->u = SYN_BLDC_GATE_OFF;
        out->v = SYN_BLDC_GATE_OFF;
        out->w = SYN_BLDC_GATE_OFF;
        out->duty = 0;
    }

    return SYN_OK;
}

uint32_t syn_bldc_6step_update_speed(SYN_BLDC_6Step *bldc, uint32_t now_ms, uint32_t target_rpm)
{
    if (!bldc) {
        return 0;
    }

    if (bldc->last_hall_tick_ms == 0) {
        bldc->last_hall_tick_ms = now_ms;
        bldc->hall_transitions = 0;
        return 0;
    }

    uint32_t dt_ms = now_ms - bldc->last_hall_tick_ms;
    if (dt_ms >= 100U) { /* Calculate speed every 100ms window */
        uint32_t transitions = bldc->hall_transitions;
        bldc->hall_transitions = 0;
        bldc->last_hall_tick_ms = now_ms;

        /*
         * RPM calculation:
         * 1 mechanical revolution = 6 * pole_pairs Hall transitions.
         * RPM = (transitions / (6 * pole_pairs)) * (60,000 ms / dt_ms)
         */
        uint32_t p = (uint32_t)bldc->config.pole_pairs;
        if (p == 0) {
            p = 4;
        }

        uint32_t mechanical_rev_denom = 6U * p;
        bldc->rpm = (transitions * 60000U) / (mechanical_rev_denom * dt_ms);

        /* Apply closed-loop speed PID if enabled */
        if (bldc->speed_pid_active && bldc->state == SYN_BLDC_STATE_RUNNING) {
            int32_t pid_out =
                syn_pid_update(&bldc->speed_pid, (int32_t)target_rpm, (int32_t)bldc->rpm, dt_ms);
            uint16_t new_duty = (uint16_t)pid_out;
            syn_bldc_6step_set_duty(bldc, new_duty);
        }
    }

    return bldc->rpm;
}

SYN_Status syn_bldc_6step_get_phase_outputs(const SYN_BLDC_6Step *bldc, SYN_BLDC_PhaseOutputs *out)
{
    if (!bldc || !out) {
        return SYN_INVALID_PARAM;
    }

    if (bldc->state != SYN_BLDC_STATE_RUNNING || bldc->hall_state == 0 || bldc->hall_state == 7) {
        out->u = SYN_BLDC_GATE_OFF;
        out->v = SYN_BLDC_GATE_OFF;
        out->w = SYN_BLDC_GATE_OFF;
        out->duty = 0;
        return SYN_OK;
    }

    const PhaseGatePattern *pattern = (bldc->direction == SYN_BLDC_DIR_CW)
                                          ? &s_step_table_cw[bldc->hall_state]
                                          : &s_step_table_ccw[bldc->hall_state];

    out->u = pattern->u;
    out->v = pattern->v;
    out->w = pattern->w;
    out->duty = bldc->duty;

    return SYN_OK;
}
```


