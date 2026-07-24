

# File syn\_cia402.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia402.c**](syn__cia402_8c.md)

[Go to the documentation of this file](syn__cia402_8c.md)


```C++

#include "syntropic/proto/syn_cia402.h"
#include <string.h>
#include <math.h>

SYN_Status syn_cia402_init(SYN_CiA402Drive *drive, const SYN_CiA402Config *cfg)
{
    if (drive == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(drive, 0, sizeof(*drive));
    drive->cfg = *cfg;
    drive->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
    drive->mode_of_operation = (int8_t)SYN_CIA402_MODE_NO_MODE;
    drive->mode_display = (int8_t)SYN_CIA402_MODE_NO_MODE;
    drive->target_reached = true;

    /* Initialize internal S-Curve trajectory generator defaults */
    int32_t v_max = (cfg->max_profile_velocity > 0) ? cfg->max_profile_velocity : 1000;
    int32_t a_max = (cfg->profile_acceleration > 0) ? cfg->profile_acceleration : 5000;
    int32_t j_max = a_max * 10;
    syn_scurve_init(&drive->scurve, 0);
    syn_scurve_set_constraints(&drive->scurve, v_max, a_max, j_max);

    return SYN_OK;
}

uint16_t syn_cia402_get_statusword(const SYN_CiA402Drive *drive)
{
    if (drive == NULL) {
        return 0U;
    }

    uint16_t sw = 0U;

    switch (drive->state) {
    case SYN_CIA402_STATE_NOT_READY:
        sw = 0U;
        break;

    case SYN_CIA402_STATE_SWITCH_ON_DISABLED:
        sw = SYN_CIA402_SW_SWITCH_ON_DISABLED;
        break;

    case SYN_CIA402_STATE_READY_TO_SWITCH_ON:
        sw = SYN_CIA402_SW_READY_TO_SWITCH_ON | SYN_CIA402_SW_QUICK_STOP;
        break;

    case SYN_CIA402_STATE_SWITCHED_ON:
        sw = SYN_CIA402_SW_READY_TO_SWITCH_ON | SYN_CIA402_SW_SWITCHED_ON |
             SYN_CIA402_SW_VOLTAGE_ENABLED | SYN_CIA402_SW_QUICK_STOP;
        break;

    case SYN_CIA402_STATE_OPERATION_ENABLED:
        sw = SYN_CIA402_SW_READY_TO_SWITCH_ON | SYN_CIA402_SW_SWITCHED_ON |
             SYN_CIA402_SW_OPERATION_ENABLED | SYN_CIA402_SW_VOLTAGE_ENABLED |
             SYN_CIA402_SW_QUICK_STOP;
        break;

    case SYN_CIA402_STATE_QUICK_STOP_ACTIVE:
        sw = SYN_CIA402_SW_READY_TO_SWITCH_ON | SYN_CIA402_SW_SWITCHED_ON |
             SYN_CIA402_SW_OPERATION_ENABLED | SYN_CIA402_SW_VOLTAGE_ENABLED;
        break;

    case SYN_CIA402_STATE_FAULT_REACTION:
    case SYN_CIA402_STATE_FAULT:
        sw = SYN_CIA402_SW_FAULT;
        break;

    default:
        break;
    }

    if (drive->target_reached) {
        sw |= SYN_CIA402_SW_TARGET_REACHED;
    }

    if (drive->setpoint_ack) {
        sw |= SYN_CIA402_SW_SETPOINT_ACK;
    }

    return sw;
}

SYN_Status syn_cia402_set_controlword(SYN_CiA402Drive *drive, uint16_t controlword)
{
    if (drive == NULL) {
        return SYN_INVALID_PARAM;
    }

    drive->controlword = controlword;

    /* Rising edge on Fault Reset bit (Bit 7) */
    if ((controlword & SYN_CIA402_CW_FAULT_RESET) != 0U &&
        (drive->prev_controlword & SYN_CIA402_CW_FAULT_RESET) == 0U) {
        if (drive->state == SYN_CIA402_STATE_FAULT) {
            drive->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
            drive->error_code = 0U;
        }
    }

    drive->prev_controlword = controlword;

    /* Evaluate state machine command bit combinations if not in Fault state */
    if (drive->state != SYN_CIA402_STATE_FAULT && drive->state != SYN_CIA402_STATE_FAULT_REACTION) {
        bool switch_on = (controlword & SYN_CIA402_CW_SWITCH_ON) != 0U;
        bool enable_volt = (controlword & SYN_CIA402_CW_ENABLE_VOLTAGE) != 0U;
        bool quick_stop = (controlword & SYN_CIA402_CW_QUICK_STOP) != 0U;
        bool enable_op = (controlword & SYN_CIA402_CW_ENABLE_OP) != 0U;

        if (!enable_volt) {
            drive->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
        } else if (!quick_stop) {
            if (drive->state == SYN_CIA402_STATE_OPERATION_ENABLED) {
                drive->state = SYN_CIA402_STATE_QUICK_STOP_ACTIVE;
            }
        } else if (!switch_on && enable_volt && quick_stop) {
            drive->state = SYN_CIA402_STATE_READY_TO_SWITCH_ON;
        } else if (switch_on && enable_volt && quick_stop && !enable_op) {
            drive->state = SYN_CIA402_STATE_SWITCHED_ON;
        } else if (switch_on && enable_volt && quick_stop && enable_op) {
            drive->state = SYN_CIA402_STATE_OPERATION_ENABLED;
        }
    }

    drive->statusword = syn_cia402_get_statusword(drive);

    return SYN_OK;
}

SYN_Status syn_cia402_set_mode(SYN_CiA402Drive *drive, int8_t mode)
{
    if (drive == NULL) {
        return SYN_INVALID_PARAM;
    }

    drive->mode_of_operation = mode;
    drive->mode_display = mode;

    return SYN_OK;
}

SYN_Status syn_cia402_set_target_position(SYN_CiA402Drive *drive, int32_t target_pos)
{
    if (drive == NULL) {
        return SYN_INVALID_PARAM;
    }

    drive->target_position = target_pos;

    return SYN_OK;
}

SYN_Status syn_cia402_set_target_velocity(SYN_CiA402Drive *drive, int32_t target_vel)
{
    if (drive == NULL) {
        return SYN_INVALID_PARAM;
    }

    drive->target_velocity = target_vel;

    return SYN_OK;
}

SYN_Status syn_cia402_set_target_torque(SYN_CiA402Drive *drive, int16_t target_torque)
{
    if (drive == NULL) {
        return SYN_INVALID_PARAM;
    }

    drive->target_torque = target_torque;

    return SYN_OK;
}

SYN_Status syn_cia402_report_actuals(SYN_CiA402Drive *drive, int32_t actual_pos, int32_t actual_vel, int16_t actual_torque)
{
    if (drive == NULL) {
        return SYN_INVALID_PARAM;
    }

    drive->actual_position = actual_pos;
    drive->actual_velocity = actual_vel;
    drive->actual_torque = actual_torque;

    return SYN_OK;
}

SYN_Status syn_cia402_trigger_fault(SYN_CiA402Drive *drive, uint16_t error_code)
{
    if (drive == NULL) {
        return SYN_INVALID_PARAM;
    }

    drive->error_code = error_code;
    drive->state = SYN_CIA402_STATE_FAULT;
    drive->statusword = syn_cia402_get_statusword(drive);

    return SYN_OK;
}

SYN_Status syn_cia402_update(SYN_CiA402Drive *drive, uint32_t dt_ms)
{
    if (drive == NULL) {
        return SYN_INVALID_PARAM;
    }

    float dt = (float)dt_ms / 1000.0f;
    if (dt <= 0.000001f) {
        dt = 0.001f;
    }

    if (drive->state == SYN_CIA402_STATE_OPERATION_ENABLED) {
        switch (drive->mode_of_operation) {
        case SYN_CIA402_MODE_PP: {
            bool new_setpoint = (drive->controlword & SYN_CIA402_CW_NEW_SETPOINT) != 0U;
            if (new_setpoint && !drive->setpoint_ack) {
                syn_scurve_set_target(&drive->scurve, drive->target_position);
                drive->setpoint_ack = true;
                drive->target_reached = false;
            } else if (!new_setpoint && drive->setpoint_ack) {
                drive->setpoint_ack = false;
            }

            if (!syn_scurve_done(&drive->scurve)) {
                drive->actual_position = syn_scurve_update(&drive->scurve);
                drive->actual_velocity = syn_scurve_velocity(&drive->scurve);
                drive->current_pos = (float)drive->actual_position;
                drive->current_speed = (float)drive->actual_velocity;
            } else {
                drive->target_reached = true;
                drive->actual_position = drive->target_position;
                drive->actual_velocity = 0;
            }
            break;
        }

        case SYN_CIA402_MODE_PV: {
            float target_v = (float)drive->target_velocity;
            float accel = (drive->cfg.profile_acceleration > 0) ? (float)drive->cfg.profile_acceleration : 5000.0f;

            if (drive->current_speed < target_v) {
                drive->current_speed += accel * dt;
                if (drive->current_speed > target_v) {
                    drive->current_speed = target_v;
                }
            } else if (drive->current_speed > target_v) {
                drive->current_speed -= accel * dt;
                if (drive->current_speed < target_v) {
                    drive->current_speed = target_v;
                }
            }

            drive->current_pos += drive->current_speed * dt;
            drive->actual_velocity = (int32_t)drive->current_speed;
            drive->actual_position = (int32_t)drive->current_pos;
            drive->target_reached = (fabsf(drive->current_speed - target_v) < 0.1f);
            break;
        }

        case SYN_CIA402_MODE_PT:
            drive->actual_torque = drive->target_torque;
            drive->target_reached = true;
            break;

        case SYN_CIA402_MODE_CSP:
            drive->actual_position = drive->target_position;
            drive->current_pos = (float)drive->target_position;
            drive->target_reached = true;
            break;

        case SYN_CIA402_MODE_CSV:
            drive->actual_velocity = drive->target_velocity;
            drive->current_speed = (float)drive->target_velocity;
            drive->current_pos += drive->current_speed * dt;
            drive->actual_position = (int32_t)drive->current_pos;
            drive->target_reached = true;
            break;

        default:
            break;
        }
    } else if (drive->state == SYN_CIA402_STATE_QUICK_STOP_ACTIVE) {
        float q_decel = (drive->cfg.quick_stop_deceleration > 0) ? (float)drive->cfg.quick_stop_deceleration : 10000.0f;
        if (drive->current_speed > 0.0f) {
            drive->current_speed -= q_decel * dt;
            if (drive->current_speed < 0.0f) {
                drive->current_speed = 0.0f;
            }
        } else if (drive->current_speed < 0.0f) {
            drive->current_speed += q_decel * dt;
            if (drive->current_speed > 0.0f) {
                drive->current_speed = 0.0f;
            }
        }

        drive->actual_velocity = (int32_t)drive->current_speed;
        if (fabsf(drive->current_speed) < 0.001f) {
            drive->state = SYN_CIA402_STATE_SWITCH_ON_DISABLED;
        }
    }

    drive->statusword = syn_cia402_get_statusword(drive);

    return SYN_OK;
}

size_t syn_cia402_populate_od(SYN_CANOpenODEntry *entries, size_t max_entries, SYN_CiA402Drive *drive)
{
    if (entries == NULL || drive == NULL || max_entries < 10U) {
        return 0U;
    }

    size_t count = 0U;

    /* 0x6040: Controlword */
    entries[count].index = 0x6040U;
    entries[count].subindex = 0x00U;
    entries[count].type = SYN_CANOPEN_TYPE_U16;
    entries[count].access = SYN_CANOPEN_ACCESS_RW;
    entries[count].data_ptr = &drive->controlword;
    entries[count].size = sizeof(drive->controlword);
    count++;

    /* 0x6041: Statusword */
    entries[count].index = 0x6041U;
    entries[count].subindex = 0x00U;
    entries[count].type = SYN_CANOPEN_TYPE_U16;
    entries[count].access = SYN_CANOPEN_ACCESS_RO;
    entries[count].data_ptr = &drive->statusword;
    entries[count].size = sizeof(drive->statusword);
    count++;

    /* 0x6060: Modes of Operation */
    entries[count].index = 0x6060U;
    entries[count].subindex = 0x00U;
    entries[count].type = SYN_CANOPEN_TYPE_I8;
    entries[count].access = SYN_CANOPEN_ACCESS_RW;
    entries[count].data_ptr = &drive->mode_of_operation;
    entries[count].size = sizeof(drive->mode_of_operation);
    count++;

    /* 0x6061: Modes of Operation Display */
    entries[count].index = 0x6061U;
    entries[count].subindex = 0x00U;
    entries[count].type = SYN_CANOPEN_TYPE_I8;
    entries[count].access = SYN_CANOPEN_ACCESS_RO;
    entries[count].data_ptr = &drive->mode_display;
    entries[count].size = sizeof(drive->mode_display);
    count++;

    /* 0x6064: Position Actual Value */
    entries[count].index = 0x6064U;
    entries[count].subindex = 0x00U;
    entries[count].type = SYN_CANOPEN_TYPE_I32;
    entries[count].access = SYN_CANOPEN_ACCESS_RO;
    entries[count].data_ptr = &drive->actual_position;
    entries[count].size = sizeof(drive->actual_position);
    count++;

    /* 0x606C: Velocity Actual Value */
    entries[count].index = 0x606CU;
    entries[count].subindex = 0x00U;
    entries[count].type = SYN_CANOPEN_TYPE_I32;
    entries[count].access = SYN_CANOPEN_ACCESS_RO;
    entries[count].data_ptr = &drive->actual_velocity;
    entries[count].size = sizeof(drive->actual_velocity);
    count++;

    /* 0x6071: Target Torque */
    entries[count].index = 0x6071U;
    entries[count].subindex = 0x00U;
    entries[count].type = SYN_CANOPEN_TYPE_I16;
    entries[count].access = SYN_CANOPEN_ACCESS_RW;
    entries[count].data_ptr = &drive->target_torque;
    entries[count].size = sizeof(drive->target_torque);
    count++;

    /* 0x6077: Torque Actual Value */
    entries[count].index = 0x6077U;
    entries[count].subindex = 0x00U;
    entries[count].type = SYN_CANOPEN_TYPE_I16;
    entries[count].access = SYN_CANOPEN_ACCESS_RO;
    entries[count].data_ptr = &drive->actual_torque;
    entries[count].size = sizeof(drive->actual_torque);
    count++;

    /* 0x607A: Target Position */
    entries[count].index = 0x607AU;
    entries[count].subindex = 0x00U;
    entries[count].type = SYN_CANOPEN_TYPE_I32;
    entries[count].access = SYN_CANOPEN_ACCESS_RW;
    entries[count].data_ptr = &drive->target_position;
    entries[count].size = sizeof(drive->target_position);
    count++;

    /* 0x60FF: Target Velocity */
    entries[count].index = 0x60FFU;
    entries[count].subindex = 0x00U;
    entries[count].type = SYN_CANOPEN_TYPE_I32;
    entries[count].access = SYN_CANOPEN_ACCESS_RW;
    entries[count].data_ptr = &drive->target_velocity;
    entries[count].size = sizeof(drive->target_velocity);
    count++;

    return count;
}
```


