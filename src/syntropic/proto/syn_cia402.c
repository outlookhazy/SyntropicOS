/**
 * @file syn_cia402.c
 * @brief CANopen CiA 402 Device Profile for Drives and Motion Control implementation.
 */

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_CIA402) || SYN_USE_CIA402

#include "../util/syn_assert.h"
#include "syn_cia402.h"

#include <string.h>

#define SYN_CIA402_FXP_ONE 4294967296LL /**< Q32.32 fixed-point unity scale (2^32) */

/**
 * @brief Synchronize internal Q32.32 position accumulator with actual_position.
 * @param axis CiA 402 axis instance.
 */
static void cia402_sync_fxp(SYN_CIA402_Axis *axis)
{
    if ((int32_t)(axis->current_pos_fxp / SYN_CIA402_FXP_ONE) != axis->actual_position) {
        axis->current_pos_fxp = (int64_t)axis->actual_position * SYN_CIA402_FXP_ONE;
    }
}

/* ── State Transition Logic ──────────────────────────────────────────────── */

/**
 * @brief Handle FSA transitions out of Switch On Disabled state.
 * @param axis    CiA 402 axis instance.
 * @param sw_on   Controlword Switch On bit.
 * @param en_volt Controlword Enable Voltage bit.
 * @param q_stop  Controlword Quick Stop bit.
 * @param en_op   Controlword Enable Operation bit.
 */
static void cia402_fsm_switch_on_disabled(SYN_CIA402_Axis *axis, bool sw_on, bool en_volt,
                                          bool q_stop, bool en_op)
{
    if (!en_volt || !q_stop) {
        return;
    }
    if (!sw_on) {
        axis->state = SYN_CIA402_STATE_READY_TO_SWITCH_ON;
        return;
    }
    axis->state = en_op ? SYN_CIA402_STATE_OPERATION_ENABLED : SYN_CIA402_STATE_SWITCHED_ON;
}

/**
 * @brief Handle FSA transitions for Ready to Switch On and Switched On states.
 * @param axis    CiA 402 axis instance.
 * @param sw_on   Controlword Switch On bit.
 * @param en_volt Controlword Enable Voltage bit.
 * @param q_stop  Controlword Quick Stop bit.
 * @param en_op   Controlword Enable Operation bit.
 */
static void cia402_fsm_ready_switched(SYN_CIA402_Axis *axis, bool sw_on, bool en_volt, bool q_stop,
                                      bool en_op)
{
    if (!en_volt || !q_stop) {
        axis->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
        return;
    }
    if (axis->state == SYN_CIA402_STATE_READY_TO_SWITCH_ON) {
        if (sw_on) {
            axis->state = en_op ? SYN_CIA402_STATE_OPERATION_ENABLED : SYN_CIA402_STATE_SWITCHED_ON;
        }
        return;
    }
    if (!sw_on) {
        axis->state = SYN_CIA402_STATE_READY_TO_SWITCH_ON;
    } else if (en_op) {
        axis->state = SYN_CIA402_STATE_OPERATION_ENABLED;
    }
}

/**
 * @brief Handle FSA transitions for Operation Enabled and Quick Stop Active states.
 * @param axis    CiA 402 axis instance.
 * @param sw_on   Controlword Switch On bit.
 * @param en_volt Controlword Enable Voltage bit.
 * @param q_stop  Controlword Quick Stop bit.
 * @param en_op   Controlword Enable Operation bit.
 */
static void cia402_fsm_op_quickstop(SYN_CIA402_Axis *axis, bool sw_on, bool en_volt, bool q_stop,
                                    bool en_op)
{
    if (!en_volt) {
        axis->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
        return;
    }
    if (axis->state == SYN_CIA402_STATE_QUICK_STOP_ACTIVE) {
        if (q_stop && sw_on && en_op) {
            axis->state = SYN_CIA402_STATE_OPERATION_ENABLED;
        }
        return;
    }
    if (!q_stop) {
        axis->state = SYN_CIA402_STATE_QUICK_STOP_ACTIVE;
    } else if (!sw_on) {
        axis->state = SYN_CIA402_STATE_READY_TO_SWITCH_ON;
    } else if (!en_op) {
        axis->state = SYN_CIA402_STATE_SWITCHED_ON;
    }
}

/**
 * @brief Evaluate CiA 402 Device Control State Machine transitions.
 *
 * @param axis CiA 402 axis instance.
 */
static void syn_cia402_eval_fsm(SYN_CIA402_Axis *axis)
{
    uint16_t cw = axis->controlword;
    uint16_t prev_cw = axis->prev_controlword;

    if (axis->state == SYN_CIA402_STATE_FAULT) {
        if (((cw & SYN_CIA402_CW_FAULT_RESET) != 0U) &&
            ((prev_cw & SYN_CIA402_CW_FAULT_RESET) == 0U)) {
            axis->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
            axis->error_code = 0U;
        }
        return;
    }

    if (axis->state == SYN_CIA402_STATE_FAULT_REACTION_ACTIVE) {
        if (axis->actual_velocity == 0 && (int32_t)axis->current_speed == 0) {
            axis->state = SYN_CIA402_STATE_FAULT;
        }
        return;
    }

    bool sw_on = (cw & SYN_CIA402_CW_SWITCH_ON) != 0U;
    bool en_volt = (cw & SYN_CIA402_CW_ENABLE_VOLTAGE) != 0U;
    bool q_stop = (cw & SYN_CIA402_CW_QUICK_STOP) != 0U;
    bool en_op = (cw & SYN_CIA402_CW_ENABLE_OPERATION) != 0U;

    switch (axis->state) {
    case SYN_CIA402_STATE_NOT_READY_TO_SWITCH_ON:
        axis->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
        break;
    case SYN_CIA402_STATE_SWITCH_ON_DISABLED:
        cia402_fsm_switch_on_disabled(axis, sw_on, en_volt, q_stop, en_op);
        break;
    case SYN_CIA402_STATE_READY_TO_SWITCH_ON:
    case SYN_CIA402_STATE_SWITCHED_ON:
        cia402_fsm_ready_switched(axis, sw_on, en_volt, q_stop, en_op);
        break;
    case SYN_CIA402_STATE_OPERATION_ENABLED:
    case SYN_CIA402_STATE_QUICK_STOP_ACTIVE:
        cia402_fsm_op_quickstop(axis, sw_on, en_volt, q_stop, en_op);
        break;
    default:
        break;
    }
}

/* ── Statusword Generation ───────────────────────────────────────────────── */

/**
 * @brief Construct CANopen Statusword (0x6041) from current drive state.
 *
 * @param axis CiA 402 axis instance.
 */
static void syn_cia402_update_statusword(SYN_CIA402_Axis *axis)
{
    uint16_t sw = 0U;

    switch (axis->state) {
    case SYN_CIA402_STATE_NOT_READY_TO_SWITCH_ON:
        sw = 0x0000U;
        break;

    case SYN_CIA402_STATE_SWITCH_ON_DISABLED:
        sw = SYN_CIA402_SW_SWITCH_ON_DISABLED;
        break;

    case SYN_CIA402_STATE_READY_TO_SWITCH_ON:
        sw = SYN_CIA402_SW_READY_TO_SWITCH_ON | SYN_CIA402_SW_QUICK_STOP |
             SYN_CIA402_SW_VOLTAGE_ENABLED;
        break;

    case SYN_CIA402_STATE_SWITCHED_ON:
        sw = SYN_CIA402_SW_READY_TO_SWITCH_ON | SYN_CIA402_SW_SWITCHED_ON |
             SYN_CIA402_SW_QUICK_STOP | SYN_CIA402_SW_VOLTAGE_ENABLED;
        break;

    case SYN_CIA402_STATE_OPERATION_ENABLED:
        sw = SYN_CIA402_SW_READY_TO_SWITCH_ON | SYN_CIA402_SW_SWITCHED_ON |
             SYN_CIA402_SW_OPERATION_ENABLED | SYN_CIA402_SW_QUICK_STOP |
             SYN_CIA402_SW_VOLTAGE_ENABLED;
        break;

    case SYN_CIA402_STATE_QUICK_STOP_ACTIVE:
        sw = SYN_CIA402_SW_READY_TO_SWITCH_ON | SYN_CIA402_SW_SWITCHED_ON |
             SYN_CIA402_SW_OPERATION_ENABLED | SYN_CIA402_SW_VOLTAGE_ENABLED;
        break;

    case SYN_CIA402_STATE_FAULT_REACTION_ACTIVE:
        sw = SYN_CIA402_SW_READY_TO_SWITCH_ON | SYN_CIA402_SW_SWITCHED_ON |
             SYN_CIA402_SW_OPERATION_ENABLED | SYN_CIA402_SW_FAULT | SYN_CIA402_SW_VOLTAGE_ENABLED;
        break;

    case SYN_CIA402_STATE_FAULT:
        sw = SYN_CIA402_SW_FAULT;
        break;

    default:
        sw = 0x0000U;
        break;
    }

    if (axis->target_reached && axis->state != SYN_CIA402_STATE_FAULT &&
        axis->state != SYN_CIA402_STATE_NOT_READY_TO_SWITCH_ON &&
        axis->state <= SYN_CIA402_STATE_FAULT) {
        sw |= SYN_CIA402_SW_TARGET_REACHED;
    }

    if (axis->setpoint_ack) {
        sw |= SYN_CIA402_SW_SETPOINT_ACK;
    }

    axis->statusword = sw;
}

/* ── Public API Implementation ──────────────────────────────────────────── */

SYN_Status syn_cia402_init(SYN_CIA402_Axis *axis, const SYN_CIA402_Config *cfg)
{
    if (axis == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(axis, 0, sizeof(*axis));
    axis->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
    axis->mode_of_operation = (int8_t)SYN_CIA402_MODE_NO_MODE;
    axis->mode_of_operation_disp = (int8_t)SYN_CIA402_MODE_NO_MODE;
    axis->target_reached = true;
    axis->quick_stop_option = 2;

    axis->sw_limits_enabled = cfg->sw_limits_enabled;
    axis->sw_limit_min = cfg->sw_limit_min;
    axis->sw_limit_max = cfg->sw_limit_max;
    axis->profile_vel = (cfg->max_profile_velocity > 0U) ? cfg->max_profile_velocity : 1000U;
    axis->profile_acc = (cfg->profile_acceleration > 0U) ? cfg->profile_acceleration : 5000U;
    axis->profile_dec = (cfg->profile_deceleration > 0U) ? cfg->profile_deceleration : 5000U;
    axis->quick_stop_dec =
        (cfg->quick_stop_deceleration > 0U) ? cfg->quick_stop_deceleration : 10000U;
    axis->following_err_win = cfg->following_err_window;
    axis->position_window = (cfg->position_window > 0U) ? cfg->position_window : 5U;
    axis->max_torque = (cfg->max_torque > 0U) ? cfg->max_torque : 1000U;
    axis->torque_slope = (cfg->torque_slope > 0U) ? cfg->torque_slope : 10000U;

    syn_cia402_update_statusword(axis);
    return SYN_OK;
}

SYN_Status syn_cia402_set_controlword(SYN_CIA402_Axis *axis, uint16_t cw)
{
    if (axis == NULL) {
        return SYN_INVALID_PARAM;
    }
    axis->controlword = cw;
    syn_cia402_eval_fsm(axis);
    syn_cia402_update_statusword(axis);
    axis->prev_controlword = cw;
    return SYN_OK;
}

uint16_t syn_cia402_get_controlword(const SYN_CIA402_Axis *axis)
{
    if (axis == NULL) {
        return 0U;
    }
    return axis->controlword;
}

uint16_t syn_cia402_get_statusword(const SYN_CIA402_Axis *axis)
{
    if (axis == NULL) {
        return 0U;
    }
    SYN_CIA402_Axis *mut_axis = (SYN_CIA402_Axis *)axis;
    syn_cia402_update_statusword(mut_axis);
    return axis->statusword;
}

SYN_CIA402_State syn_cia402_get_state(const SYN_CIA402_Axis *axis)
{
    if (axis == NULL) {
        return SYN_CIA402_STATE_FAULT;
    }
    return axis->state;
}

SYN_Status syn_cia402_set_mode(SYN_CIA402_Axis *axis, int8_t mode)
{
    if (axis == NULL) {
        return SYN_INVALID_PARAM;
    }
    axis->mode_of_operation = mode;
    axis->mode_of_operation_disp = mode;
    return SYN_OK;
}

int8_t syn_cia402_get_mode(const SYN_CIA402_Axis *axis)
{
    if (axis == NULL) {
        return 0;
    }
    return axis->mode_of_operation;
}

int8_t syn_cia402_get_mode_display(const SYN_CIA402_Axis *axis)
{
    if (axis == NULL) {
        return 0;
    }
    return axis->mode_of_operation_disp;
}

SYN_Status syn_cia402_set_target_position(SYN_CIA402_Axis *axis, int32_t pos)
{
    if (axis == NULL) {
        return SYN_INVALID_PARAM;
    }
    axis->target_position = pos;
    return SYN_OK;
}

SYN_Status syn_cia402_set_target_velocity(SYN_CIA402_Axis *axis, int32_t vel)
{
    if (axis == NULL) {
        return SYN_INVALID_PARAM;
    }
    axis->target_velocity = vel;
    return SYN_OK;
}

SYN_Status syn_cia402_set_target_torque(SYN_CIA402_Axis *axis, int16_t torque)
{
    if (axis == NULL) {
        return SYN_INVALID_PARAM;
    }
    axis->target_torque = torque;
    return SYN_OK;
}

SYN_Status syn_cia402_feed_feedback(SYN_CIA402_Axis *axis, int32_t pos_actual, int32_t vel_actual,
                                    int16_t torque_actual)
{
    if (axis == NULL) {
        return SYN_INVALID_PARAM;
    }
    axis->actual_position = pos_actual;
    axis->current_pos_fxp = (int64_t)pos_actual * SYN_CIA402_FXP_ONE;
    axis->actual_velocity = vel_actual;
    axis->actual_torque = torque_actual;
    axis->current_speed = (float)vel_actual;
    return SYN_OK;
}

SYN_Status syn_cia402_report_actuals(SYN_CIA402_Axis *axis, int32_t pos_actual, int32_t vel_actual,
                                     int16_t torque_actual)
{
    return syn_cia402_feed_feedback(axis, pos_actual, vel_actual, torque_actual);
}

/**
 * @brief Decelerate velocity to zero during Quick Stop Active state.
 * @param axis CiA 402 axis instance.
 * @param dt_s Step interval in seconds.
 */
static void cia402_step_quick_stop(SYN_CIA402_Axis *axis, float dt_s)
{
    float dec = (float)((axis->quick_stop_dec > 0U) ? axis->quick_stop_dec : 10000U);
    float delta_v = dec * dt_s;

    if (axis->current_speed > 0.0f) {
        axis->current_speed =
            (axis->current_speed > delta_v) ? (axis->current_speed - delta_v) : 0.0f;
    } else if (axis->current_speed < 0.0f) {
        axis->current_speed =
            (-axis->current_speed > delta_v) ? (axis->current_speed + delta_v) : 0.0f;
    }

    axis->actual_velocity = (int32_t)axis->current_speed;
    if (axis->actual_velocity == 0) {
        axis->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
        axis->target_reached = true;
    }
}

/**
 * @brief Execute one trajectory step in Profile Position (PP) mode.
 * @param axis      CiA 402 axis instance.
 * @param eff_dt_us Effective step interval in microseconds.
 */
static void cia402_step_pp(SYN_CIA402_Axis *axis, uint32_t eff_dt_us)
{
    cia402_sync_fxp(axis);
    if ((axis->controlword & SYN_CIA402_CW_NEW_SETPOINT) != 0U) {
        axis->setpoint_ack = true;
        axis->target_reached = false;
    } else {
        axis->setpoint_ack = false;
    }

    uint64_t max_v = (axis->profile_vel > 0U) ? (uint64_t)axis->profile_vel : 1000ULL;
    uint64_t prod = max_v * (uint64_t)eff_dt_us;
    uint64_t step_fxp = ((prod / 1000000ULL) * (uint64_t)SYN_CIA402_FXP_ONE) +
                        (((prod % 1000000ULL) * (uint64_t)SYN_CIA402_FXP_ONE) / 1000000ULL);
    int64_t target_fxp = (int64_t)axis->target_position * SYN_CIA402_FXP_ONE;
    uint64_t abs_diff = (target_fxp >= axis->current_pos_fxp)
                            ? ((uint64_t)target_fxp - (uint64_t)axis->current_pos_fxp)
                            : ((uint64_t)axis->current_pos_fxp - (uint64_t)target_fxp);

    if (abs_diff <= step_fxp) {
        axis->current_pos_fxp = target_fxp;
        axis->actual_position = axis->target_position;
        axis->actual_velocity = 0;
        axis->current_speed = 0.0f;
        axis->target_reached = true;
    } else if (target_fxp > axis->current_pos_fxp) {
        axis->current_pos_fxp = (int64_t)((uint64_t)axis->current_pos_fxp + step_fxp);
        axis->actual_position = (int32_t)(axis->current_pos_fxp / SYN_CIA402_FXP_ONE);
        axis->actual_velocity = (int32_t)max_v;
        axis->current_speed = (float)max_v;
        axis->target_reached = false;
    } else {
        axis->current_pos_fxp = (int64_t)((uint64_t)axis->current_pos_fxp - step_fxp);
        axis->actual_position = (int32_t)(axis->current_pos_fxp / SYN_CIA402_FXP_ONE);
        axis->actual_velocity = -(int32_t)max_v;
        axis->current_speed = -(float)max_v;
        axis->target_reached = false;
    }
}

/**
 * @brief Execute one velocity ramp step in Profile Velocity (PV) mode.
 * @param axis CiA 402 axis instance.
 * @param dt_s Step interval in seconds.
 */
static void cia402_step_pv(SYN_CIA402_Axis *axis, float dt_s)
{
    cia402_sync_fxp(axis);
    float target_v = (float)axis->target_velocity;
    float acc = (float)((axis->profile_acc > 0U) ? axis->profile_acc : 1000U);
    float dec = (float)((axis->profile_dec > 0U) ? axis->profile_dec : 1000U);

    if (axis->current_speed < target_v) {
        axis->current_speed += acc * dt_s;
        if (axis->current_speed > target_v) {
            axis->current_speed = target_v;
        }
    } else if (axis->current_speed > target_v) {
        axis->current_speed -= dec * dt_s;
        if (axis->current_speed < target_v) {
            axis->current_speed = target_v;
        }
    }

    axis->actual_velocity = (int32_t)axis->current_speed;
    int64_t delta_fxp = (int64_t)(axis->current_speed * dt_s * (float)SYN_CIA402_FXP_ONE);
    axis->current_pos_fxp = (int64_t)((uint64_t)axis->current_pos_fxp + (uint64_t)delta_fxp);
    axis->actual_position = (int32_t)(axis->current_pos_fxp / SYN_CIA402_FXP_ONE);
    axis->target_reached = (axis->current_speed == target_v);
}

/**
 * @brief Dispatch active motion mode update while in Operation Enabled state.
 * @param axis      CiA 402 axis instance.
 * @param eff_dt_us Effective step interval in microseconds.
 * @param dt_s      Step interval in seconds.
 */
static void cia402_step_op_enabled(SYN_CIA402_Axis *axis, uint32_t eff_dt_us, float dt_s)
{
    axis->mode_of_operation_disp = axis->mode_of_operation;

    switch (axis->mode_of_operation) {
    case SYN_CIA402_MODE_PROFILE_POSITION:
        cia402_step_pp(axis, eff_dt_us);
        break;
    case SYN_CIA402_MODE_PROFILE_VELOCITY:
        cia402_step_pv(axis, dt_s);
        break;
    case SYN_CIA402_MODE_PROFILE_TORQUE:
    case SYN_CIA402_MODE_CYCLIC_SYNC_TORQUE:
        axis->actual_torque = axis->target_torque;
        axis->target_reached = true;
        break;
    case SYN_CIA402_MODE_CYCLIC_SYNC_POSITION:
        axis->actual_position = axis->target_position;
        axis->current_pos_fxp = (int64_t)axis->target_position * SYN_CIA402_FXP_ONE;
        axis->target_reached = true;
        break;
    case SYN_CIA402_MODE_CYCLIC_SYNC_VELOCITY:
        axis->actual_velocity = axis->target_velocity;
        axis->target_reached = true;
        break;
    default:
        axis->target_reached = true;
        break;
    }
}

SYN_Status syn_cia402_step(SYN_CIA402_Axis *axis, uint32_t dt_us,
                           SYN_CIA402_Setpoints *out_setpoints)
{
    if (axis == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint32_t eff_dt_us = (dt_us > 0U) ? dt_us : 1000U;
    float dt_s = (float)eff_dt_us / 1000000.0f;

    /* Evaluate FSM only if controlword changed or during fault reaction */
    if (axis->controlword != axis->prev_controlword ||
        axis->state == SYN_CIA402_STATE_FAULT_REACTION_ACTIVE) {
        syn_cia402_eval_fsm(axis);
    }

    if (axis->state == SYN_CIA402_STATE_QUICK_STOP_ACTIVE) {
        cia402_step_quick_stop(axis, dt_s);
        syn_cia402_update_statusword(axis);
        axis->prev_controlword = axis->controlword;
        return SYN_OK;
    }

    if (axis->state == SYN_CIA402_STATE_OPERATION_ENABLED) {
        cia402_step_op_enabled(axis, eff_dt_us, dt_s);
    }

    syn_cia402_update_statusword(axis);
    axis->prev_controlword = axis->controlword;

    if (out_setpoints != NULL) {
        out_setpoints->power_enabled = (axis->state == SYN_CIA402_STATE_OPERATION_ENABLED);
        out_setpoints->brake_released = out_setpoints->power_enabled;
        out_setpoints->mode = (SYN_CIA402_Mode)axis->mode_of_operation_disp;
        out_setpoints->position_cmd = axis->actual_position;
        out_setpoints->velocity_cmd = axis->actual_velocity;
        out_setpoints->torque_cmd = axis->actual_torque;
    }

    return SYN_OK;
}

SYN_Status syn_cia402_update(SYN_CIA402_Axis *axis, uint32_t dt_ms)
{
    if (axis == NULL) {
        return SYN_INVALID_PARAM;
    }
    uint64_t raw_us = (dt_ms > 0U) ? ((uint64_t)dt_ms * 1000ULL) : 1000ULL;
    uint32_t dt_us = (raw_us > (uint64_t)UINT32_MAX) ? UINT32_MAX : (uint32_t)raw_us;
    return syn_cia402_step(axis, dt_us, NULL);
}

SYN_Status syn_cia402_trigger_fault(SYN_CIA402_Axis *axis, uint16_t error_code)
{
    if (axis == NULL) {
        return SYN_INVALID_PARAM;
    }
    axis->error_code = error_code;
    axis->state = SYN_CIA402_STATE_FAULT;
    syn_cia402_update_statusword(axis);
    return SYN_OK;
}

size_t syn_cia402_populate_od(SYN_CANOpenODEntry *od_table, size_t max_entries,
                              SYN_CIA402_Axis *axis)
{
    if (od_table == NULL || axis == NULL || max_entries < 10U) {
        return 0U;
    }

    size_t count = 0;

    od_table[count++] = (SYN_CANOpenODEntry){SYN_CIA402_OD_CONTROLWORD, 0x00U,
                                             SYN_CANOPEN_TYPE_U16,      SYN_CANOPEN_ACCESS_RW,
                                             &axis->controlword,        sizeof(axis->controlword)};
    od_table[count++] = (SYN_CANOpenODEntry){SYN_CIA402_OD_STATUSWORD, 0x00U,
                                             SYN_CANOPEN_TYPE_U16,     SYN_CANOPEN_ACCESS_RO,
                                             &axis->statusword,        sizeof(axis->statusword)};
    od_table[count++] = (SYN_CANOpenODEntry){SYN_CIA402_OD_MODES_OF_OPERATION,
                                             0x00U,
                                             SYN_CANOPEN_TYPE_I8,
                                             SYN_CANOPEN_ACCESS_RW,
                                             &axis->mode_of_operation,
                                             sizeof(axis->mode_of_operation)};
    od_table[count++] = (SYN_CANOpenODEntry){SYN_CIA402_OD_MODES_OF_OPERATION_DISPLAY,
                                             0x00U,
                                             SYN_CANOPEN_TYPE_I8,
                                             SYN_CANOPEN_ACCESS_RO,
                                             &axis->mode_of_operation_disp,
                                             sizeof(axis->mode_of_operation_disp)};
    od_table[count++] = (SYN_CANOpenODEntry){SYN_CIA402_OD_POSITION_ACTUAL_VALUE,
                                             0x00U,
                                             SYN_CANOPEN_TYPE_I32,
                                             SYN_CANOPEN_ACCESS_RO,
                                             &axis->actual_position,
                                             sizeof(axis->actual_position)};
    od_table[count++] = (SYN_CANOpenODEntry){SYN_CIA402_OD_VELOCITY_ACTUAL_VALUE,
                                             0x00U,
                                             SYN_CANOPEN_TYPE_I32,
                                             SYN_CANOPEN_ACCESS_RO,
                                             &axis->actual_velocity,
                                             sizeof(axis->actual_velocity)};
    od_table[count++] =
        (SYN_CANOpenODEntry){SYN_CIA402_OD_TARGET_TORQUE, 0x00U,
                             SYN_CANOPEN_TYPE_I16,        SYN_CANOPEN_ACCESS_RW,
                             &axis->target_torque,        sizeof(axis->target_torque)};
    od_table[count++] = (SYN_CANOpenODEntry){SYN_CIA402_OD_TORQUE_ACTUAL_VALUE,
                                             0x00U,
                                             SYN_CANOPEN_TYPE_I16,
                                             SYN_CANOPEN_ACCESS_RO,
                                             &axis->actual_torque,
                                             sizeof(axis->actual_torque)};
    od_table[count++] =
        (SYN_CANOpenODEntry){SYN_CIA402_OD_TARGET_POSITION, 0x00U,
                             SYN_CANOPEN_TYPE_I32,          SYN_CANOPEN_ACCESS_RW,
                             &axis->target_position,        sizeof(axis->target_position)};
    od_table[count++] =
        (SYN_CANOpenODEntry){SYN_CIA402_OD_TARGET_VELOCITY, 0x00U,
                             SYN_CANOPEN_TYPE_I32,          SYN_CANOPEN_ACCESS_RW,
                             &axis->target_velocity,        sizeof(axis->target_velocity)};

    return count;
}

#else
typedef int syn_cia402_dummy_t;
#endif /* SYN_USE_CIA402 */
