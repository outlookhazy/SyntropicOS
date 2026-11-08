/**
 * @file syn_gbt27930.c
 * @brief GB/T 27930 EV DC Fast Charging Protocol Implementation.
 */

#include "syn_gbt27930.h"

#if !defined(SYN_USE_GBT27930) || SYN_USE_GBT27930

#include <string.h>

/**
 * @brief Pack 16-bit unsigned integer into little-endian byte buffer.
 * @param dst Target 2-byte output buffer.
 * @param val 16-bit unsigned value to encode.
 */
static void gbt_pack_u16_le(uint8_t *dst, uint16_t val)
{
    dst[0] = (uint8_t)(val & 0xFFU);
    dst[1] = (uint8_t)((val >> 8U) & 0xFFU);
}

/**
 * @brief Unpack 16-bit unsigned integer from little-endian byte buffer.
 * @param src Source 2-byte input buffer.
 * @return Decoded 16-bit unsigned value.
 */
static uint16_t gbt_unpack_u16_le(const uint8_t *src)
{
    return (uint16_t)((uint16_t)src[0] | ((uint16_t)src[1] << 8U));
}

void syn_gbt27930_init(SYN_GBT27930_Session *session, SYN_GBT27930_Role role)
{
    if (session == NULL) {
        return;
    }

    memset(session, 0, sizeof(*session));
    session->role = role;
    session->state = SYN_GBT27930_STATE_IDLE;
    session->telemetry.charge_mode = 0x01U; /* Default CV */
}

SYN_Status syn_gbt27930_start_handshake(SYN_GBT27930_Session *session)
{
    if (session == NULL) {
        return SYN_INVALID_PARAM;
    }

    session->state = SYN_GBT27930_STATE_HANDSHAKE;
    session->timer_ms = 0U;
    session->timeout_ms = 0U;
    session->ready_for_charging = false;
    session->peer_ready_for_charging = false;
    session->crm00_received = false;
    session->brm_received = false;
    session->bcp_received = false;
    session->cml_sent = false;
    session->cml_received = false;
    session->cts_received = false;
    session->ready_tx_sent = false;
    session->stop_tx_sent = false;
    session->peer_stopped = false;
    session->stats_received = false;
    session->tx_substep = 0U;
    return SYN_OK;
}

void syn_gbt27930_stop_charging(SYN_GBT27930_Session *session, uint8_t reason)
{
    if (session == NULL) {
        return;
    }

    session->state = SYN_GBT27930_STATE_STOPPING;
    session->stop_reason = reason;
    session->timer_ms = 0U;
    session->timeout_ms = 0U;
    session->tx_substep = 0U;
    session->stop_tx_sent = false;
    session->peer_stopped = false;
    session->stats_received = false;
}

/**
 * @brief Handle received stop charging frame (BST or CST) and update session state.
 * @param session GB/T 27930 session instance.
 * @param frame Received CAN frame.
 */
static void gbt_handle_stop_rx(SYN_GBT27930_Session *session, const SYN_CAN_Frame *frame)
{
    if (session->state == SYN_GBT27930_STATE_IDLE || session->state == SYN_GBT27930_STATE_ERROR ||
        session->state == SYN_GBT27930_STATE_END) {
        return;
    }

    if (frame->dlc >= 1U) {
        session->stop_reason = frame->data[0];
    }
    session->peer_stopped = true;
    session->timeout_ms = 0U;

    if (session->state == SYN_GBT27930_STATE_STOPPING && session->stop_tx_sent) {
        session->state = SYN_GBT27930_STATE_END;
        session->timer_ms = 0U;
        session->tx_substep = 0U;
        return;
    }

    if (session->state != SYN_GBT27930_STATE_STOPPING) {
        session->state = SYN_GBT27930_STATE_STOPPING;
        session->stop_tx_sent = false;
        session->timer_ms = 0U;
        session->tx_substep = 0U;
    }
}

/**
 * @brief Handle received error message frame (BEM or CEM) and transition to error state.
 * @param session GB/T 27930 session instance.
 * @param frame Received CAN frame.
 */
static void gbt_handle_error_rx(SYN_GBT27930_Session *session, const SYN_CAN_Frame *frame)
{
    if (session->state == SYN_GBT27930_STATE_IDLE) {
        return;
    }

    session->state = SYN_GBT27930_STATE_ERROR;
    session->timeout_ms = 0U;
    session->timer_ms = 0U;
    if (frame->dlc >= 1U) {
        session->fault_code = frame->data[0];
    }
}

/**
 * @brief Process BMS-side handshake frames (CHM, CRM) from charger.
 * @param session GB/T 27930 session instance.
 * @param pgn Unpacked J1939 parameter group number.
 * @param frame Received CAN frame.
 */
static void gbt_rx_bms_handshake(SYN_GBT27930_Session *session, uint32_t pgn,
                                 const SYN_CAN_Frame *frame)
{
    if (pgn == SYN_GBT27930_PGN_CHM && frame->dlc >= 1U) {
        session->timeout_ms = 0U;
        return;
    }

    if (pgn == SYN_GBT27930_PGN_CRM && frame->dlc >= 1U) {
        if (frame->data[0] == 0x00U) {
            session->crm00_received = true;
            session->timeout_ms = 0U;
        } else if (frame->data[0] == 0xAAU) {
            session->state = SYN_GBT27930_STATE_PARAM_CONFIG;
            session->timeout_ms = 0U;
            session->tx_substep = 0U;
            session->ready_for_charging = false;
            session->peer_ready_for_charging = false;
            session->ready_tx_sent = false;
        }
    }
}

/**
 * @brief Process BMS-side parameter configuration frames (CTS, CML, CRO) from charger.
 * @param session GB/T 27930 session instance.
 * @param pgn Unpacked J1939 parameter group number.
 * @param frame Received CAN frame.
 */
static void gbt_rx_bms_config(SYN_GBT27930_Session *session, uint32_t pgn,
                              const SYN_CAN_Frame *frame)
{
    if (pgn == SYN_GBT27930_PGN_CTS && frame->dlc >= 7U) {
        memcpy(session->sync_time_bcd, frame->data, 7U);
        session->cts_received = true;
        session->timeout_ms = 0U;
        return;
    }

    if (pgn == SYN_GBT27930_PGN_CML && frame->dlc >= 8U) {
        session->charger_cfg.max_output_volt_v = gbt_unpack_u16_le(&frame->data[0]);
        session->charger_cfg.min_output_volt_v = gbt_unpack_u16_le(&frame->data[2]);
        session->charger_cfg.max_output_curr_a = gbt_unpack_u16_le(&frame->data[4]);
        session->charger_cfg.min_output_curr_a = gbt_unpack_u16_le(&frame->data[6]);
        session->cml_received = true;
        session->timeout_ms = 0U;
        session->tx_substep = 0U;
        return;
    }

    if (pgn == SYN_GBT27930_PGN_CRO && frame->dlc >= 1U) {
        if (frame->data[0] == 0x00U) {
            session->timeout_ms = 0U;
        } else if (frame->data[0] == 0xAAU) {
            session->peer_ready_for_charging = true;
            session->timeout_ms = 0U;
            if (session->ready_for_charging && session->ready_tx_sent) {
                session->state = SYN_GBT27930_STATE_CHARGING;
                session->tx_substep = 0U;
            }
        }
    }
}

/**
 * @brief Process BMS-side charging, stop, statistics, and error frames (CCS, CST, CSD, CEM).
 * @param session GB/T 27930 session instance.
 * @param pgn Unpacked J1939 parameter group number.
 * @param frame Received CAN frame.
 */
static void gbt_rx_bms_runtime(SYN_GBT27930_Session *session, uint32_t pgn,
                               const SYN_CAN_Frame *frame)
{
    if (pgn == SYN_GBT27930_PGN_CCS && session->state == SYN_GBT27930_STATE_CHARGING &&
        frame->dlc >= 6U) {
        session->telemetry.measured_volt_v = gbt_unpack_u16_le(&frame->data[0]);
        session->telemetry.measured_curr_a = gbt_unpack_u16_le(&frame->data[2]);
        session->telemetry.cumulative_time_min = gbt_unpack_u16_le(&frame->data[4]);
        session->timeout_ms = 0U;
    } else if (pgn == SYN_GBT27930_PGN_CST) {
        gbt_handle_stop_rx(session, frame);
    } else if (pgn == SYN_GBT27930_PGN_CSD &&
               (session->state == SYN_GBT27930_STATE_STOPPING ||
                session->state == SYN_GBT27930_STATE_END) &&
               frame->dlc >= 4U) {
        session->telemetry.cumulative_time_min = gbt_unpack_u16_le(&frame->data[0]);
        session->telemetry.output_energy_kwh = gbt_unpack_u16_le(&frame->data[2]);
        session->stats_received = true;
        session->state = SYN_GBT27930_STATE_END;
        session->timeout_ms = 0U;
    } else if (pgn == SYN_GBT27930_PGN_CEM) {
        gbt_handle_error_rx(session, frame);
    }
}

/**
 * @brief Dispatch inbound CAN frame for a BMS-role GB/T 27930 session.
 * @param session GB/T 27930 session instance.
 * @param pgn Unpacked J1939 parameter group number.
 * @param frame Received CAN frame.
 */
static void gbt_rx_bms(SYN_GBT27930_Session *session, uint32_t pgn, const SYN_CAN_Frame *frame)
{
    if (session->state == SYN_GBT27930_STATE_HANDSHAKE &&
        (pgn == SYN_GBT27930_PGN_CHM || pgn == SYN_GBT27930_PGN_CRM)) {
        gbt_rx_bms_handshake(session, pgn, frame);
    } else if (session->state == SYN_GBT27930_STATE_PARAM_CONFIG &&
               (pgn == SYN_GBT27930_PGN_CTS || pgn == SYN_GBT27930_PGN_CML ||
                pgn == SYN_GBT27930_PGN_CRO)) {
        gbt_rx_bms_config(session, pgn, frame);
    } else {
        gbt_rx_bms_runtime(session, pgn, frame);
    }
}

/**
 * @brief Process charger-side handshake and configuration frames (BHM, BRM, BCP, BRO).
 * @param session GB/T 27930 session instance.
 * @param pgn Unpacked J1939 parameter group number.
 * @param frame Received CAN frame.
 */
static void gbt_rx_charger_handshake_config(SYN_GBT27930_Session *session, uint32_t pgn,
                                            const SYN_CAN_Frame *frame)
{
    if (session->state == SYN_GBT27930_STATE_HANDSHAKE) {
        if (pgn == SYN_GBT27930_PGN_BHM && frame->dlc >= 2U) {
            session->bms_cfg.max_charge_volt_v = gbt_unpack_u16_le(&frame->data[0]);
            session->timeout_ms = 0U;
        } else if (pgn == SYN_GBT27930_PGN_BRM && frame->dlc >= 8U) {
            session->bms_cfg.battery_type = frame->data[3];
            session->bms_cfg.rated_capacity_ah = gbt_unpack_u16_le(&frame->data[4]);
            session->bms_cfg.rated_voltage_v = gbt_unpack_u16_le(&frame->data[6]);
            session->brm_received = true;
            session->timeout_ms = 0U;
        }
        return;
    }

    if (pgn == SYN_GBT27930_PGN_BCP && frame->dlc >= 6U) {
        session->bms_cfg.max_charge_volt_v = gbt_unpack_u16_le(&frame->data[0]);
        session->bms_cfg.max_charge_curr_a = gbt_unpack_u16_le(&frame->data[2]);
        session->bcp_received = true;
        session->timeout_ms = 0U;
    } else if (pgn == SYN_GBT27930_PGN_BRO && frame->dlc >= 1U) {
        if (frame->data[0] == 0x00U) {
            session->timeout_ms = 0U;
        } else if (frame->data[0] == 0xAAU) {
            session->peer_ready_for_charging = true;
            session->timeout_ms = 0U;
            if (session->ready_for_charging && session->ready_tx_sent) {
                session->state = SYN_GBT27930_STATE_CHARGING;
                session->tx_substep = 0U;
            }
        }
    }
}

/**
 * @brief Process charger runtime frames (BCL, BCS, BST, BSD, BEM).
 * @param session GB/T 27930 session instance.
 * @param pgn Unpacked J1939 parameter group number.
 * @param frame Received CAN frame.
 */
static void gbt_rx_charger_runtime(SYN_GBT27930_Session *session, uint32_t pgn,
                                   const SYN_CAN_Frame *frame)
{
    if (pgn == SYN_GBT27930_PGN_BCL && session->state == SYN_GBT27930_STATE_CHARGING &&
        frame->dlc >= 5U) {
        session->telemetry.volt_demand_v = gbt_unpack_u16_le(&frame->data[0]);
        session->telemetry.curr_demand_a = gbt_unpack_u16_le(&frame->data[2]);
        session->telemetry.charge_mode = frame->data[4];
        session->timeout_ms = 0U;
    } else if (pgn == SYN_GBT27930_PGN_BCS && session->state == SYN_GBT27930_STATE_CHARGING &&
               frame->dlc >= 7U) {
        session->telemetry.measured_volt_v = gbt_unpack_u16_le(&frame->data[0]);
        session->telemetry.measured_curr_a = gbt_unpack_u16_le(&frame->data[2]);
        session->telemetry.soc_percent = frame->data[6];
        session->timeout_ms = 0U;
    } else if (pgn == SYN_GBT27930_PGN_BST) {
        gbt_handle_stop_rx(session, frame);
    } else if (pgn == SYN_GBT27930_PGN_BSD &&
               (session->state == SYN_GBT27930_STATE_STOPPING ||
                session->state == SYN_GBT27930_STATE_END) &&
               frame->dlc >= 7U) {
        session->telemetry.soc_percent = frame->data[0];
        session->telemetry.max_cell_volt_v = gbt_unpack_u16_le(&frame->data[3]);
        session->telemetry.min_cell_temp_c = frame->data[5];
        session->telemetry.max_cell_temp_c = frame->data[6];
        session->stats_received = true;
        session->state = SYN_GBT27930_STATE_END;
        session->timeout_ms = 0U;
    } else if (pgn == SYN_GBT27930_PGN_BEM) {
        gbt_handle_error_rx(session, frame);
    }
}

/**
 * @brief Dispatch inbound CAN frame for a charger-role GB/T 27930 session.
 * @param session GB/T 27930 session instance.
 * @param pgn Unpacked J1939 parameter group number.
 * @param frame Received CAN frame.
 */
static void gbt_rx_charger(SYN_GBT27930_Session *session, uint32_t pgn, const SYN_CAN_Frame *frame)
{
    if ((session->state == SYN_GBT27930_STATE_HANDSHAKE &&
         (pgn == SYN_GBT27930_PGN_BHM || pgn == SYN_GBT27930_PGN_BRM)) ||
        (session->state == SYN_GBT27930_STATE_PARAM_CONFIG &&
         (pgn == SYN_GBT27930_PGN_BCP || pgn == SYN_GBT27930_PGN_BRO))) {
        gbt_rx_charger_handshake_config(session, pgn, frame);
    } else {
        gbt_rx_charger_runtime(session, pgn, frame);
    }
}

SYN_Status syn_gbt27930_process_rx_frame(SYN_GBT27930_Session *session, const SYN_CAN_Frame *frame)
{
    if (session == NULL || frame == NULL || !frame->extended) {
        return SYN_INVALID_PARAM;
    }

    SYN_J1939_Header hdr;
    (void)syn_j1939_id_unpack(frame->id, &hdr);

    if (session->role == SYN_GBT27930_ROLE_BMS) {
        gbt_rx_bms(session, hdr.pgn, frame);
    } else {
        gbt_rx_charger(session, hdr.pgn, frame);
    }

    return SYN_OK;
}

/**
 * @brief Build periodic BMS frame for handshake and parameter configuration phases.
 * @param session GB/T 27930 session instance.
 * @param sa Source CAN node address.
 * @param da Destination CAN node address.
 * @param tx_frame Output CAN frame buffer.
 * @return true when a CAN frame was populated for transmission.
 */
static bool gbt_tx_bms_early(SYN_GBT27930_Session *session, uint8_t sa, uint8_t da,
                             SYN_CAN_Frame *tx_frame)
{
    if (session->state == SYN_GBT27930_STATE_HANDSHAKE) {
        if (!session->crm00_received) {
            tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BHM, sa, da);
            gbt_pack_u16_le(&tx_frame->data[0], session->bms_cfg.max_charge_volt_v);
        } else {
            tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BRM, sa, da);
            tx_frame->data[0] = 0x01U;
            tx_frame->data[1] = 0x01U;
            tx_frame->data[2] = 0x00U;
            tx_frame->data[3] = session->bms_cfg.battery_type;
            gbt_pack_u16_le(&tx_frame->data[4], session->bms_cfg.rated_capacity_ah);
            gbt_pack_u16_le(&tx_frame->data[6], session->bms_cfg.rated_voltage_v);
        }
        session->tx_substep++;
        return true;
    }

    if (!session->ready_for_charging) {
        if (session->cml_received && (session->tx_substep & 1U) == 1U) {
            tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BRO, sa, da);
            tx_frame->data[0] = 0x00U;
        } else {
            tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BCP, sa, da);
            gbt_pack_u16_le(&tx_frame->data[0], session->bms_cfg.max_charge_volt_v);
            gbt_pack_u16_le(&tx_frame->data[2], session->bms_cfg.max_charge_curr_a);
            gbt_pack_u16_le(&tx_frame->data[4], session->bms_cfg.max_cell_volt_v);
            tx_frame->data[6] = (uint8_t)(session->bms_cfg.max_temp_c & 0xFFU);
            tx_frame->data[7] = session->telemetry.soc_percent;
        }
        session->tx_substep++;
        return true;
    }

    tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BRO, sa, da);
    tx_frame->data[0] = 0xAAU;
    session->ready_tx_sent = true;
    if (session->peer_ready_for_charging) {
        session->state = SYN_GBT27930_STATE_CHARGING;
        session->tx_substep = 0U;
        session->timeout_ms = 0U;
    }
    return true;
}

/**
 * @brief Build next periodic transmit frame for a BMS-role GB/T 27930 session.
 * @param session GB/T 27930 session instance.
 * @param sa Source CAN node address.
 * @param da Destination CAN node address.
 * @param tx_frame Output CAN frame buffer.
 * @return true when a CAN frame is ready for transmission, false otherwise.
 */
static bool gbt_tx_bms(SYN_GBT27930_Session *session, uint8_t sa, uint8_t da,
                       SYN_CAN_Frame *tx_frame)
{
    if (session->state == SYN_GBT27930_STATE_HANDSHAKE ||
        session->state == SYN_GBT27930_STATE_PARAM_CONFIG) {
        return gbt_tx_bms_early(session, sa, da, tx_frame);
    }

    switch (session->state) {
    case SYN_GBT27930_STATE_CHARGING:
        if ((session->tx_substep & 1U) == 0U) {
            tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BCL, sa, da);
            gbt_pack_u16_le(&tx_frame->data[0], session->telemetry.volt_demand_v);
            gbt_pack_u16_le(&tx_frame->data[2], session->telemetry.curr_demand_a);
            tx_frame->data[4] = session->telemetry.charge_mode;
        } else {
            tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BCS, sa, da);
            gbt_pack_u16_le(&tx_frame->data[0], session->telemetry.measured_volt_v);
            gbt_pack_u16_le(&tx_frame->data[2], session->telemetry.measured_curr_a);
            gbt_pack_u16_le(&tx_frame->data[4], session->telemetry.max_cell_volt_v);
            tx_frame->data[6] = session->telemetry.soc_percent;
            tx_frame->data[7] = (uint8_t)(session->telemetry.remaining_time_min & 0xFFU);
        }
        session->tx_substep++;
        return true;

    case SYN_GBT27930_STATE_STOPPING:
        tx_frame->id = syn_j1939_id_pack(4U, SYN_GBT27930_PGN_BST, sa, da);
        tx_frame->data[0] = session->stop_reason;
        session->stop_tx_sent = true;
        if (session->peer_stopped) {
            session->state = SYN_GBT27930_STATE_END;
            session->timer_ms = 0U;
            session->tx_substep = 0U;
        }
        return true;

    case SYN_GBT27930_STATE_END:
        tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_BSD, sa, da);
        tx_frame->data[0] = session->telemetry.soc_percent;
        gbt_pack_u16_le(&tx_frame->data[3], session->telemetry.max_cell_volt_v);
        tx_frame->data[5] = session->telemetry.min_cell_temp_c;
        tx_frame->data[6] = session->telemetry.max_cell_temp_c;
        session->tx_substep++;
        return true;

    case SYN_GBT27930_STATE_ERROR:
        tx_frame->id = syn_j1939_id_pack(2U, SYN_GBT27930_PGN_BEM, sa, da);
        tx_frame->data[0] = session->fault_code;
        return true;

    default:
        return false;
    }
}

/**
 * @brief Build periodic charger frame for handshake and parameter configuration phases.
 * @param session GB/T 27930 session instance.
 * @param sa Source CAN node address.
 * @param da Destination CAN node address.
 * @param tx_frame Output CAN frame buffer.
 * @return true when a CAN frame was populated for transmission.
 */
static bool gbt_tx_charger_early(SYN_GBT27930_Session *session, uint8_t sa, uint8_t da,
                                 SYN_CAN_Frame *tx_frame)
{
    if (session->state == SYN_GBT27930_STATE_HANDSHAKE) {
        if (session->brm_received) {
            tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CRM, sa, da);
            tx_frame->data[0] = 0xAAU;
            session->state = SYN_GBT27930_STATE_PARAM_CONFIG;
            session->tx_substep = 0U;
            session->timeout_ms = 0U;
            session->ready_for_charging = false;
            session->peer_ready_for_charging = false;
            session->ready_tx_sent = false;
            return true;
        }
        if ((session->tx_substep & 1U) == 0U) {
            tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CHM, sa, da);
            tx_frame->data[0] = 0x01U;
            tx_frame->data[1] = 0x01U;
            tx_frame->data[2] = 0x00U;
        } else {
            tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CRM, sa, da);
            tx_frame->data[0] = 0x00U;
        }
        session->tx_substep++;
        return true;
    }

    if (session->ready_for_charging) {
        tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CRO, sa, da);
        tx_frame->data[0] = 0xAAU;
        session->ready_tx_sent = true;
        if (session->peer_ready_for_charging) {
            session->state = SYN_GBT27930_STATE_CHARGING;
            session->tx_substep = 0U;
            session->timeout_ms = 0U;
        }
        return true;
    }

    if (session->bcp_received && session->cml_sent) {
        tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CRO, sa, da);
        tx_frame->data[0] = 0x00U;
        session->tx_substep++;
        return true;
    }

    if ((session->tx_substep & 1U) == 0U) {
        tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CTS, sa, da);
        if (session->sync_time_bcd[6] != 0U) {
            memcpy(tx_frame->data, session->sync_time_bcd, 7U);
        } else {
            memset(tx_frame->data, 0xFF, 7U);
        }
        tx_frame->data[7] = 0xFFU;
        session->tx_substep++;
        return true;
    }

    tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CML, sa, da);
    gbt_pack_u16_le(&tx_frame->data[0], session->charger_cfg.max_output_volt_v);
    gbt_pack_u16_le(&tx_frame->data[2], session->charger_cfg.min_output_volt_v);
    gbt_pack_u16_le(&tx_frame->data[4], session->charger_cfg.max_output_curr_a);
    gbt_pack_u16_le(&tx_frame->data[6], session->charger_cfg.min_output_curr_a);
    session->cml_sent = true;
    session->tx_substep++;
    return true;
}

/**
 * @brief Build next periodic transmit frame for a charger-role GB/T 27930 session.
 * @param session GB/T 27930 session instance.
 * @param sa Source CAN node address.
 * @param da Destination CAN node address.
 * @param tx_frame Output CAN frame buffer.
 * @return true when a CAN frame is ready for transmission, false otherwise.
 */
static bool gbt_tx_charger(SYN_GBT27930_Session *session, uint8_t sa, uint8_t da,
                           SYN_CAN_Frame *tx_frame)
{
    if (session->state == SYN_GBT27930_STATE_HANDSHAKE ||
        session->state == SYN_GBT27930_STATE_PARAM_CONFIG) {
        return gbt_tx_charger_early(session, sa, da, tx_frame);
    }

    switch (session->state) {
    case SYN_GBT27930_STATE_CHARGING:
        tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CCS, sa, da);
        gbt_pack_u16_le(&tx_frame->data[0], session->telemetry.measured_volt_v);
        gbt_pack_u16_le(&tx_frame->data[2], session->telemetry.measured_curr_a);
        gbt_pack_u16_le(&tx_frame->data[4], session->telemetry.cumulative_time_min);
        tx_frame->data[6] = 0x01U;
        return true;

    case SYN_GBT27930_STATE_STOPPING:
        tx_frame->id = syn_j1939_id_pack(4U, SYN_GBT27930_PGN_CST, sa, da);
        tx_frame->data[0] = session->stop_reason;
        session->stop_tx_sent = true;
        if (session->peer_stopped) {
            session->state = SYN_GBT27930_STATE_END;
            session->timer_ms = 0U;
            session->tx_substep = 0U;
        }
        return true;

    case SYN_GBT27930_STATE_END:
        if (!session->stats_received && (session->tx_substep & 1U) == 1U) {
            tx_frame->id = syn_j1939_id_pack(4U, SYN_GBT27930_PGN_CST, sa, da);
            tx_frame->data[0] = session->stop_reason;
        } else {
            tx_frame->id = syn_j1939_id_pack(6U, SYN_GBT27930_PGN_CSD, sa, da);
            gbt_pack_u16_le(&tx_frame->data[0], session->telemetry.cumulative_time_min);
            gbt_pack_u16_le(&tx_frame->data[2], session->telemetry.output_energy_kwh);
        }
        session->tx_substep++;
        return true;

    case SYN_GBT27930_STATE_ERROR:
        tx_frame->id = syn_j1939_id_pack(2U, SYN_GBT27930_PGN_CEM, sa, da);
        tx_frame->data[0] = session->fault_code;
        return true;

    default:
        return false;
    }
}

/**
 * @brief Evaluate phase timeout and bounded STATE_END completion.
 * @param session GB/T 27930 session instance.
 * @return false if session transitioned to IDLE and needs no TX, true otherwise.
 */
static bool gbt_check_phase_bounds(SYN_GBT27930_Session *session)
{
    if (session->state == SYN_GBT27930_STATE_END && session->stats_received &&
        session->tx_substep >= 2U) {
        session->state = SYN_GBT27930_STATE_IDLE;
        return false;
    }
    uint32_t limit_ms = (session->state == SYN_GBT27930_STATE_CHARGING) ? 1000U : 5000U;
    if (session->state != SYN_GBT27930_STATE_ERROR && session->timeout_ms >= limit_ms) {
        session->state = SYN_GBT27930_STATE_ERROR;
        session->fault_code = 0x01U;
    }
    return true;
}

bool syn_gbt27930_step(SYN_GBT27930_Session *session, uint32_t dt_ms, SYN_CAN_Frame *tx_frame)
{
    if (session == NULL || tx_frame == NULL || session->state == SYN_GBT27930_STATE_IDLE) {
        return false;
    }

    session->timer_ms += dt_ms;
    session->timeout_ms += dt_ms;
    if (!gbt_check_phase_bounds(session)) {
        return false;
    }

    uint32_t interval_ms = 250U;
    if (session->state == SYN_GBT27930_STATE_CHARGING) {
        interval_ms = (session->role == SYN_GBT27930_ROLE_BMS) ? 25U : 50U;
    } else if (session->state == SYN_GBT27930_STATE_STOPPING) {
        interval_ms = 10U;
    } else if (session->state == SYN_GBT27930_STATE_ERROR) {
        interval_ms = 100U;
    }

    if (session->timer_ms < interval_ms) {
        return false;
    }
    session->timer_ms = 0U;

    memset(tx_frame, 0, sizeof(*tx_frame));
    tx_frame->extended = true;
    tx_frame->dlc = 8U;

    if (session->role == SYN_GBT27930_ROLE_BMS) {
        return gbt_tx_bms(session, SYN_GBT27930_ADDR_BMS, SYN_GBT27930_ADDR_CHARGER, tx_frame);
    }
    return gbt_tx_charger(session, SYN_GBT27930_ADDR_CHARGER, SYN_GBT27930_ADDR_BMS, tx_frame);
}

#endif /* SYN_USE_GBT27930 */
