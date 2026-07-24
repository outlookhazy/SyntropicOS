

# File syn\_canopen\_mgr.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_canopen\_mgr.c**](syn__canopen__mgr_8c.md)

[Go to the documentation of this file](syn__canopen__mgr_8c.md)


```C++

#include "syn_canopen_mgr.h"
#include "../util/syn_assert.h"
#include "../util/syn_pack.h"
#include <string.h>

void syn_canopen_mgr_init(SYN_CANOpenManager *mgr)
{
    SYN_ASSERT(mgr != NULL);
    memset(mgr, 0, sizeof(*mgr));
}

void syn_canopen_mgr_build_nmt(SYN_CAN_Frame *frame, uint8_t target_node, uint8_t nmt_cmd)
{
    SYN_ASSERT(frame != NULL);
    memset(frame, 0, sizeof(*frame));
    frame->id = 0x000U; /* NMT COB-ID */
    frame->dlc = 2;
    frame->data[0] = nmt_cmd;
    frame->data[1] = target_node;
}

SYN_Status syn_canopen_mgr_sdo_read_init(SYN_CANOpenManager *mgr, SYN_CAN_Frame *frame,
                                          uint8_t node_id, uint16_t index, uint8_t subindex)
{
    SYN_ASSERT(mgr != NULL && frame != NULL);
    if (node_id < 1 || node_id > 127) return SYN_INVALID_PARAM;
    if (mgr->sdo_client.state == SYN_SDO_CLIENT_STATE_WAIT_READ ||
        mgr->sdo_client.state == SYN_SDO_CLIENT_STATE_WAIT_WRITE) {
        return SYN_BUSY;
    }

    mgr->sdo_client.target_node = node_id;
    mgr->sdo_client.index       = index;
    mgr->sdo_client.subindex    = subindex;
    mgr->sdo_client.state       = SYN_SDO_CLIENT_STATE_WAIT_READ;
    mgr->sdo_client.abort_code  = 0;
    mgr->sdo_client.timeout_ms  = 1000;

    memset(frame, 0, sizeof(*frame));
    frame->id = 0x600U + node_id; /* SDO Request COB-ID */
    frame->dlc = 8;
    frame->data[0] = 0x40U; /* Initiate Upload Request */
    syn_poke_u16_le(index, frame->data, 1);
    frame->data[3] = subindex;

    return SYN_OK;
}

SYN_Status syn_canopen_mgr_sdo_write_init(SYN_CANOpenManager *mgr, SYN_CAN_Frame *frame,
                                           uint8_t node_id, uint16_t index, uint8_t subindex,
                                           const void *data, size_t len)
{
    SYN_ASSERT(mgr != NULL && frame != NULL && data != NULL);
    if (node_id < 1 || node_id > 127 || len == 0 || len > 4) return SYN_INVALID_PARAM;
    if (mgr->sdo_client.state == SYN_SDO_CLIENT_STATE_WAIT_READ ||
        mgr->sdo_client.state == SYN_SDO_CLIENT_STATE_WAIT_WRITE) {
        return SYN_BUSY;
    }

    mgr->sdo_client.target_node = node_id;
    mgr->sdo_client.index       = index;
    mgr->sdo_client.subindex    = subindex;
    mgr->sdo_client.state       = SYN_SDO_CLIENT_STATE_WAIT_WRITE;
    mgr->sdo_client.abort_code  = 0;
    mgr->sdo_client.data_len    = len;
    memcpy(mgr->sdo_client.data, data, len);
    mgr->sdo_client.timeout_ms  = 1000;

    memset(frame, 0, sizeof(*frame));
    frame->id = 0x600U + node_id; /* SDO Request COB-ID */
    frame->dlc = 8;
    /* Command byte for expedited download: CS=1 (0x20), e=1, s=1, n=(4-len) */
    uint8_t cs = 0x23U | ((uint8_t)(4 - len) << 2);
    frame->data[0] = cs;
    syn_poke_u16_le(index, frame->data, 1);
    frame->data[3] = subindex;
    memcpy(&frame->data[4], data, len);

    return SYN_OK;
}

void syn_canopen_mgr_process_frame(SYN_CANOpenManager *mgr, const SYN_CAN_Frame *frame)
{
    SYN_ASSERT(mgr != NULL && frame != NULL);

    /* Check Heartbeat (COB-ID 0x700 + NodeID) */
    if (frame->id >= 0x701U && frame->id <= 0x77FU && frame->dlc >= 1) {
        uint8_t node_id = (uint8_t)(frame->id - 0x700U);
        mgr->nodes[node_id].node_id   = node_id;
        mgr->nodes[node_id].nmt_state = frame->data[0] & 0x7FU;
        mgr->nodes[node_id].timer_ms  = 0;
        mgr->nodes[node_id].online     = true;
        return;
    }

    /* Check SDO Response (COB-ID 0x580 + NodeID) */
    if (mgr->sdo_client.state != SYN_SDO_CLIENT_STATE_IDLE) {
        uint16_t resp_cob = 0x580U + mgr->sdo_client.target_node;
        if (frame->id == resp_cob && frame->dlc >= 8) {
            uint8_t cs = frame->data[0];

            /* Check Abort Domain Transfer (0x80) */
            if (cs == 0x80U) {
                mgr->sdo_client.state = SYN_SDO_CLIENT_STATE_ERROR;
                mgr->sdo_client.abort_code = syn_peek_u32_le(frame->data, 4);
                return;
            }

            if (mgr->sdo_client.state == SYN_SDO_CLIENT_STATE_WAIT_READ) {
                /* Upload Response (0x43/0x4B/0x4F/0x47) */
                if ((cs & 0xE0U) == 0x40U) {
                    uint8_t n = (cs >> 2) & 0x03U;
                    size_t len = 4 - n;
                    mgr->sdo_client.data_len = len;
                    memcpy(mgr->sdo_client.data, &frame->data[4], len);
                    mgr->sdo_client.state = SYN_SDO_CLIENT_STATE_SUCCESS;
                }
            } else if (mgr->sdo_client.state == SYN_SDO_CLIENT_STATE_WAIT_WRITE) {
                /* Download Response (0x60) */
                if ((cs & 0xE0U) == 0x60U) {
                    mgr->sdo_client.state = SYN_SDO_CLIENT_STATE_SUCCESS;
                }
            }
        }
    }
}

void syn_canopen_mgr_step(SYN_CANOpenManager *mgr, uint16_t delta_ms)
{
    SYN_ASSERT(mgr != NULL);

    /* Advance node heartbeat timers */
    for (uint16_t i = 1; i <= 127; i++) {
        if (mgr->nodes[i].online) {
            mgr->nodes[i].timer_ms += delta_ms;
            if (mgr->nodes[i].timer_ms > 3000) {
                mgr->nodes[i].online = false;
            }
        }
    }

    /* Advance SDO client timeout */
    if (mgr->sdo_client.state == SYN_SDO_CLIENT_STATE_WAIT_READ ||
        mgr->sdo_client.state == SYN_SDO_CLIENT_STATE_WAIT_WRITE) {
        if (mgr->sdo_client.timeout_ms <= delta_ms) {
            mgr->sdo_client.state = SYN_SDO_CLIENT_STATE_ERROR;
            mgr->sdo_client.abort_code = 0x05040000UL; /* SDO Protocol Timeout */
        } else {
            mgr->sdo_client.timeout_ms -= delta_ms;
        }
    }
}
```


