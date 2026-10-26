

# File syn\_cia402.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia402.c**](syn__cia402_8c.md)

[Go to the documentation of this file](syn__cia402_8c.md)


```C++

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_CIA402) || SYN_USE_CIA402

#include "../util/syn_assert.h"
#include "syn_cia402.h"

#include <stdlib.h>
#include <string.h>

/* ── State Transition Logic ──────────────────────────────────────────────── */

static void syn_cia402_eval_fsm(SYN_CIA402_Axis *axis)
{
    uint16_t cw = axis->controlword;
    uint16_t prev_cw = axis->prev_controlword;

    /* Check Fault Reset rising edge (Bit 7: 0 -> 1) */
    bool fault_reset_edge =
        ((cw & SYN_CIA402_CW_FAULT_RESET) != 0U) && ((prev_cw & SYN_CIA402_CW_FAULT_RESET) == 0U);

    if (axis->state == SYN_CIA402_STATE_FAULT) {
        if (fault_reset_edge) {
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

    /* Bit masks for standard commands */
    bool sw_on = (cw & SYN_CIA402_CW_SWITCH_ON) != 0U;
    bool en_volt = (cw & SYN_CIA402_CW_ENABLE_VOLTAGE) != 0U;
    bool q_stop = (cw & SYN_CIA402_CW_QUICK_STOP) != 0U; /* Active low */
    bool en_op = (cw & SYN_CIA402_CW_ENABLE_OPERATION) != 0U;

    switch (axis->state) {
    case SYN_CIA402_STATE_NOT_READY_TO_SWITCH_ON:
        axis->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
        break;

    case SYN_CIA402_STATE_SWITCH_ON_DISABLED:
        /* Transition 2: Shutdown (0x0006: Enable Voltage=1, Quick Stop=1, Switch On=0) */
        if (en_volt && q_stop && !sw_on) {
            axis->state = SYN_CIA402_STATE_READY_TO_SWITCH_ON;
        }
        /* Direct enable if 0x000F or 0x0007 passed */
        else if (en_volt && q_stop && sw_on) {
            if (en_op) {
                axis->state = SYN_CIA402_STATE_OPERATION_ENABLED;
            } else {
                axis->state = SYN_CIA402_STATE_SWITCHED_ON;
            }
        } else {
            /* Keep in SWITCH_ON_DISABLED */
        }
        break;

    case SYN_CIA402_STATE_READY_TO_SWITCH_ON:
        /* Transition 7: Disable Voltage */
        if (!en_volt || !q_stop) {
            axis->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
        }
        /* Transition 3: Switch On */
        else if (sw_on && !en_op) {
            axis->state = SYN_CIA402_STATE_SWITCHED_ON;
        }
        /* Transition 3 + 4: Direct Enable Operation */
        else if (sw_on && en_op) {
            axis->state = SYN_CIA402_STATE_OPERATION_ENABLED;
        } else {
            /* Keep in READY_TO_SWITCH_ON */
        }
        break;

    case SYN_CIA402_STATE_SWITCHED_ON:
        /* Transition 10: Disable Voltage */
        if (!en_volt || !q_stop) {
            axis->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
        }
        /* Transition 6: Shutdown */
        else if (!sw_on) {
            axis->state = SYN_CIA402_STATE_READY_TO_SWITCH_ON;
        }
        /* Transition 4: Enable Operation */
        else if (en_op) {
            axis->state = SYN_CIA402_STATE_OPERATION_ENABLED;
        } else {
            /* Keep in SWITCHED_ON */
        }
        break;

    case SYN_CIA402_STATE_OPERATION_ENABLED:
        /* Transition 9: Disable Voltage (Highest Priority) */
        if (!en_volt) {
            axis->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
        }
        /* Transition 11: Quick Stop */
        else if (!q_stop) {
            axis->state = SYN_CIA402_STATE_QUICK_STOP_ACTIVE;
        }
        /* Transition 8: Shutdown */
        else if (!sw_on) {
            axis->state = SYN_CIA402_STATE_READY_TO_SWITCH_ON;
        }
        /* Transition 5: Disable Operation */
        else if (!en_op) {
            axis->state = SYN_CIA402_STATE_SWITCHED_ON;
        } else {
            /* Keep in OPERATION_ENABLED */
        }
        break;

    case SYN_CIA402_STATE_QUICK_STOP_ACTIVE:
        /* Transition 12: Disable Voltage */
        if (!en_volt) {
            axis->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
        } else if (sw_on && en_op) {
            axis->state = SYN_CIA402_STATE_OPERATION_ENABLED;
        } else {
            /* Decelerating to 0 */
        }
        break;

    default:
        break;
    }
}

/* ── Statusword Generation ───────────────────────────────────────────────── */

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

SYN_Status syn_cia402_step(SYN_CIA402_Axis *axis, uint32_t dt_us,
                           SYN_CIA402_Setpoints *out_setpoints)
{
    if (axis == NULL) {
        return SYN_INVALID_PARAM;
    }

    float dt_s = (float)dt_us / 1000000.0f;
    if (dt_s <= 0.0f) {
        dt_s = 0.001f;
    }

    /* Evaluate FSM only if controlword changed or during fault reaction */
    if (axis->controlword != axis->prev_controlword ||
        axis->state == SYN_CIA402_STATE_FAULT_REACTION_ACTIVE) {
        syn_cia402_eval_fsm(axis);
    }

    if (axis->state == SYN_CIA402_STATE_QUICK_STOP_ACTIVE) {
        float dec = (float)((axis->quick_stop_dec > 0U) ? axis->quick_stop_dec : 10000U);
        float delta_v = dec * dt_s;

        if (axis->current_speed > 0.0f) {
            axis->current_speed -= delta_v;
            if (axis->current_speed < 0.0f) {
                axis->current_speed = 0.0f;
            }
        } else if (axis->current_speed < 0.0f) {
            axis->current_speed += delta_v;
            if (axis->current_speed > 0.0f) {
                axis->current_speed = 0.0f;
            }
        } else {
            axis->current_speed = 0.0f;
        }

        axis->actual_velocity = (int32_t)axis->current_speed;
        if (axis->actual_velocity == 0) {
            axis->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
            axis->target_reached = true;
        }
        syn_cia402_update_statusword(axis);
        axis->prev_controlword = axis->controlword;
        return SYN_OK;
    }

    if (axis->state == SYN_CIA402_STATE_OPERATION_ENABLED) {
        axis->mode_of_operation_disp = axis->mode_of_operation;

        switch (axis->mode_of_operation) {
        case SYN_CIA402_MODE_PROFILE_POSITION: {
            bool new_sp = (axis->controlword & SYN_CIA402_CW_NEW_SETPOINT) != 0U;
            if (new_sp) {
                axis->setpoint_ack = true;
                axis->target_reached = false;
            } else {
                axis->setpoint_ack = false;
            }

            int32_t pos_diff = axis->target_position - axis->actual_position;
            float max_v = (float)((axis->profile_vel > 0U) ? axis->profile_vel : 1000U);
            float step = max_v * dt_s;

            if (abs(pos_diff) <= (int32_t)step || abs(pos_diff) == 0) {
                axis->actual_position = axis->target_position;
                axis->actual_velocity = 0;
                axis->current_speed = 0.0f;
                axis->target_reached = true;
            } else if (pos_diff > 0) {
                axis->actual_position += (int32_t)step;
                axis->actual_velocity = (int32_t)max_v;
                axis->current_speed = max_v;
                axis->target_reached = false;
            } else {
                axis->actual_position -= (int32_t)step;
                axis->actual_velocity = -(int32_t)max_v;
                axis->current_speed = -max_v;
                axis->target_reached = false;
            }
            break;
        }

        case SYN_CIA402_MODE_PROFILE_VELOCITY: {
            float target_v = (float)axis->target_velocity;
            float acc = (float)((axis->profile_acc > 0U) ? axis->profile_acc : 1000U);
            float dec = (float)((axis->profile_dec > 0U) ? axis->profile_dec : 1000U);
            float delta_acc = acc * dt_s;
            float delta_dec = dec * dt_s;

            if (axis->current_speed < target_v) {
                axis->current_speed += delta_acc;
                if (axis->current_speed > target_v) {
                    axis->current_speed = target_v;
                }
            } else if (axis->current_speed > target_v) {
                axis->current_speed -= delta_dec;
                if (axis->current_speed < target_v) {
                    axis->current_speed = target_v;
                }
            }

            axis->actual_velocity = (int32_t)axis->current_speed;
            axis->actual_position += (int32_t)(axis->current_speed * dt_s);
            axis->target_reached = (axis->current_speed == target_v);
            break;
        }

        case SYN_CIA402_MODE_PROFILE_TORQUE: {
            axis->actual_torque = axis->target_torque;
            axis->target_reached = true;
            break;
        }

        case SYN_CIA402_MODE_CYCLIC_SYNC_POSITION: {
            axis->actual_position = axis->target_position;
            axis->target_reached = true;
            break;
        }

        case SYN_CIA402_MODE_CYCLIC_SYNC_VELOCITY: {
            axis->actual_velocity = axis->target_velocity;
            axis->target_reached = true;
            break;
        }

        case SYN_CIA402_MODE_CYCLIC_SYNC_TORQUE: {
            axis->actual_torque = axis->target_torque;
            axis->target_reached = true;
            break;
        }

        default:
            axis->target_reached = true;
            break;
        }
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
    uint32_t dt_us = (dt_ms > 0U) ? (dt_ms * 1000U) : 1000U;
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
```


