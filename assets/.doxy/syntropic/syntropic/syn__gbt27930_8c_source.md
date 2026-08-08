

# File syn\_gbt27930.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_gbt27930.c**](syn__gbt27930_8c.md)

[Go to the documentation of this file](syn__gbt27930_8c.md)


```C++

#include "syn_gbt27930.h"

#if !defined(SYN_USE_GBT27930) || SYN_USE_GBT27930

#include <string.h>

void syn_gbt27930_init(SYN_GBT27930_Session *session, SYN_GBT27930_Role role)
{
    if (session == NULL) {
        return;
    }

    memset(session, 0, sizeof(*session));
    session->role = role;
    session->state = SYN_GBT27930_STATE_IDLE;
    session->telemetry.charge_mode = 0x01; /* Default CV */
}

SYN_Status syn_gbt27930_start_handshake(SYN_GBT27930_Session *session)
{
    if (session == NULL) {
        return SYN_INVALID_PARAM;
    }

    session->state = SYN_GBT27930_STATE_HANDSHAKE;
    session->timer_ms = 0;
    session->timeout_ms = 0;
    session->ready_for_charging = false;
    session->peer_ready_for_charging = false;
    return SYN_OK;
}

void syn_gbt27930_stop_charging(SYN_GBT27930_Session *session, uint8_t reason)
{
    if (session == NULL) {
        return;
    }

    session->state = SYN_GBT27930_STATE_STOPPING;
    session->stop_reason = reason;
    session->timer_ms = 0;
}

SYN_Status syn_gbt27930_process_rx_frame(SYN_GBT27930_Session *session, const SYN_CAN_Frame *frame)
{
    if (session == NULL || frame == NULL || !frame->extended) {
        return SYN_INVALID_PARAM;
    }

    SYN_J1939_Header hdr;
    if (syn_j1939_id_unpack(frame->id, &hdr) != SYN_OK) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Unreachable guard */
    }

    uint32_t pgn = hdr.pgn;

    /* Handle Received Frames by PGN */
    switch (pgn) {
    case SYN_GBT27930_PGN_CHM: /* Charger Handshake */
        if (session->role == SYN_GBT27930_ROLE_BMS &&
            session->state == SYN_GBT27930_STATE_HANDSHAKE) {
            session->timeout_ms = 0;
        }
        break;

    case SYN_GBT27930_PGN_BHM: /* BMS Handshake */
        if (session->role == SYN_GBT27930_ROLE_CHARGER &&
            session->state == SYN_GBT27930_STATE_HANDSHAKE) {
            if (frame->dlc >= 2) {
                session->bms_cfg.max_charge_volt_v =
                    (uint16_t)(frame->data[0] | ((uint16_t)frame->data[1] << 8));
                session->timeout_ms = 0;
            }
        }
        break;

    case SYN_GBT27930_PGN_CRM: /* Charger Recognition */
        if (session->role == SYN_GBT27930_ROLE_BMS &&
            session->state == SYN_GBT27930_STATE_HANDSHAKE) {
            if (frame->dlc >= 1 && frame->data[0] == 0x00) {
                session->state = SYN_GBT27930_STATE_PARAM_CONFIG;
                session->timeout_ms = 0;
            }
        }
        break;

    case SYN_GBT27930_PGN_BRM: /* BMS Recognition */
        if (session->role == SYN_GBT27930_ROLE_CHARGER &&
            session->state == SYN_GBT27930_STATE_HANDSHAKE) {
            session->state = SYN_GBT27930_STATE_PARAM_CONFIG;
            session->timeout_ms = 0;
        }
        break;

    case SYN_GBT27930_PGN_BCP: /* BMS Charging Parameter */
        if (session->role == SYN_GBT27930_ROLE_CHARGER &&
            session->state == SYN_GBT27930_STATE_PARAM_CONFIG) {
            if (frame->dlc >= 6) {
                session->bms_cfg.max_charge_volt_v =
                    (uint16_t)(frame->data[0] | ((uint16_t)frame->data[1] << 8));
                session->bms_cfg.max_charge_curr_a =
                    (uint16_t)(frame->data[2] | ((uint16_t)frame->data[3] << 8));
                session->timeout_ms = 0;
            }
        }
        break;

    case SYN_GBT27930_PGN_CML: /* Charger Max Output */
        if (session->role == SYN_GBT27930_ROLE_BMS &&
            session->state == SYN_GBT27930_STATE_PARAM_CONFIG) {
            if (frame->dlc >= 8) {
                session->charger_cfg.max_output_volt_v =
                    (uint16_t)(frame->data[0] | ((uint16_t)frame->data[1] << 8));
                session->charger_cfg.min_output_volt_v =
                    (uint16_t)(frame->data[2] | ((uint16_t)frame->data[3] << 8));
                session->timeout_ms = 0;
            }
        }
        break;

    case SYN_GBT27930_PGN_BRO: /* BMS Ready State */
        if (session->role == SYN_GBT27930_ROLE_CHARGER) {
            if (frame->dlc >= 1 && frame->data[0] == 0xAA) {
                session->peer_ready_for_charging = true;
                session->timeout_ms = 0;
                if (session->ready_for_charging && session->peer_ready_for_charging) {
                    session->state = SYN_GBT27930_STATE_CHARGING;
                }
            }
        }
        break;

    case SYN_GBT27930_PGN_CRO: /* Charger Output Ready State */
        if (session->role == SYN_GBT27930_ROLE_BMS) {
            if (frame->dlc >= 1 && frame->data[0] == 0xAA) {
                session->peer_ready_for_charging = true;
                session->timeout_ms = 0;
                if (session->ready_for_charging && session->peer_ready_for_charging) {
                    session->state = SYN_GBT27930_STATE_CHARGING;
                }
            }
        }
        break;

    case SYN_GBT27930_PGN_BCL: /* BMS Charging Demand */
        if (session->role == SYN_GBT27930_ROLE_CHARGER &&
            session->state == SYN_GBT27930_STATE_CHARGING) {
            if (frame->dlc >= 5) {
                session->telemetry.volt_demand_v =
                    (uint16_t)(frame->data[0] | ((uint16_t)frame->data[1] << 8));
                session->telemetry.curr_demand_a =
                    (uint16_t)(frame->data[2] | ((uint16_t)frame->data[3] << 8));
                session->telemetry.charge_mode = frame->data[4];
                session->timeout_ms = 0;
            }
        }
        break;

    case SYN_GBT27930_PGN_BCS: /* BMS Overall Status */
        if (session->role == SYN_GBT27930_ROLE_CHARGER &&
            session->state == SYN_GBT27930_STATE_CHARGING) {
            if (frame->dlc >= 7) {
                session->telemetry.measured_volt_v =
                    (uint16_t)(frame->data[0] | ((uint16_t)frame->data[1] << 8));
                session->telemetry.measured_curr_a =
                    (uint16_t)(frame->data[2] | ((uint16_t)frame->data[3] << 8));
                session->telemetry.soc_percent = frame->data[6];
                session->timeout_ms = 0;
            }
        }
        break;

    case SYN_GBT27930_PGN_CCS: /* Charger Charging Status */
        if (session->role == SYN_GBT27930_ROLE_BMS &&
            session->state == SYN_GBT27930_STATE_CHARGING) {
            if (frame->dlc >= 6) {
                session->telemetry.measured_volt_v =
                    (uint16_t)(frame->data[0] | ((uint16_t)frame->data[1] << 8));
                session->telemetry.measured_curr_a =
                    (uint16_t)(frame->data[2] | ((uint16_t)frame->data[3] << 8));
                session->timeout_ms = 0;
            }
        }
        break;

    case SYN_GBT27930_PGN_BST: /* BMS Stop Charging */
    case SYN_GBT27930_PGN_CST: /* Charger Stop Charging */
        session->state = SYN_GBT27930_STATE_STOPPING;
        session->timeout_ms = 0;
        if (frame->dlc >= 1) {
            session->stop_reason = frame->data[0];
        }
        break;

    case SYN_GBT27930_PGN_BEM: /* BMS Error */
    case SYN_GBT27930_PGN_CEM: /* Charger Error */
        session->state = SYN_GBT27930_STATE_ERROR;
        session->timeout_ms = 0;
        if (frame->dlc >= 1) {
            session->fault_code = frame->data[0];
        }
        break;

    default:
        break;
    }

    return SYN_OK;
}

bool syn_gbt27930_step(SYN_GBT27930_Session *session, uint32_t dt_ms, SYN_CAN_Frame *tx_frame)
{
    if (session == NULL || tx_frame == NULL || session->state == SYN_GBT27930_STATE_IDLE) {
        return false;
    }

    session->timer_ms += dt_ms;
    session->timeout_ms += dt_ms;

    /* Specification-mandated message timeout monitoring */
    uint32_t timeout_limit_ms = (session->state == SYN_GBT27930_STATE_CHARGING) ? 1000 : 5000;
    if (session->state != SYN_GBT27930_STATE_ERROR &&
        session->state != SYN_GBT27930_STATE_STOPPING && session->timeout_ms >= timeout_limit_ms) {
        session->state = SYN_GBT27930_STATE_ERROR;
        session->fault_code = 0x01; /* Communication Timeout Fault */
    }

    /* Frame Transmission Intervals */
    uint32_t interval_ms = 250;
    if (session->state == SYN_GBT27930_STATE_CHARGING) {
        interval_ms = 50; /* Fast 50ms periodic demand/status loop */
    } else if (session->state == SYN_GBT27930_STATE_STOPPING) {
        interval_ms = 10; /* Fast 10ms stop notification loop */
    } else if (session->state == SYN_GBT27930_STATE_ERROR) {
        interval_ms = 100; /* Periodic 100ms error broadcast loop */
    }

    if (session->timer_ms < interval_ms) {
        return false;
    }
    session->timer_ms = 0;

    memset(tx_frame, 0, sizeof(*tx_frame));
    tx_frame->extended = true; /* Extended 29-bit CAN ID */
    tx_frame->dlc = 8;

    uint8_t sa = (session->role == SYN_GBT27930_ROLE_BMS) ? SYN_GBT27930_ADDR_BMS
                                                          : SYN_GBT27930_ADDR_CHARGER;
    uint8_t da = (session->role == SYN_GBT27930_ROLE_BMS) ? SYN_GBT27930_ADDR_CHARGER
                                                          : SYN_GBT27930_ADDR_BMS;

    if (session->role == SYN_GBT27930_ROLE_BMS) {
        switch (session->state) {
        case SYN_GBT27930_STATE_HANDSHAKE:
            tx_frame->id = syn_j1939_id_pack(6, SYN_GBT27930_PGN_BHM, sa, da);
            tx_frame->data[0] = (uint8_t)(session->bms_cfg.max_charge_volt_v & 0xFF);
            tx_frame->data[1] = (uint8_t)((session->bms_cfg.max_charge_volt_v >> 8) & 0xFF);
            return true;

        case SYN_GBT27930_STATE_PARAM_CONFIG:
            tx_frame->id = syn_j1939_id_pack(6, SYN_GBT27930_PGN_BRO, sa, da);
            tx_frame->data[0] = session->ready_for_charging ? 0xAA : 0x00;
            return true;

        case SYN_GBT27930_STATE_CHARGING:
            tx_frame->id = syn_j1939_id_pack(6, SYN_GBT27930_PGN_BCL, sa, da);
            tx_frame->data[0] = (uint8_t)(session->telemetry.volt_demand_v & 0xFF);
            tx_frame->data[1] = (uint8_t)((session->telemetry.volt_demand_v >> 8) & 0xFF);
            tx_frame->data[2] = (uint8_t)(session->telemetry.curr_demand_a & 0xFF);
            tx_frame->data[3] = (uint8_t)((session->telemetry.curr_demand_a >> 8) & 0xFF);
            tx_frame->data[4] = session->telemetry.charge_mode;
            return true;

        case SYN_GBT27930_STATE_STOPPING:
            tx_frame->id = syn_j1939_id_pack(4, SYN_GBT27930_PGN_BST, sa, da);
            tx_frame->data[0] = session->stop_reason;
            return true;

        case SYN_GBT27930_STATE_ERROR:
            tx_frame->id = syn_j1939_id_pack(2, SYN_GBT27930_PGN_BEM, sa, da);
            tx_frame->data[0] = session->fault_code;
            return true;

        default:
            return false;
        }
    } else { /* Charger Role */
        switch (session->state) {
        case SYN_GBT27930_STATE_HANDSHAKE:
            tx_frame->id = syn_j1939_id_pack(6, SYN_GBT27930_PGN_CHM, sa, da);
            tx_frame->data[0] = 0x01; /* Version 1.1 */
            return true;

        case SYN_GBT27930_STATE_PARAM_CONFIG:
            tx_frame->id = syn_j1939_id_pack(6, SYN_GBT27930_PGN_CRO, sa, da);
            tx_frame->data[0] = session->ready_for_charging ? 0xAA : 0x00;
            return true;

        case SYN_GBT27930_STATE_CHARGING:
            tx_frame->id = syn_j1939_id_pack(6, SYN_GBT27930_PGN_CCS, sa, da);
            tx_frame->data[0] = (uint8_t)(session->telemetry.measured_volt_v & 0xFF);
            tx_frame->data[1] = (uint8_t)((session->telemetry.measured_volt_v >> 8) & 0xFF);
            tx_frame->data[2] = (uint8_t)(session->telemetry.measured_curr_a & 0xFF);
            tx_frame->data[3] = (uint8_t)((session->telemetry.measured_curr_a >> 8) & 0xFF);
            return true;

        case SYN_GBT27930_STATE_STOPPING:
            tx_frame->id = syn_j1939_id_pack(4, SYN_GBT27930_PGN_CST, sa, da);
            tx_frame->data[0] = session->stop_reason;
            return true;

        case SYN_GBT27930_STATE_ERROR:
            tx_frame->id = syn_j1939_id_pack(2, SYN_GBT27930_PGN_CEM, sa, da);
            tx_frame->data[0] = session->fault_code;
            return true;

        default:
            return false;
        }
    }
}

#endif /* SYN_USE_GBT27930 */
```


