#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_ACTUATOR) || SYN_USE_ACTUATOR

/**
 * @file syn_actuator.c
 * @brief Linear actuator implementation.
 */

#include "syn_actuator.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Convert position percentage (0.0–1000 = 0%–100%) to ADC units.
 * @param act  Actuator instance.
 * @param pct  Position in 0.1% units (0–1000).
 * @return ADC value.
 */
static int32_t pct_to_adc(const SYN_Actuator *act, int16_t pct)
{
    return act->stroke_min +
           ((int32_t)pct * act->stroke_range) / 1000;
}

/**
 * @brief Convert ADC value to position percentage.
 * @param act  Actuator instance.
 * @param adc  ADC reading.
 * @return Position in 0.1% units (0–1000).
 */
static int16_t adc_to_pct(const SYN_Actuator *act, int32_t adc)
{
    if (act->stroke_range == 0) return 0;

    int32_t pct = ((adc - act->stroke_min) * 1000) / act->stroke_range;
    if (pct < 0)    pct = 0;
    if (pct > 1000) pct = 1000;
    return (int16_t)pct;
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_actuator_init(SYN_Actuator *act, const SYN_Actuator_Config *cfg)
{
    SYN_ASSERT(act != NULL);
    SYN_ASSERT(cfg != NULL);
    SYN_ASSERT(cfg->dc_motor != NULL);
    SYN_ASSERT(cfg->read_pos != NULL);

    memset(act, 0, sizeof(*act));
    act->stroke_min   = cfg->stroke_min;
    act->stroke_max   = cfg->stroke_max;
    act->stroke_range = cfg->stroke_max - cfg->stroke_min;

    /* Configure underlying motor controller */
    SYN_MotorCtrl_Config mc;
    memset(&mc, 0, sizeof(mc));
    mc.type              = SYN_MCTRL_DC;
    mc.dc_motor          = cfg->dc_motor;
    mc.read_pos          = cfg->read_pos;
    mc.read_pos_ctx      = cfg->read_ctx;
    mc.update_hz         = cfg->update_hz;
    mc.pid_kp            = cfg->pid_kp > 0 ? cfg->pid_kp : 100;
    mc.pid_ki            = cfg->pid_ki;
    mc.pid_kd            = cfg->pid_kd;
    mc.pid_scale         = cfg->pid_scale;
    mc.position_deadband = cfg->deadband;
    mc.output_min        = -255;
    mc.output_max        = 255;
    mc.position_min      = cfg->stroke_min;
    mc.position_max      = cfg->stroke_max;
    mc.stall_timeout_ms  = cfg->stall_timeout_ms;
    mc.stall_threshold   = cfg->stall_threshold;
    mc.errlog            = cfg->errlog;

    syn_motor_ctrl_init(&act->ctrl, &mc);

    /* Read initial position */
    int32_t pos = cfg->read_pos(cfg->read_ctx);
    act->current_pct = adc_to_pct(act, pos);
    act->target_pct  = act->current_pct;
}

void syn_actuator_set_position(SYN_Actuator *act, int16_t pct_x10)
{
    SYN_ASSERT(act != NULL);

    if (pct_x10 < 0)    pct_x10 = 0;
    if (pct_x10 > 1000) pct_x10 = 1000;

    act->target_pct = pct_x10;

    int32_t target_adc = pct_to_adc(act, pct_x10);
    syn_motor_ctrl_set_position(&act->ctrl, target_adc);
}

int16_t syn_actuator_update(SYN_Actuator *act)
{
    SYN_ASSERT(act != NULL);

    syn_motor_ctrl_update(&act->ctrl);
    act->current_pct = adc_to_pct(act, act->ctrl.measured_position);

    return act->current_pct;
}

void syn_actuator_stop(SYN_Actuator *act)
{
    SYN_ASSERT(act != NULL);
    syn_motor_ctrl_stop(&act->ctrl);
    act->target_pct = act->current_pct;
}

void syn_actuator_clear_stall(SYN_Actuator *act)
{
    SYN_ASSERT(act != NULL);
    syn_motor_ctrl_clear_stall(&act->ctrl);
}

#endif /* SYN_USE_ACTUATOR */
