

# File syn\_j1939.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_j1939.c**](syn__j1939_8c.md)

[Go to the documentation of this file](syn__j1939_8c.md)


```C++

#include "syn_j1939.h"
#include "../util/syn_assert.h"
#include "../util/syn_pack.h"
#include <string.h>

uint32_t syn_j1939_calc_pgn(uint8_t dp, uint8_t pf, uint8_t ps)
{
    if (pf < 240U) {
        return (((uint32_t)dp & 0x01U) << 16) | (((uint32_t)pf) << 8);
    }
    return (((uint32_t)dp & 0x01U) << 16) | (((uint32_t)pf) << 8) | (uint32_t)ps;
}

uint32_t syn_j1939_id_pack(uint8_t priority, uint32_t pgn, uint8_t sa, uint8_t da)
{
    uint8_t dp = (uint8_t)((pgn >> 16) & 0x01U);
    uint8_t pf = (uint8_t)((pgn >> 8) & 0xFFU);
    uint8_t ps = (uint8_t)(pgn & 0xFFU);

    if (pf < 240U) {
        ps = da; /* Destination-specific PDU1 format */
    }

    uint32_t id = 0;
    id |= ((uint32_t)(priority & 0x07U)) << 26;
    id |= ((uint32_t)(dp & 0x01U)) << 24;
    id |= ((uint32_t)pf) << 16;
    id |= ((uint32_t)ps) << 8;
    id |= ((uint32_t)sa);

    return id;
}

SYN_Status syn_j1939_id_unpack(uint32_t can_id, SYN_J1939_Header *header)
{
    if (!header) return SYN_INVALID_PARAM;

    header->priority = (uint8_t)((can_id >> 26) & 0x07U);
    header->dp       = (uint8_t)((can_id >> 24) & 0x01U);
    header->pf       = (uint8_t)((can_id >> 16) & 0xFFU);
    header->ps       = (uint8_t)((can_id >> 8) & 0xFFU);
    header->sa       = (uint8_t)(can_id & 0xFFU);
    header->is_pdu1  = (header->pf < 240U);

    if (header->is_pdu1) {
        header->da  = header->ps;
        header->pgn = syn_j1939_calc_pgn(header->dp, header->pf, 0);
    } else {
        header->da  = SYN_J1939_ADDR_GLOBAL;
        header->pgn = syn_j1939_calc_pgn(header->dp, header->pf, header->ps);
    }

    return SYN_OK;
}

void syn_j1939_name_encode(const SYN_J1939_Name *name, uint8_t buf[8])
{
    if (!name || !buf) return;
    memset(buf, 0, 8);

    uint64_t n = 0;
    n |= ((uint64_t)(name->identity_number & 0x1FFFFFU));
    n |= ((uint64_t)(name->manufacturer_code & 0x7FFU)) << 21;
    n |= ((uint64_t)(name->ecu_instance & 0x07U)) << 32;
    n |= ((uint64_t)(name->function_instance & 0x1FU)) << 35;
    n |= ((uint64_t)(name->function)) << 40;
    n |= ((uint64_t)(name->vehicle_system & 0x7FU)) << 49;
    n |= ((uint64_t)(name->vehicle_system_inst & 0x0FU)) << 56;
    n |= ((uint64_t)(name->industry_group & 0x07U)) << 60;
    n |= ((uint64_t)(name->arbitrary_addr_cap ? 1U : 0U)) << 63;

    size_t pos = 0;
    syn_pack_u64_le(buf, &pos, n);
}

void syn_j1939_name_decode(const uint8_t buf[8], SYN_J1939_Name *name)
{
    if (!name || !buf) return;
    memset(name, 0, sizeof(*name));

    size_t pos = 0;
    uint64_t n = syn_unpack_u64_le(buf, &pos);

    name->identity_number    = (uint32_t)(n & 0x1FFFFFU);
    name->manufacturer_code  = (uint16_t)((n >> 21) & 0x7FFU);
    name->ecu_instance       = (uint8_t)((n >> 32) & 0x07U);
    name->function_instance  = (uint8_t)((n >> 35) & 0x1FU);
    name->function           = (uint8_t)((n >> 40) & 0xFFU);
    name->vehicle_system     = (uint8_t)((n >> 49) & 0x7FU);
    name->vehicle_system_inst= (uint8_t)((n >> 56) & 0x0FU);
    name->industry_group     = (uint8_t)((n >> 60) & 0x07U);
    name->arbitrary_addr_cap = ((n >> 63) & 0x01U) != 0;
}

SYN_Status syn_j1939_node_init(SYN_J1939_Node *node, uint8_t sa, const SYN_J1939_Name *name)
{
    if (!node) return SYN_INVALID_PARAM;

    memset(node, 0, sizeof(*node));
    node->sa = sa;
    if (name) {
        node->name = *name;
    }
    node->state = SYN_J1939_STATE_UNCLAIMED;

    return SYN_OK;
}

SYN_Status syn_j1939_build_address_claim(const SYN_J1939_Node *node, SYN_CAN_Frame *frame)
{
    if (!node || !frame) return SYN_INVALID_PARAM;

    frame->id  = syn_j1939_id_pack(6, SYN_J1939_PGN_ADDR_CLAIM, node->sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;
    syn_j1939_name_encode(&node->name, frame->data);

    return SYN_OK;
}

SYN_Status syn_j1939_build_request(uint8_t sa, uint8_t da, uint32_t requested_pgn, SYN_CAN_Frame *frame)
{
    if (!frame) return SYN_INVALID_PARAM;

    frame->id  = syn_j1939_id_pack(6, SYN_J1939_PGN_REQUEST, sa, da);
    frame->dlc = 3;
    frame->extended = true;

    frame->data[0] = (uint8_t)(requested_pgn & 0xFFU);
    frame->data[1] = (uint8_t)((requested_pgn >> 8) & 0xFFU);
    frame->data[2] = (uint8_t)((requested_pgn >> 16) & 0xFFU);

    return SYN_OK;
}

SYN_Status syn_j1939_build_tp_bam(uint8_t sa, uint32_t pgn, uint16_t total_bytes, SYN_CAN_Frame *frame)
{
    if (!frame || total_bytes < 9 || total_bytes > 1785) return SYN_INVALID_PARAM;

    frame->id  = syn_j1939_id_pack(6, SYN_J1939_PGN_TP_CM, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    uint8_t total_packets = (uint8_t)((total_bytes + 6U) / 7U);

    frame->data[0] = SYN_J1939_TP_CTRL_BAM;
    frame->data[1] = (uint8_t)(total_bytes & 0xFFU);
    frame->data[2] = (uint8_t)((total_bytes >> 8) & 0xFFU);
    frame->data[3] = total_packets;
    frame->data[4] = 0xFFU; /* Reserved */
    frame->data[5] = (uint8_t)(pgn & 0xFFU);
    frame->data[6] = (uint8_t)((pgn >> 8) & 0xFFU);
    frame->data[7] = (uint8_t)((pgn >> 16) & 0xFFU);

    return SYN_OK;
}

SYN_Status syn_j1939_build_tp_dt(uint8_t sa, uint8_t sequence, const uint8_t *payload, size_t payload_len, SYN_CAN_Frame *frame)
{
    if (!frame || !payload || sequence == 0 || payload_len == 0 || payload_len > 7) {
        return SYN_INVALID_PARAM;
    }

    frame->id  = syn_j1939_id_pack(6, SYN_J1939_PGN_TP_DT, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    frame->data[0] = sequence;
    memset(&frame->data[1], 0xFF, 7);
    memcpy(&frame->data[1], payload, payload_len);

    return SYN_OK;
}

size_t syn_j1939_encode_dm1(uint8_t *buf, size_t buf_size, const SYN_J1939_DTC *dtc_list, size_t dtc_count, uint8_t mil_lamp_status)
{
    if (!buf || buf_size < 2) return 0;

    buf[0] = (uint8_t)(mil_lamp_status & 0xFFU);
    buf[1] = 0xFFU; /* Reserved / Flash Lamp status */

    if (dtc_count == 0 || !dtc_list) {
        if (buf_size >= 6) {
            memset(&buf[2], 0, 4);
            return 6;
        }
        return 2;
    }

    size_t offset = 2;
    for (size_t i = 0; i < dtc_count; i++) {
        if (offset + 4 > buf_size) break;

        uint32_t spn = dtc_list[i].spn & 0x7FFFFU;
        uint8_t fmi = dtc_list[i].fmi & 0x1FU;
        uint8_t oc  = dtc_list[i].occurrence_count & 0x7FU;
        uint8_t cm  = (uint8_t)(dtc_list[i].conversion_method & 0x01U);

        buf[offset + 0] = (uint8_t)(spn & 0xFFU);
        buf[offset + 1] = (uint8_t)((spn >> 8) & 0xFFU);
        buf[offset + 2] = (uint8_t)(((spn >> 11) & 0xE0U) | fmi);
        buf[offset + 3] = (uint8_t)((cm << 7) | oc);

        offset += 4;
    }

    return offset;
}

SYN_Status syn_j1939_process_frame(SYN_J1939_Node *node,
                                   const SYN_CAN_Frame *frame,
                                   uint32_t *out_pgn,
                                   const uint8_t **out_data,
                                   size_t *out_len)
{
    if (!node || !frame || !out_pgn || !out_data || !out_len) {
        return SYN_INVALID_PARAM;
    }

    SYN_J1939_Header hdr;
    if (syn_j1939_id_unpack(frame->id, &hdr) != SYN_OK) {
        return SYN_INVALID_PARAM;
    }

    /* Handle Multi-Packet Transport Protocol Connection Management (TP.CM) */
    if (hdr.pgn == SYN_J1939_PGN_TP_CM && frame->dlc == 8) {
        uint8_t ctrl = frame->data[0];
        if (ctrl == SYN_J1939_TP_CTRL_BAM || ctrl == SYN_J1939_TP_CTRL_RTS) {
            uint16_t total_bytes = (uint16_t)frame->data[1] | ((uint16_t)frame->data[2] << 8);
            uint8_t total_packets = frame->data[3];
            uint32_t pgn = (uint32_t)frame->data[5] | ((uint32_t)frame->data[6] << 8) | ((uint32_t)frame->data[7] << 16);

            if (total_bytes <= sizeof(node->tp_rx.data) && total_packets > 0) {
                node->tp_rx.active = true;
                node->tp_rx.is_bam = (ctrl == SYN_J1939_TP_CTRL_BAM);
                node->tp_rx.sa = hdr.sa;
                node->tp_rx.da = hdr.da;
                node->tp_rx.pgn = pgn;
                node->tp_rx.total_bytes = total_bytes;
                node->tp_rx.total_packets = total_packets;
                node->tp_rx.next_seq = 1;
                node->tp_rx.received_packets = 0;
            }
        }
        return SYN_BUSY;
    }

    /* Handle Multi-Packet Transport Protocol Data Transfer (TP.DT) */
    if (hdr.pgn == SYN_J1939_PGN_TP_DT && node->tp_rx.active && hdr.sa == node->tp_rx.sa) {
        uint8_t seq = frame->data[0];
        if (seq == node->tp_rx.next_seq) {
            size_t offset = (size_t)(seq - 1) * 7U;
            size_t copy_len = 7U;
            if (offset + copy_len > node->tp_rx.total_bytes) {
                copy_len = node->tp_rx.total_bytes - offset;
            }
            if (offset + copy_len <= sizeof(node->tp_rx.data)) {
                memcpy(&node->tp_rx.data[offset], &frame->data[1], copy_len);
                node->tp_rx.next_seq++;
                node->tp_rx.received_packets++;

                if (node->tp_rx.received_packets >= node->tp_rx.total_packets) {
                    *out_pgn  = node->tp_rx.pgn;
                    *out_data = node->tp_rx.data;
                    *out_len  = node->tp_rx.total_bytes;
                    node->tp_rx.active = false;
                    return SYN_OK;
                }
            }
        }
        return SYN_BUSY;
    }

    /* Single-frame packet handling */
    *out_pgn  = hdr.pgn;
    *out_data = frame->data;
    *out_len  = frame->dlc;

    return SYN_OK;
}
```


