#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_J1939) || SYN_USE_J1939

/**
 * @file syn_j1939.c
 * @brief SAE J1939 Heavy-Duty Vehicle CAN Protocol Stack implementation.
 */

#include "../util/syn_assert.h"
#include "../util/syn_pack.h"
#include "syn_j1939.h"

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
    if (!header)
        return SYN_INVALID_PARAM;

    header->priority = (uint8_t)((can_id >> 26) & 0x07U);
    header->dp = (uint8_t)((can_id >> 24) & 0x01U);
    header->pf = (uint8_t)((can_id >> 16) & 0xFFU);
    header->ps = (uint8_t)((can_id >> 8) & 0xFFU);
    header->sa = (uint8_t)(can_id & 0xFFU);
    header->is_pdu1 = (header->pf < 240U);

    if (header->is_pdu1) {
        header->da = header->ps;
        header->pgn = syn_j1939_calc_pgn(header->dp, header->pf, 0);
    } else {
        header->da = SYN_J1939_ADDR_GLOBAL;
        header->pgn = syn_j1939_calc_pgn(header->dp, header->pf, header->ps);
    }

    return SYN_OK;
}

void syn_j1939_name_encode(const SYN_J1939_Name *name, uint8_t buf[8])
{
    if (!name || !buf)
        return;
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
    if (!name || !buf)
        return;
    memset(name, 0, sizeof(*name));

    size_t pos = 0;
    uint64_t n = syn_unpack_u64_le(buf, &pos);

    name->identity_number = (uint32_t)(n & 0x1FFFFFU);
    name->manufacturer_code = (uint16_t)((n >> 21) & 0x7FFU);
    name->ecu_instance = (uint8_t)((n >> 32) & 0x07U);
    name->function_instance = (uint8_t)((n >> 35) & 0x1FU);
    name->function = (uint8_t)((n >> 40) & 0xFFU);
    name->vehicle_system = (uint8_t)((n >> 49) & 0x7FU);
    name->vehicle_system_inst = (uint8_t)((n >> 56) & 0x0FU);
    name->industry_group = (uint8_t)((n >> 60) & 0x07U);
    name->arbitrary_addr_cap = ((n >> 63) & 0x01U) != 0;
}

SYN_Status syn_j1939_node_init(SYN_J1939_Node *node, uint8_t sa, const SYN_J1939_Name *name)
{
    if (!node)
        return SYN_INVALID_PARAM;

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
    if (!node || !frame)
        return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(6, SYN_J1939_PGN_ADDR_CLAIM, node->sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;
    syn_j1939_name_encode(&node->name, frame->data);

    return SYN_OK;
}

SYN_Status syn_j1939_build_request(uint8_t sa, uint8_t da, uint32_t requested_pgn,
                                   SYN_CAN_Frame *frame)
{
    if (!frame)
        return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(6, SYN_J1939_PGN_REQUEST, sa, da);
    frame->dlc = 3;
    frame->extended = true;

    frame->data[0] = (uint8_t)(requested_pgn & 0xFFU);
    frame->data[1] = (uint8_t)((requested_pgn >> 8) & 0xFFU);
    frame->data[2] = (uint8_t)((requested_pgn >> 16) & 0xFFU);

    return SYN_OK;
}

SYN_Status syn_j1939_build_tp_bam(uint8_t sa, uint32_t pgn, uint16_t total_bytes,
                                  SYN_CAN_Frame *frame)
{
    if (!frame || total_bytes < 9 || total_bytes > 1785)
        return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(6, SYN_J1939_PGN_TP_CM, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    uint8_t total_packets = (uint8_t)((total_bytes + 6U) / 7U);

    frame->data[0] = SYN_J1939_TP_CTRL_BAM;
    syn_poke_u16_le(total_bytes, frame->data, 1);
    frame->data[3] = total_packets;
    frame->data[4] = 0xFFU; /* Reserved */
    frame->data[5] = (uint8_t)(pgn & 0xFFU);
    frame->data[6] = (uint8_t)((pgn >> 8) & 0xFFU);
    frame->data[7] = (uint8_t)((pgn >> 16) & 0xFFU);

    return SYN_OK;
}

SYN_Status syn_j1939_build_tp_rts(uint8_t sa, uint8_t da, uint32_t pgn, uint16_t total_bytes,
                                  uint8_t max_packets, SYN_CAN_Frame *frame)
{
    if (!frame || da >= SYN_J1939_ADDR_NULL || max_packets == 0U || total_bytes < 9U ||
        total_bytes > 1785U) {
        return SYN_INVALID_PARAM;
    }

    frame->id = syn_j1939_id_pack(7U, SYN_J1939_PGN_TP_CM, sa, da);
    frame->dlc = 8U;
    frame->extended = true;

    uint8_t total_packets = (uint8_t)((total_bytes + 6U) / 7U);
    frame->data[0] = SYN_J1939_TP_CTRL_RTS;
    syn_poke_u16_le(total_bytes, frame->data, 1U);
    frame->data[3] = total_packets;
    frame->data[4] = max_packets;
    frame->data[5] = (uint8_t)(pgn & 0xFFU);
    frame->data[6] = (uint8_t)((pgn >> 8U) & 0xFFU);
    frame->data[7] = (uint8_t)((pgn >> 16U) & 0xFFU);
    return SYN_OK;
}

SYN_Status syn_j1939_build_tp_dt_da(uint8_t sa, uint8_t da, uint8_t sequence,
                                    const uint8_t *payload, size_t payload_len,
                                    SYN_CAN_Frame *frame)
{
    if (!frame || !payload || sequence == 0U || payload_len == 0U || payload_len > 7U) {
        return SYN_INVALID_PARAM;
    }

    frame->id = syn_j1939_id_pack(7U, SYN_J1939_PGN_TP_DT, sa, da);
    frame->dlc = 8U;
    frame->extended = true;

    frame->data[0] = sequence;
    memset(&frame->data[1], 0xFF, 7U);
    memcpy(&frame->data[1], payload, payload_len);

    return SYN_OK;
}

SYN_Status syn_j1939_build_tp_dt(uint8_t sa, uint8_t sequence, const uint8_t *payload,
                                 size_t payload_len, SYN_CAN_Frame *frame)
{
    return syn_j1939_build_tp_dt_da(sa, SYN_J1939_ADDR_GLOBAL, sequence, payload, payload_len,
                                    frame);
}

bool syn_j1939_get_tx(SYN_J1939_Node *node, SYN_CAN_Frame *out_frame)
{
    if (!node || !out_frame || !node->tx_pending) {
        return false;
    }
    memset(out_frame, 0, sizeof(*out_frame));
    out_frame->id = node->tx_id;
    out_frame->dlc = 8U;
    out_frame->extended = true;
    memcpy(out_frame->data, node->tx_data, 8U);
    node->tx_pending = false;
    return true;
}

size_t syn_j1939_encode_dm1(uint8_t *buf, size_t buf_size, const SYN_J1939_DTC *dtc_list,
                            size_t dtc_count, uint8_t mil_lamp_status)
{
    if (!buf || buf_size < 2)
        return 0;

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
        if (offset + 4 > buf_size)
            break;

        uint32_t spn = dtc_list[i].spn & 0x7FFFFU;
        uint8_t fmi = dtc_list[i].fmi & 0x1FU;
        uint8_t oc = dtc_list[i].occurrence_count & 0x7FU;
        uint8_t cm = (uint8_t)(dtc_list[i].conversion_method & 0x01U);

        syn_poke_u16_le((uint16_t)spn, buf, offset);
        buf[offset + 2] = (uint8_t)(((spn >> 11) & 0xE0U) | fmi);
        buf[offset + 3] = (uint8_t)((cm << 7) | oc);

        offset += 4;
    }

    return offset;
}

/**
 * @brief Queue a J1939 TP.CM Clear-To-Send (CTS) frame on the node transmit slot.
 * @param node J1939 node instance.
 * @param da Destination CAN node address.
 * @param num_pkts Number of TP.DT packets permitted in this CTS window.
 * @param next_seq Next expected 1-based packet sequence number.
 * @param pgn Target multi-packet parameter group number.
 */
static void j1939_queue_tp_cts(SYN_J1939_Node *node, uint8_t da, uint8_t num_pkts, uint8_t next_seq,
                               uint32_t pgn)
{
    node->tx_id = syn_j1939_id_pack(7U, SYN_J1939_PGN_TP_CM, node->sa, da);
    node->tx_data[0] = SYN_J1939_TP_CTRL_CTS;
    node->tx_data[1] = num_pkts;
    node->tx_data[2] = next_seq;
    node->tx_data[3] = 0xFFU;
    node->tx_data[4] = 0xFFU;
    node->tx_data[5] = (uint8_t)(pgn & 0xFFU);
    node->tx_data[6] = (uint8_t)((pgn >> 8U) & 0xFFU);
    node->tx_data[7] = (uint8_t)((pgn >> 16U) & 0xFFU);
    node->tx_pending = true;
}

/**
 * @brief Queue a J1939 TP.CM End-of-Message Acknowledgment (ACK) frame on the node transmit slot.
 * @param node J1939 node instance.
 * @param da Destination CAN node address.
 * @param total_bytes Total reassembled payload byte count.
 * @param total_pkts Total received TP.DT packet count.
 * @param pgn Completed multi-packet parameter group number.
 */
static void j1939_queue_tp_ack(SYN_J1939_Node *node, uint8_t da, uint16_t total_bytes,
                               uint8_t total_pkts, uint32_t pgn)
{
    node->tx_id = syn_j1939_id_pack(7U, SYN_J1939_PGN_TP_CM, node->sa, da);
    node->tx_data[0] = SYN_J1939_TP_CTRL_ACK;
    syn_poke_u16_le(total_bytes, node->tx_data, 1U);
    node->tx_data[3] = total_pkts;
    node->tx_data[4] = 0xFFU;
    node->tx_data[5] = (uint8_t)(pgn & 0xFFU);
    node->tx_data[6] = (uint8_t)((pgn >> 8U) & 0xFFU);
    node->tx_data[7] = (uint8_t)((pgn >> 16U) & 0xFFU);
    node->tx_pending = true;
}

/**
 * @brief Queue a J1939 TP.CM Connection Abort frame on the node transmit slot.
 * @param node J1939 node instance.
 * @param da Destination CAN node address.
 * @param reason J1939-21 Connection Abort reason code (e.g. 1 = already in session).
 * @param pgn Target multi-packet parameter group number.
 */
static void j1939_queue_tp_abort(SYN_J1939_Node *node, uint8_t da, uint8_t reason, uint32_t pgn)
{
    node->tx_id = syn_j1939_id_pack(7U, SYN_J1939_PGN_TP_CM, node->sa, da);
    node->tx_data[0] = SYN_J1939_TP_CTRL_ABORT;
    node->tx_data[1] = reason;
    node->tx_data[2] = 0xFFU;
    node->tx_data[3] = 0xFFU;
    node->tx_data[4] = 0xFFU;
    node->tx_data[5] = (uint8_t)(pgn & 0xFFU);
    node->tx_data[6] = (uint8_t)((pgn >> 8U) & 0xFFU);
    node->tx_data[7] = (uint8_t)((pgn >> 16U) & 0xFFU);
    node->tx_pending = true;
}

/**
 * @brief Initialize multi-packet TP RX state and emit initial CTS for unicast RTS sessions.
 * @param node J1939 node instance.
 * @param hdr Unpacked J1939 CAN identifier header.
 * @param frame Validated 8-byte TP.CM BAM or RTS CAN frame.
 * @param pgn Unpacked 24-bit parameter group number.
 * @param total_bytes Validated payload byte count (9..1785).
 * @param total_packets Validated TP.DT packet count.
 */
static void j1939_start_tp_rx(SYN_J1939_Node *node, const SYN_J1939_Header *hdr,
                              const SYN_CAN_Frame *frame, uint32_t pgn, uint16_t total_bytes,
                              uint8_t total_packets)
{
    node->tp_rx.active = true;
    node->tp_rx.is_bam = (frame->data[0] == SYN_J1939_TP_CTRL_BAM);
    node->tp_rx.sa = hdr->sa;
    node->tp_rx.da = hdr->da;
    node->tp_rx.pgn = pgn;
    node->tp_rx.total_bytes = total_bytes;
    node->tp_rx.total_packets = total_packets;
    node->tp_rx.next_seq = 1U;
    node->tp_rx.received_packets = 0U;

    if (!node->tp_rx.is_bam) {
        uint8_t max_cts = frame->data[4];
        uint8_t window = (max_cts < total_packets) ? max_cts : total_packets;
        node->tp_rx.cts_window = window;
        node->tp_rx.cts_remaining = window;
        j1939_queue_tp_cts(node, hdr->sa, window, 1U, pgn);
    }
}

/**
 * @brief Validate inbound TP.CM BAM or RTS addressing, session precedence, and window parameters.
 * @param node J1939 node instance.
 * @param hdr Unpacked J1939 CAN identifier header.
 * @param frame Received 8-byte TP.CM CAN frame.
 * @return true if control frame is accepted to start or abort-check an RX session.
 */
static bool j1939_tp_cm_accept(const SYN_J1939_Node *node, const SYN_J1939_Header *hdr,
                               const SYN_CAN_Frame *frame)
{
    uint8_t ctrl = frame->data[0];
    if (ctrl == SYN_J1939_TP_CTRL_BAM) {
        return (hdr->da == SYN_J1939_ADDR_GLOBAL) && (!node->tp_rx.active);
    }
    if (ctrl == SYN_J1939_TP_CTRL_RTS) {
        return (node->sa < SYN_J1939_ADDR_NULL) && (hdr->da == node->sa) && (frame->data[4] > 0U);
    }
    return false;
}

/**
 * @brief Handle inbound J1939 TP.CM connection management frame (BAM, RTS, or Abort).
 * @param node J1939 node instance.
 * @param hdr Unpacked J1939 CAN identifier header.
 * @param frame Received CAN frame.
 * @return SYN_BUSY while multi-packet session is in progress, SYN_INVALID_PARAM on bad DLC.
 */
static SYN_Status j1939_handle_tp_cm(SYN_J1939_Node *node, const SYN_J1939_Header *hdr,
                                     const SYN_CAN_Frame *frame)
{
    if (frame->dlc != 8U) {
        return SYN_INVALID_PARAM;
    }

    uint8_t ctrl = frame->data[0];
    if (ctrl == SYN_J1939_TP_CTRL_ABORT) {
        if (node->tp_rx.active && hdr->sa == node->tp_rx.sa && hdr->da == node->tp_rx.da) {
            node->tp_rx.active = false;
            node->tx_pending = false;
        }
        return SYN_BUSY;
    }

    if (!j1939_tp_cm_accept(node, hdr, frame)) {
        return SYN_BUSY;
    }

    uint32_t pgn = (uint32_t)syn_peek_u16_le(frame->data, 5U) | ((uint32_t)frame->data[7] << 16U);
    if (node->tp_rx.active) {
        j1939_queue_tp_abort(node, hdr->sa, 1U, pgn);
        return SYN_BUSY;
    }

    uint16_t total_bytes = syn_peek_u16_le(frame->data, 1U);
    uint8_t total_packets = frame->data[3];
    uint8_t expected_packets = (uint8_t)((total_bytes + 6U) / 7U);

    if (total_bytes >= 9U && total_bytes <= sizeof(node->tp_rx.data) &&
        total_packets == expected_packets) {
        j1939_start_tp_rx(node, hdr, frame, pgn, total_bytes, total_packets);
    }
    return SYN_BUSY;
}

/**
 * @brief Handle inbound J1939 TP.DT data transfer packet and reassemble multi-packet payload.
 * @param node J1939 node instance.
 * @param hdr Unpacked J1939 CAN identifier header.
 * @param frame Received CAN frame.
 * @param out_pgn Output reassembled PGN when transfer completes.
 * @param out_data Output pointer to reassembled payload buffer when complete.
 * @param out_len Output reassembled payload byte length when complete.
 * @return SYN_OK on final packet completion, SYN_BUSY if incomplete, SYN_INVALID_PARAM on bad DLC.
 */
static SYN_Status j1939_handle_tp_dt(SYN_J1939_Node *node, const SYN_J1939_Header *hdr,
                                     const SYN_CAN_Frame *frame, uint32_t *out_pgn,
                                     const uint8_t **out_data, size_t *out_len)
{
    if (frame->dlc != 8U) {
        return SYN_INVALID_PARAM;
    }
    if (!node->tp_rx.active || hdr->sa != node->tp_rx.sa || hdr->da != node->tp_rx.da) {
        return SYN_BUSY;
    }

    uint8_t seq = frame->data[0];
    if (seq != node->tp_rx.next_seq) {
        return SYN_BUSY;
    }

    size_t offset = (size_t)(seq - 1U) * 7U;
    size_t rem = (size_t)node->tp_rx.total_bytes - offset;
    size_t copy_len = (rem < 7U) ? rem : 7U;

    memcpy(&node->tp_rx.data[offset], &frame->data[1], copy_len);
    node->tp_rx.next_seq++;
    node->tp_rx.received_packets++;

    if (node->tp_rx.received_packets >= node->tp_rx.total_packets) {
        if (!node->tp_rx.is_bam) {
            j1939_queue_tp_ack(node, node->tp_rx.sa, node->tp_rx.total_bytes,
                               node->tp_rx.total_packets, node->tp_rx.pgn);
        }
        *out_pgn = node->tp_rx.pgn;
        *out_data = node->tp_rx.data;
        *out_len = node->tp_rx.total_bytes;
        node->tp_rx.active = false;
        return SYN_OK;
    }

    if (!node->tp_rx.is_bam) {
        node->tp_rx.cts_remaining--;
        if (node->tp_rx.cts_remaining == 0U) {
            uint8_t left = (uint8_t)(node->tp_rx.total_packets - node->tp_rx.received_packets);
            uint8_t next_win = (node->tp_rx.cts_window < left) ? node->tp_rx.cts_window : left;
            node->tp_rx.cts_remaining = next_win;
            j1939_queue_tp_cts(node, node->tp_rx.sa, next_win, node->tp_rx.next_seq,
                               node->tp_rx.pgn);
        }
    }

    return SYN_BUSY;
}

SYN_Status syn_j1939_process_frame(SYN_J1939_Node *node, const SYN_CAN_Frame *frame,
                                   uint32_t *out_pgn, const uint8_t **out_data, size_t *out_len)
{
    if (!node || !frame || !out_pgn || !out_data || !out_len || !frame->extended) {
        return SYN_INVALID_PARAM;
    }

    SYN_J1939_Header hdr;
    (void)syn_j1939_id_unpack(frame->id, &hdr);

    if (hdr.pgn == SYN_J1939_PGN_TP_CM) {
        return j1939_handle_tp_cm(node, &hdr, frame);
    }

    if (hdr.pgn == SYN_J1939_PGN_TP_DT) {
        return j1939_handle_tp_dt(node, &hdr, frame, out_pgn, out_data, out_len);
    }

    /* Single-frame packet handling */
    *out_pgn = hdr.pgn;
    *out_data = frame->data;
    *out_len = frame->dlc;

    return SYN_OK;
}

size_t syn_j1939_encode_dm2(uint8_t *buf, size_t buf_size, const SYN_J1939_DTC *dtc_list,
                            size_t dtc_count, uint8_t mil_lamp_status)
{
    return syn_j1939_encode_dm1(buf, buf_size, dtc_list, dtc_count, mil_lamp_status);
}

void syn_j1939_dtc_log_init(SYN_J1939_DTCLog *log)
{
    if (log != NULL) {
        (void)memset(log, 0, sizeof(*log));
    }
}

SYN_Status syn_j1939_dtc_add_active(SYN_J1939_DTCLog *log, uint32_t spn, uint8_t fmi)
{
    if (log == NULL) {
        return SYN_INVALID_PARAM;
    }

    /* Check if already active */
    for (size_t i = 0; i < log->active_count; i++) {
        if (log->active_dtcs[i].spn == spn && log->active_dtcs[i].fmi == fmi) {
            if (log->active_dtcs[i].occurrence_count < 126U) {
                log->active_dtcs[i].occurrence_count++;
            }
            return SYN_OK;
        }
    }

    if (log->active_count >= SYN_J1939_MAX_LOGGED_DTCS) {
        return SYN_ERROR;
    }

    log->active_dtcs[log->active_count].spn = spn;
    log->active_dtcs[log->active_count].fmi = fmi;
    log->active_dtcs[log->active_count].occurrence_count = 1;
    log->active_dtcs[log->active_count].conversion_method = 0;
    log->active_count++;

    return SYN_OK;
}

/**
 * @brief Record or merge a cleared active DTC into the DM2 previously-active table.
 * @param log Pointer to DTC log container.
 * @param dtc Cleared active DTC entry to merge or append.
 */
static void j1939_dtc_record_prev(SYN_J1939_DTCLog *log, const SYN_J1939_DTC *dtc)
{
    for (size_t k = 0U; k < log->prev_count; k++) {
        if ((log->prev_dtcs[k].spn == dtc->spn) && (log->prev_dtcs[k].fmi == dtc->fmi)) {
            uint16_t merged =
                (uint16_t)log->prev_dtcs[k].occurrence_count + (uint16_t)dtc->occurrence_count;
            log->prev_dtcs[k].occurrence_count = (merged > 126U) ? 126U : (uint8_t)merged;
            return;
        }
    }
    if (log->prev_count < SYN_J1939_MAX_LOGGED_DTCS) {
        log->prev_dtcs[log->prev_count] = *dtc;
        log->prev_count++;
    }
}

SYN_Status syn_j1939_dtc_clear_active(SYN_J1939_DTCLog *log, uint32_t spn, uint8_t fmi)
{
    if (log == NULL) {
        return SYN_INVALID_PARAM;
    }

    for (size_t i = 0; i < log->active_count; i++) {
        if (log->active_dtcs[i].spn == spn && log->active_dtcs[i].fmi == fmi) {
            j1939_dtc_record_prev(log, &log->active_dtcs[i]);
            /* Remove from active_dtcs */
            for (size_t j = i; j + 1 < log->active_count; j++) {
                log->active_dtcs[j] = log->active_dtcs[j + 1];
            }
            log->active_count--;
            return SYN_OK;
        }
    }

    return SYN_ERROR;
}

void syn_j1939_dtc_clear_dm3(SYN_J1939_DTCLog *log)
{
    if (log != NULL) {
        log->prev_count = 0;
        (void)memset(log->prev_dtcs, 0, sizeof(log->prev_dtcs));
    }
}

void syn_j1939_dtc_clear_dm11(SYN_J1939_DTCLog *log)
{
    if (log != NULL) {
        log->active_count = 0;
        (void)memset(log->active_dtcs, 0, sizeof(log->active_dtcs));
    }
}

#endif /* SYN_USE_J1939 */
