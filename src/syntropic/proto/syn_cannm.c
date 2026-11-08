/**
 * @file syn_cannm.c
 * @brief AUTOSAR CAN Network Management (CanNm) Protocol Implementation.
 * @ingroup syn_protocol
 */

#include "syn_cannm.h"

#include <string.h>

#if !defined(SYN_USE_CANNM) || SYN_USE_CANNM

void syn_cannm_init(SYN_CanNM_Session *session, const SYN_CanNM_Config *cfg)
{
    if (!session) {
        return;
    }

    memset(session, 0, sizeof(*session));

    if (cfg) {
        session->config = *cfg;
    } else {
        /* Default AUTOSAR CAN NM Timers & IDs */
        session->config.node_id = 0x01U;
        session->config.can_id_base = 0x400U;
        session->config.can_id_mask = 0x7F0U;
        session->config.msg_cycle_ms = 100U;
        session->config.nm_timeout_ms = 1000U;
        session->config.wait_bus_sleep_ms = 1500U;
        session->config.repeat_msg_time_ms = 1600U;
    }

    session->state = SYN_CANNM_STATE_BUS_SLEEP;
    session->prev_state = SYN_CANNM_STATE_BUS_SLEEP;
}

void syn_cannm_request_network(SYN_CanNM_Session *session)
{
    if (!session) {
        return;
    }
    session->node_comm_req = true;
    if (session->state == SYN_CANNM_STATE_BUS_SLEEP ||
        session->state == SYN_CANNM_STATE_PRE_BUS_SLEEP) {
        session->active_wakeup = true;
    }
}

void syn_cannm_release_network(SYN_CanNM_Session *session)
{
    if (!session) {
        return;
    }
    session->node_comm_req = false;
}

void syn_cannm_request_repeat_msg(SYN_CanNM_Session *session)
{
    if (!session) {
        return;
    }
    session->repeat_msg_req = true;
}

void syn_cannm_set_user_data(SYN_CanNM_Session *session, const uint8_t *data, size_t len)
{
    if (!session) {
        return;
    }
    memset(session->user_data, 0, sizeof(session->user_data));
    if (data && len > 0) {
        size_t copy_len = len > 6U ? 6U : len;
        memcpy(session->user_data, data, copy_len);
    }
}

bool syn_cannm_process_rx_frame(SYN_CanNM_Session *session, const SYN_CAN_Frame *frame)
{
    if (!session || !frame) {
        return false;
    }

    /* Verify DLC and CAN ID matching */
    if (frame->dlc < 2U) {
        return false;
    }

    uint32_t mask = session->config.can_id_mask;
    if ((frame->id & mask) != (session->config.can_id_base & mask)) {
        return false;
    }

    session->rx_source_node_id = frame->data[0];
    session->rx_cbv = frame->data[1];
    if (frame->dlc >= 8U) {
        memcpy(session->rx_user_data, &frame->data[2], 6U);
    }

    /* Restart timeout timer on any valid CAN NM reception */
    session->timeout_timer = session->config.nm_timeout_ms;

    /* Handle state transitions triggered by Rx */
    switch (session->state) {
    case SYN_CANNM_STATE_BUS_SLEEP:
    case SYN_CANNM_STATE_PRE_BUS_SLEEP:
        if (!session->node_comm_req) {
            session->active_wakeup = false;
        }
        session->prev_state = session->state;
        session->state = SYN_CANNM_STATE_REPEAT_MSG;
        session->repeat_msg_timer = session->config.repeat_msg_time_ms;
        session->msg_cycle_timer = 0U; /* Transmit immediately */
        break;

    case SYN_CANNM_STATE_NORMAL_OP:
    case SYN_CANNM_STATE_READY_SLEEP:
        if ((session->rx_cbv & SYN_CANNM_CBV_REPEAT_MSG_REQ) != 0U) {
            session->prev_state = session->state;
            session->state = SYN_CANNM_STATE_REPEAT_MSG;
            session->repeat_msg_timer = session->config.repeat_msg_time_ms;
            session->msg_cycle_timer = 0U;
        }
        break;

    case SYN_CANNM_STATE_REPEAT_MSG:
    default:
        break;
    }

    return true;
}

/**
 * @brief Pack CAN NM PDU payload into tx_frame.
 * @param session Pointer to CAN NM session instance.
 * @param tx_frame Pointer to target CAN frame structure.
 */
static void pack_nm_tx_pdu(SYN_CanNM_Session *session, SYN_CAN_Frame *tx_frame)
{
    tx_frame->id = session->config.can_id_base + session->config.node_id;
    tx_frame->extended = false;
    tx_frame->dlc = 8U;
    tx_frame->data[0] = session->config.node_id;

    uint8_t cbv = 0U;
    if (session->repeat_msg_req) {
        cbv |= SYN_CANNM_CBV_REPEAT_MSG_REQ;
    }
    if (session->active_wakeup) {
        cbv |= SYN_CANNM_CBV_ACTIVE_WAKEUP_REQ;
    }
    tx_frame->data[1] = cbv;

    memcpy(&tx_frame->data[2], session->user_data, 6U);
}

/**
 * @brief Reset message cycle and timeout timers and emit a periodic NM PDU.
 * @param session  Pointer to CAN NM session instance.
 * @param tx_frame Optional output CAN frame buffer.
 * @return true if tx_frame was populated, false otherwise.
 */
static bool cannm_emit_cycle_pdu(SYN_CanNM_Session *session, SYN_CAN_Frame *tx_frame)
{
    session->msg_cycle_timer = session->config.msg_cycle_ms;
    session->timeout_timer = session->config.nm_timeout_ms;
    if (tx_frame != NULL) {
        pack_nm_tx_pdu(session, tx_frame);
        return true;
    }
    return false;
}

/**
 * @brief Transition session into Repeat Message state and emit an immediate NM PDU.
 * @param session  Pointer to CAN NM session instance.
 * @param tx_frame Optional output CAN frame buffer.
 * @return true if tx_frame was populated, false otherwise.
 */
static bool cannm_enter_repeat_msg(SYN_CanNM_Session *session, SYN_CAN_Frame *tx_frame)
{
    session->prev_state = session->state;
    session->state = SYN_CANNM_STATE_REPEAT_MSG;
    session->repeat_msg_timer = session->config.repeat_msg_time_ms;
    return cannm_emit_cycle_pdu(session, tx_frame);
}

/**
 * @brief Advance timers and state transitions while in Repeat Message state.
 * @param session  Pointer to CAN NM session instance.
 * @param delta_ms Elapsed time since previous step in milliseconds.
 * @param tx_frame Optional output CAN frame buffer.
 * @return true if a periodic NM frame was generated, false otherwise.
 */
static bool cannm_step_repeat_msg(SYN_CanNM_Session *session, uint32_t delta_ms,
                                  SYN_CAN_Frame *tx_frame)
{
    if (session->repeat_msg_timer <= delta_ms) {
        session->repeat_msg_timer = 0U;
        session->prev_state = session->state;
        session->repeat_msg_req = false;
        session->state =
            session->node_comm_req ? SYN_CANNM_STATE_NORMAL_OP : SYN_CANNM_STATE_READY_SLEEP;
    } else {
        session->repeat_msg_timer -= delta_ms;
    }

    if (session->msg_cycle_timer <= delta_ms) {
        return cannm_emit_cycle_pdu(session, tx_frame);
    }
    session->msg_cycle_timer -= delta_ms;
    return false;
}

/**
 * @brief Advance timers and state transitions while in Normal Operation state.
 * @param session  Pointer to CAN NM session instance.
 * @param delta_ms Elapsed time since previous step in milliseconds.
 * @param tx_frame Optional output CAN frame buffer.
 * @return true if a periodic NM frame was generated, false otherwise.
 */
static bool cannm_step_normal_op(SYN_CanNM_Session *session, uint32_t delta_ms,
                                 SYN_CAN_Frame *tx_frame)
{
    if (session->repeat_msg_req) {
        return cannm_enter_repeat_msg(session, tx_frame);
    }
    if (!session->node_comm_req) {
        session->prev_state = session->state;
        session->state = SYN_CANNM_STATE_READY_SLEEP;
        return false;
    }
    if (session->msg_cycle_timer <= delta_ms) {
        return cannm_emit_cycle_pdu(session, tx_frame);
    }
    session->msg_cycle_timer -= delta_ms;
    return false;
}

/**
 * @brief Advance timeout timer and state transitions while in Ready Sleep state.
 * @param session  Pointer to CAN NM session instance.
 * @param delta_ms Elapsed time since previous step in milliseconds.
 * @param tx_frame Optional output CAN frame buffer.
 * @return true if an NM frame was generated on Repeat Message entry, false otherwise.
 */
static bool cannm_step_ready_sleep(SYN_CanNM_Session *session, uint32_t delta_ms,
                                   SYN_CAN_Frame *tx_frame)
{
    if (session->repeat_msg_req) {
        return cannm_enter_repeat_msg(session, tx_frame);
    }
    if (session->node_comm_req) {
        session->prev_state = session->state;
        session->state = SYN_CANNM_STATE_NORMAL_OP;
        session->msg_cycle_timer = 0U;
        return false;
    }
    if (session->timeout_timer <= delta_ms) {
        session->timeout_timer = 0U;
        session->prev_state = session->state;
        session->state = SYN_CANNM_STATE_PRE_BUS_SLEEP;
        session->wait_bus_sleep_timer = session->config.wait_bus_sleep_ms;
        return false;
    }
    session->timeout_timer -= delta_ms;
    return false;
}

bool syn_cannm_step(SYN_CanNM_Session *session, uint32_t delta_ms, SYN_CAN_Frame *tx_frame)
{
    if (!session) {
        return false;
    }

    if ((session->state == SYN_CANNM_STATE_BUS_SLEEP ||
         session->state == SYN_CANNM_STATE_PRE_BUS_SLEEP) &&
        session->node_comm_req) {
        session->active_wakeup = true;
        return cannm_enter_repeat_msg(session, tx_frame);
    }

    switch (session->state) {
    case SYN_CANNM_STATE_PRE_BUS_SLEEP:
        if (session->wait_bus_sleep_timer <= delta_ms) {
            session->wait_bus_sleep_timer = 0U;
            session->prev_state = session->state;
            session->state = SYN_CANNM_STATE_BUS_SLEEP;
            session->active_wakeup = false;
            session->repeat_msg_req = false;
        } else {
            session->wait_bus_sleep_timer -= delta_ms;
        }
        return false;

    case SYN_CANNM_STATE_REPEAT_MSG:
        return cannm_step_repeat_msg(session, delta_ms, tx_frame);

    case SYN_CANNM_STATE_NORMAL_OP:
        return cannm_step_normal_op(session, delta_ms, tx_frame);

    case SYN_CANNM_STATE_READY_SLEEP:
        return cannm_step_ready_sleep(session, delta_ms, tx_frame);

    case SYN_CANNM_STATE_BUS_SLEEP:
    default:
        session->active_wakeup = false;
        session->repeat_msg_req = false;
        return false;
    }
}

#endif /* SYN_USE_CANNM */
