

# File syn\_cannm.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cannm.c**](syn__cannm_8c.md)

[Go to the documentation of this file](syn__cannm_8c.md)


```C++

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
        session->prev_state = session->state;
        session->state = SYN_CANNM_STATE_REPEAT_MSG;
        session->repeat_msg_timer = session->config.repeat_msg_time_ms;
        session->msg_cycle_timer = 0U; /* Transmit immediately */
        break;

    case SYN_CANNM_STATE_NORMAL_OP:
    case SYN_CANNM_STATE_READY_SLEEP:
        if (session->rx_cbv & SYN_CANNM_CBV_REPEAT_MSG_REQ) {
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
    if (session->node_comm_req) {
        cbv |= SYN_CANNM_CBV_ACTIVE_WAKEUP_REQ;
    }
    tx_frame->data[1] = cbv;

    memcpy(&tx_frame->data[2], session->user_data, 6U);
}

bool syn_cannm_step(SYN_CanNM_Session *session, uint32_t delta_ms, SYN_CAN_Frame *tx_frame)
{
    if (!session) {
        return false;
    }

    bool generate_tx = false;

    /* Bus Wakeup checks */
    if ((session->state == SYN_CANNM_STATE_BUS_SLEEP ||
         session->state == SYN_CANNM_STATE_PRE_BUS_SLEEP) &&
        session->node_comm_req) {
        session->prev_state = session->state;
        session->state = SYN_CANNM_STATE_REPEAT_MSG;
        session->repeat_msg_timer = session->config.repeat_msg_time_ms;
        session->timeout_timer = session->config.nm_timeout_ms;
        session->msg_cycle_timer = 0U;
    }

    /* Execute active state logic */
    switch (session->state) {
    case SYN_CANNM_STATE_BUS_SLEEP:
        break;

    case SYN_CANNM_STATE_PRE_BUS_SLEEP:
        if (session->wait_bus_sleep_timer <= delta_ms) {
            session->wait_bus_sleep_timer = 0U;
            session->prev_state = session->state;
            session->state = SYN_CANNM_STATE_BUS_SLEEP;
        } else {
            session->wait_bus_sleep_timer -= delta_ms;
        }
        break;

    case SYN_CANNM_STATE_REPEAT_MSG:
        /* Timer countdowns */
        if (session->repeat_msg_timer <= delta_ms) {
            session->repeat_msg_timer = 0U;
            session->prev_state = session->state;
            if (session->node_comm_req) {
                session->state = SYN_CANNM_STATE_NORMAL_OP;
            } else {
                session->state = SYN_CANNM_STATE_READY_SLEEP;
            }
        } else {
            session->repeat_msg_timer -= delta_ms;
        }

        if (session->msg_cycle_timer <= delta_ms) {
            session->msg_cycle_timer = session->config.msg_cycle_ms;
            session->timeout_timer = session->config.nm_timeout_ms;
            if (tx_frame) {
                pack_nm_tx_pdu(session, tx_frame);
                generate_tx = true;
            }
        } else {
            session->msg_cycle_timer -= delta_ms;
        }
        break;

    case SYN_CANNM_STATE_NORMAL_OP:
        if (session->repeat_msg_req) {
            session->prev_state = session->state;
            session->state = SYN_CANNM_STATE_REPEAT_MSG;
            session->repeat_msg_timer = session->config.repeat_msg_time_ms;
            session->msg_cycle_timer = 0U;
            break;
        }

        if (!session->node_comm_req) {
            session->prev_state = session->state;
            session->state = SYN_CANNM_STATE_READY_SLEEP;
            break;
        }

        if (session->msg_cycle_timer <= delta_ms) {
            session->msg_cycle_timer = session->config.msg_cycle_ms;
            session->timeout_timer = session->config.nm_timeout_ms;
            if (tx_frame) {
                pack_nm_tx_pdu(session, tx_frame);
                generate_tx = true;
            }
        } else {
            session->msg_cycle_timer -= delta_ms;
        }
        break;

    case SYN_CANNM_STATE_READY_SLEEP:
        if (session->repeat_msg_req) {
            session->prev_state = session->state;
            session->state = SYN_CANNM_STATE_REPEAT_MSG;
            session->repeat_msg_timer = session->config.repeat_msg_time_ms;
            session->msg_cycle_timer = 0U;
            break;
        }

        if (session->node_comm_req) {
            session->prev_state = session->state;
            session->state = SYN_CANNM_STATE_NORMAL_OP;
            session->msg_cycle_timer = 0U;
            break;
        }

        if (session->timeout_timer <= delta_ms) {
            session->timeout_timer = 0U;
            session->prev_state = session->state;
            session->state = SYN_CANNM_STATE_PRE_BUS_SLEEP;
            session->wait_bus_sleep_timer = session->config.wait_bus_sleep_ms;
        } else {
            session->timeout_timer -= delta_ms;
        }
        break;
    }

    session->repeat_msg_req = false;
    return generate_tx;
}

#endif /* SYN_USE_CANNM */
```


