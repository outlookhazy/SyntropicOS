#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_CANOPEN) || SYN_USE_CANOPEN

/**
 * @file syn_canopen.c
 * @brief CANopen DS301 Slave Protocol Engine implementation.
 */

#include "syntropic/proto/syn_canopen.h"
#include "syntropic/util/syn_pack.h"

#include <string.h>

/**
 * @brief Helper function to queue a CAN transmit frame.
 *
 * @param node Pointer to node handle.
 * @param cob_id 11-bit CAN COB-ID.
 * @param data Payload data pointer.
 * @param len Payload length.
 * @return true if frame was queued, false if TX FIFO is full.
 */
static bool canopen_queue_tx(SYN_CANOpenNode *node, uint32_t cob_id, const uint8_t *data,
                             uint8_t len)
{
    if (node->tx_count >= SYN_CANOPEN_TX_QUEUE_DEPTH) {
        return false;
    }

    SYN_CANOpenTxSlot *slot = &node->tx_queue[node->tx_tail];
    slot->cob_id = cob_id;
    slot->len = len;
    (void)memcpy(slot->data, data, len);

    node->tx_tail = (uint8_t)((node->tx_tail + 1U) % SYN_CANOPEN_TX_QUEUE_DEPTH);
    node->tx_count++;
    if (node->tx_count == 1U) {
        node->tx_cob_id = cob_id;
        (void)memcpy(node->tx_data, data, len);
        node->tx_len = len;
    }
    node->tx_pending = true;
    return true;
}

/**
 * @brief Helper function to find an Object Dictionary entry.
 *
 * @param node Pointer to node handle.
 * @param index 16-bit OD Index.
 * @param subindex 8-bit OD Subindex.
 * @return Pointer to entry if found, NULL otherwise.
 */
static const SYN_CANOpenODEntry *canopen_find_od(const SYN_CANOpenNode *node, uint16_t index,
                                                 uint8_t subindex)
{
    if (node == NULL || node->od_table == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < node->od_count; i++) {
        if (node->od_table[i].index == index && node->od_table[i].subindex == subindex) {
            return &node->od_table[i];
        }
    }

    return NULL;
}

/**
 * @brief Send an SDO Abort frame.
 *
 * @param node Pointer to node handle.
 * @param index 16-bit OD Index.
 * @param subindex 8-bit OD Subindex.
 * @param abort_code 32-bit SDO Abort code.
 */
static void canopen_send_sdo_abort(SYN_CANOpenNode *node, uint16_t index, uint8_t subindex,
                                   uint32_t abort_code)
{
    uint8_t payload[8];
    size_t pos = 0;
    syn_pack_u8(payload, &pos, 0x80U);
    syn_pack_u16_le(payload, &pos, index);
    syn_pack_u8(payload, &pos, subindex);
    syn_pack_u32_le(payload, &pos, abort_code);

    canopen_queue_tx(node, 0x580U + node->node_id, payload, 8);
}

SYN_Status syn_canopen_init(SYN_CANOpenNode *node, const SYN_CANOpenNodeConfig *cfg,
                            const SYN_CANOpenODEntry *od_table, size_t od_count)
{
    if (node == NULL || cfg == NULL || od_table == NULL || od_count == 0) {
        return SYN_INVALID_PARAM;
    }

    if (cfg->node_id < 1U || cfg->node_id > 127U) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(node, 0, sizeof(*node));
    node->node_id = cfg->node_id;
    node->cfg = *cfg;
    node->od_table = od_table;
    node->od_count = od_count;
    node->nmt_state = SYN_CANOPEN_NMT_STATE_PREOP;

    /* Queue Bootup Message: COB-ID = 0x700 + NodeID, payload = 0x00 */
    uint8_t boot_payload[1] = {0x00U};
    canopen_queue_tx(node, 0x700U + node->node_id, boot_payload, 1);

    return SYN_OK;
}

SYN_Status syn_canopen_od_read(SYN_CANOpenNode *node, uint16_t index, uint8_t subindex, void *buf,
                               size_t buf_size, size_t *out_len)
{
    if (node == NULL || buf == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM;
    }

    const SYN_CANOpenODEntry *entry = canopen_find_od(node, index, subindex);
    if (entry == NULL) {
        return SYN_ERROR;
    }

    if (entry->access == SYN_CANOPEN_ACCESS_WO || entry->data_ptr == NULL) {
        return SYN_ERROR;
    }

    if (buf_size < entry->size) {
        return SYN_INVALID_PARAM;
    }

    (void)memcpy(buf, entry->data_ptr, entry->size);
    *out_len = entry->size;

    return SYN_OK;
}

SYN_Status syn_canopen_od_write(SYN_CANOpenNode *node, uint16_t index, uint8_t subindex,
                                const void *data, size_t len)
{
    if (node == NULL || data == NULL) {
        return SYN_INVALID_PARAM;
    }

    const SYN_CANOpenODEntry *entry = canopen_find_od(node, index, subindex);
    if (entry == NULL) {
        return SYN_ERROR;
    }

    if (entry->access == SYN_CANOPEN_ACCESS_RO || entry->data_ptr == NULL) {
        return SYN_ERROR;
    }

    if (len != entry->size) {
        return SYN_INVALID_PARAM;
    }

    (void)memcpy(entry->data_ptr, data, len);

    return SYN_OK;
}

/**
 * @brief Send SDO abort for a missing Object Dictionary index or subindex.
 * @param node     Pointer to node handle.
 * @param index    16-bit OD index.
 * @param subindex 8-bit OD subindex.
 */
static void canopen_abort_missing_od(SYN_CANOpenNode *node, uint16_t index, uint8_t subindex)
{
    for (size_t i = 0; i < node->od_count; i++) {
        if (node->od_table[i].index == index) {
            canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_SUBINDEX_NOT_EXIST);
            return;
        }
    }
    canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_NOT_EXIST);
}

/**
 * @brief Process an incoming NMT slave control frame (COB-ID 0x000).
 * @param node Pointer to node handle.
 * @param data 2-byte NMT command payload.
 */
static void canopen_handle_nmt(SYN_CANOpenNode *node, const uint8_t *data)
{
    uint8_t target_node = data[1];
    if (target_node != 0U && target_node != node->node_id) {
        return;
    }

    switch (data[0]) {
    case SYN_CANOPEN_NMT_CMD_START:
        node->nmt_state = SYN_CANOPEN_NMT_STATE_OPERATIONAL;
        break;
    case SYN_CANOPEN_NMT_CMD_STOP:
        node->nmt_state = SYN_CANOPEN_NMT_STATE_STOPPED;
        break;
    case SYN_CANOPEN_NMT_CMD_PREOP:
        node->nmt_state = SYN_CANOPEN_NMT_STATE_PREOP;
        break;
    case SYN_CANOPEN_NMT_CMD_RESET_NODE:
    case SYN_CANOPEN_NMT_CMD_RESET_COMM: {
        uint8_t boot_payload[1] = {0x00U};
        node->nmt_state = SYN_CANOPEN_NMT_STATE_PREOP;
        node->heartbeat_timer_ms = 0U;
        (void)memset(&node->sdo_session, 0, sizeof(node->sdo_session));
        (void)canopen_queue_tx(node, 0x700U + node->node_id, boot_payload, 1U);
        break;
    }
    default:
        break;
    }
}

/**
 * @brief Handle SDO Initiate Download (expedited or segmented) request.
 * @param node     Pointer to node handle.
 * @param cmd      SDO command specifier byte.
 * @param index    16-bit OD index.
 * @param subindex 8-bit OD subindex.
 * @param data     8-byte SDO request frame payload.
 */
static void canopen_sdo_download(SYN_CANOpenNode *node, uint8_t cmd, uint16_t index,
                                 uint8_t subindex, const uint8_t *data)
{
    const SYN_CANOpenODEntry *entry = canopen_find_od(node, index, subindex);
    if (entry == NULL) {
        canopen_abort_missing_od(node, index, subindex);
        return;
    }
    if (entry->access == SYN_CANOPEN_ACCESS_RO) {
        canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_READ_ONLY);
        return;
    }
    if (entry->data_ptr == NULL) {
        canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_UNSUPPORTED_ACCESS);
        return;
    }
    if ((cmd & 0x02U) != 0U) {
        uint8_t n = (cmd >> 2U) & 0x03U;
        size_t write_len = (size_t)(4U - n);
        if (write_len != entry->size) {
            canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_TYPE_MISMATCH);
            return;
        }
        (void)syn_canopen_od_write(node, index, subindex, &data[4], write_len);
        uint8_t resp[8] = {0x60U, data[1], data[2], data[3], 0U, 0U, 0U, 0U};
        canopen_queue_tx(node, 0x580U + node->node_id, resp, 8U);
        return;
    }

    node->sdo_session.state = SYN_CANOPEN_SDO_SEG_DOWNLOAD;
    node->sdo_session.index = index;
    node->sdo_session.subindex = subindex;
    node->sdo_session.toggle = 0U;
    node->sdo_session.transferred_bytes = 0U;
    node->sdo_session.total_bytes =
        (size_t)((uint32_t)data[4] | ((uint32_t)data[5] << 8U) | ((uint32_t)data[6] << 16U) |
                 ((uint32_t)data[7] << 24U));
    uint8_t resp[8] = {0x60U, data[1], data[2], data[3], 0U, 0U, 0U, 0U};
    canopen_queue_tx(node, 0x580U + node->node_id, resp, 8U);
}

/**
 * @brief Handle SDO Download Segment request.
 * @param node Pointer to node handle.
 * @param cmd  SDO segment command byte.
 * @param data 8-byte SDO segment frame payload.
 */
static void canopen_sdo_seg_download(SYN_CANOpenNode *node, uint8_t cmd, const uint8_t *data)
{
    uint8_t t = (cmd >> 4U) & 0x01U;
    if (t != node->sdo_session.toggle) {
        canopen_send_sdo_abort(node, node->sdo_session.index, node->sdo_session.subindex,
                               SYN_CANOPEN_SDO_ABORT_TOGGLE_BIT);
        node->sdo_session.state = SYN_CANOPEN_SDO_IDLE;
        return;
    }

    const SYN_CANOpenODEntry *entry =
        canopen_find_od(node, node->sdo_session.index, node->sdo_session.subindex);
    uint8_t n = (cmd >> 1U) & 0x07U;
    size_t seg_len = 7U - n;

    if (entry != NULL && entry->data_ptr != NULL &&
        (node->sdo_session.transferred_bytes + seg_len) <= entry->size) {
        (void)memcpy((uint8_t *)entry->data_ptr + node->sdo_session.transferred_bytes, &data[1],
                     seg_len);
        node->sdo_session.transferred_bytes += seg_len;

        uint8_t resp[8] = {(uint8_t)((t << 4U) | 0x20U), 0U, 0U, 0U, 0U, 0U, 0U, 0U};
        canopen_queue_tx(node, 0x580U + node->node_id, resp, 8U);
        node->sdo_session.toggle ^= 1U;

        if ((cmd & 0x01U) != 0U) {
            node->sdo_session.state = SYN_CANOPEN_SDO_IDLE;
        }
        return;
    }

    canopen_send_sdo_abort(node, node->sdo_session.index, node->sdo_session.subindex,
                           SYN_CANOPEN_SDO_ABORT_TYPE_MISMATCH);
    node->sdo_session.state = SYN_CANOPEN_SDO_IDLE;
}

/**
 * @brief Handle SDO Initiate Upload (expedited or segmented) request.
 * @param node     Pointer to node handle.
 * @param index    16-bit OD index.
 * @param subindex 8-bit OD subindex.
 * @param data     8-byte SDO request frame payload.
 */
static void canopen_sdo_upload(SYN_CANOpenNode *node, uint16_t index, uint8_t subindex,
                               const uint8_t *data)
{
    const SYN_CANOpenODEntry *entry = canopen_find_od(node, index, subindex);
    if (entry == NULL) {
        canopen_abort_missing_od(node, index, subindex);
        return;
    }
    if (entry->access == SYN_CANOPEN_ACCESS_WO) {
        canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_WRITE_ONLY);
        return;
    }
    if (entry->size <= 4U) {
        uint8_t read_buf[4] = {0};
        size_t read_len = 0U;
        if (syn_canopen_od_read(node, index, subindex, read_buf, sizeof(read_buf), &read_len) ==
                SYN_OK &&
            read_len <= 4U) {
            uint8_t n = (uint8_t)(4U - read_len);
            uint8_t resp[8] = {
                (uint8_t)(0x43U | (n << 2U)), data[1], data[2], data[3], 0U, 0U, 0U, 0U};
            (void)memcpy(&resp[4], read_buf, read_len);
            canopen_queue_tx(node, 0x580U + node->node_id, resp, 8U);
            return;
        }
        canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_TYPE_MISMATCH);
        return;
    }

    node->sdo_session.state = SYN_CANOPEN_SDO_SEG_UPLOAD;
    node->sdo_session.index = index;
    node->sdo_session.subindex = subindex;
    node->sdo_session.toggle = 0U;
    node->sdo_session.total_bytes = entry->size;
    node->sdo_session.transferred_bytes = 0U;

    uint32_t usize = (uint32_t)entry->size;
    uint8_t resp[8] = {0x41U,
                       data[1],
                       data[2],
                       data[3],
                       (uint8_t)(usize & 0xFFU),
                       (uint8_t)((usize >> 8U) & 0xFFU),
                       (uint8_t)((usize >> 16U) & 0xFFU),
                       (uint8_t)((usize >> 24U) & 0xFFU)};
    canopen_queue_tx(node, 0x580U + node->node_id, resp, 8U);
}

/**
 * @brief Handle SDO Upload Segment request.
 * @param node Pointer to node handle.
 * @param cmd  SDO segment command byte.
 */
static void canopen_sdo_seg_upload(SYN_CANOpenNode *node, uint8_t cmd)
{
    uint8_t t = (cmd >> 4U) & 0x01U;
    if (t != node->sdo_session.toggle) {
        canopen_send_sdo_abort(node, node->sdo_session.index, node->sdo_session.subindex,
                               SYN_CANOPEN_SDO_ABORT_TOGGLE_BIT);
        node->sdo_session.state = SYN_CANOPEN_SDO_IDLE;
        return;
    }

    const SYN_CANOpenODEntry *entry =
        canopen_find_od(node, node->sdo_session.index, node->sdo_session.subindex);
    if (entry == NULL || entry->data_ptr == NULL) {
        uint32_t code = (entry == NULL) ? SYN_CANOPEN_SDO_ABORT_NOT_EXIST
                                        : SYN_CANOPEN_SDO_ABORT_UNSUPPORTED_ACCESS;
        canopen_send_sdo_abort(node, node->sdo_session.index, node->sdo_session.subindex, code);
        node->sdo_session.state = SYN_CANOPEN_SDO_IDLE;
        return;
    }

    size_t rem = entry->size - node->sdo_session.transferred_bytes;
    size_t seg_len = (rem > 7U) ? 7U : rem;
    uint8_t n = (uint8_t)(7U - seg_len);
    uint8_t c = (rem <= 7U) ? 1U : 0U;

    uint8_t resp[8] = {0};
    resp[0] = (uint8_t)((t << 4U) | (n << 1U) | c);
    (void)memcpy(&resp[1], (const uint8_t *)entry->data_ptr + node->sdo_session.transferred_bytes,
                 seg_len);

    canopen_queue_tx(node, 0x580U + node->node_id, resp, 8U);
    node->sdo_session.transferred_bytes += seg_len;
    node->sdo_session.toggle ^= 1U;

    if (c != 0U) {
        node->sdo_session.state = SYN_CANOPEN_SDO_IDLE;
    }
}

/**
 * @brief Dispatch incoming SDO client request frame (COB-ID 0x600 + NodeID).
 * @param node Pointer to node handle.
 * @param data 8-byte SDO request frame payload.
 */
static void canopen_handle_sdo(SYN_CANOpenNode *node, const uint8_t *data)
{
    size_t sdo_pos = 0U;
    uint8_t cmd = syn_unpack_u8(data, &sdo_pos);
    uint16_t index = syn_unpack_u16_le(data, &sdo_pos);
    uint8_t subindex = syn_unpack_u8(data, &sdo_pos);
    uint8_t ccs = cmd & 0xE0U;

    if (ccs == 0x80U) {
        (void)memset(&node->sdo_session, 0, sizeof(node->sdo_session));
    } else if (ccs == 0x20U) {
        canopen_sdo_download(node, cmd, index, subindex, data);
    } else if (ccs == 0x00U && node->sdo_session.state == SYN_CANOPEN_SDO_SEG_DOWNLOAD) {
        canopen_sdo_seg_download(node, cmd, data);
    } else if (ccs == 0x40U) {
        canopen_sdo_upload(node, index, subindex, data);
    } else if (ccs == 0x60U && node->sdo_session.state == SYN_CANOPEN_SDO_SEG_UPLOAD) {
        canopen_sdo_seg_upload(node, cmd);
    }
}

SYN_Status syn_canopen_process_rx(SYN_CANOpenNode *node, uint32_t cob_id, const uint8_t *data,
                                  uint8_t len)
{
    if (node == NULL || data == NULL || len == 0U) {
        return SYN_INVALID_PARAM;
    }

    if (cob_id == 0x000U && len >= 2U) {
        canopen_handle_nmt(node, data);
        return SYN_OK;
    }

    if (cob_id == (0x600U + node->node_id) && len == 8U) {
        if (node->nmt_state == SYN_CANOPEN_NMT_STATE_STOPPED) {
            return SYN_OK;
        }
        if (node->tx_count >= SYN_CANOPEN_TX_QUEUE_DEPTH) {
            return SYN_BUSY;
        }
        canopen_handle_sdo(node, data);
        return SYN_OK;
    }

    if (node->nmt_state == SYN_CANOPEN_NMT_STATE_OPERATIONAL) {
        for (int i = 0; i < 4; i++) {
            if (node->cfg.rpdo[i].enabled != 0U && node->cfg.rpdo[i].cob_id == cob_id) {
                (void)syn_canopen_od_write(node, node->cfg.rpdo[i].od_index,
                                           node->cfg.rpdo[i].od_subindex, data, len);
                return SYN_OK;
            }
        }
    }

    return SYN_OK;
}

SYN_Status syn_canopen_update(SYN_CANOpenNode *node, uint32_t dt_ms)
{
    if (node == NULL) {
        return SYN_INVALID_PARAM;
    }

    /* Periodic Heartbeat Producer */
    if (node->cfg.heartbeat_ms > 0U) {
        uint64_t elapsed = (uint64_t)node->heartbeat_timer_ms + (uint64_t)dt_ms;
        if (elapsed >= (uint64_t)node->cfg.heartbeat_ms) {
            node->heartbeat_timer_ms = 0U;

            uint8_t hb_payload[1] = {node->nmt_state};
            (void)canopen_queue_tx(node, 0x700U + node->node_id, hb_payload, 1U);
        } else {
            node->heartbeat_timer_ms = (uint16_t)elapsed;
        }
    }

    /* Periodic TPDO Transmission (Operational State Only; reserve 2 urgent slots for EMCY/SDO) */
    if (node->nmt_state == SYN_CANOPEN_NMT_STATE_OPERATIONAL) {
        for (int i = 0; i < 4; i++) {
            if (node->cfg.tpdo[i].enabled != 0U &&
                node->tx_count < (SYN_CANOPEN_TX_QUEUE_DEPTH - 2U)) {
                uint8_t tpdo_buf[8] = {0};
                size_t tpdo_len = 0U;
                if (syn_canopen_od_read(node, node->cfg.tpdo[i].od_index,
                                        node->cfg.tpdo[i].od_subindex, tpdo_buf, sizeof(tpdo_buf),
                                        &tpdo_len) == SYN_OK) {
                    (void)canopen_queue_tx(node, node->cfg.tpdo[i].cob_id, tpdo_buf,
                                           (uint8_t)tpdo_len);
                }
            }
        }
    }

    return SYN_OK;
}

SYN_Status syn_canopen_send_emcy(SYN_CANOpenNode *node, uint16_t err_code, uint8_t err_reg)
{
    if (node == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t payload[8] = {0};
    payload[0] = (uint8_t)(err_code & 0xFFU);
    payload[1] = (uint8_t)((err_code >> 8U) & 0xFFU);
    payload[2] = err_reg;

    return canopen_queue_tx(node, 0x080U + node->node_id, payload, 8U) ? SYN_OK : SYN_BUSY;
}

bool syn_canopen_get_tx(SYN_CANOpenNode *node, uint32_t *out_cob_id, uint8_t *out_data,
                        uint8_t *out_len)
{
    if (node == NULL || node->tx_count == 0U) {
        return false;
    }

    const SYN_CANOpenTxSlot *slot = &node->tx_queue[node->tx_head];
    if (out_cob_id != NULL) {
        *out_cob_id = slot->cob_id;
    }
    if (out_data != NULL) {
        (void)memcpy(out_data, slot->data, slot->len);
    }
    if (out_len != NULL) {
        *out_len = slot->len;
    }

    node->tx_head = (uint8_t)((node->tx_head + 1U) % SYN_CANOPEN_TX_QUEUE_DEPTH);
    node->tx_count--;
    if (node->tx_count > 0U) {
        const SYN_CANOpenTxSlot *next = &node->tx_queue[node->tx_head];
        node->tx_cob_id = next->cob_id;
        (void)memcpy(node->tx_data, next->data, next->len);
        node->tx_len = next->len;
        node->tx_pending = true;
    } else {
        node->tx_pending = false;
    }

    return true;
}

SYN_Status syn_canopen_tpdo_trigger(SYN_CANOpenNode *node, uint8_t pdo_num)
{
    if (node == NULL || pdo_num < 1 || pdo_num > 4) {
        return SYN_INVALID_PARAM;
    }

    if (node->nmt_state != SYN_CANOPEN_NMT_STATE_OPERATIONAL) {
        return SYN_ERROR; /* TPDOs only transmitted in Operational state */
    }

    SYN_CANOpenPDOMap *map = &node->cfg.tpdo[pdo_num - 1];
    if (!map->enabled || map->cob_id == 0) {
        return SYN_ERROR; /* Unmapped */
    }

    uint8_t payload[8];
    size_t len = 0;
    SYN_Status st =
        syn_canopen_od_read(node, map->od_index, map->od_subindex, payload, sizeof(payload), &len);
    if (st == SYN_OK && len > 0) {
        return canopen_queue_tx(node, map->cob_id, payload, (uint8_t)len) ? SYN_OK : SYN_BUSY;
    }

    return SYN_ERROR;
}

#endif /* SYN_USE_CANOPEN */
