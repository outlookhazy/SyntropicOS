

# File syn\_canopen.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_canopen.c**](syn__canopen_8c.md)

[Go to the documentation of this file](syn__canopen_8c.md)


```C++

#include "syntropic/proto/syn_canopen.h"
#include "syntropic/util/syn_pack.h"
#include <string.h>

static void canopen_queue_tx(SYN_CANOpenNode *node, uint32_t cob_id, const uint8_t *data, uint8_t len)
{
    if (node == NULL) {
        return;
    }

    node->tx_cob_id = cob_id;
    (void)memcpy(node->tx_data, data, len);
    node->tx_len = len;
    node->tx_pending = true;
}

static const SYN_CANOpenODEntry *canopen_find_od(const SYN_CANOpenNode *node, uint16_t index, uint8_t subindex)
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

static void canopen_send_sdo_abort(SYN_CANOpenNode *node, uint16_t index, uint8_t subindex, uint32_t abort_code)
{
    uint8_t payload[8];
    size_t pos = 0;
    syn_pack_u8(payload, &pos, 0x80U);
    syn_pack_u16_le(payload, &pos, index);
    syn_pack_u8(payload, &pos, subindex);
    syn_pack_u32_le(payload, &pos, abort_code);

    canopen_queue_tx(node, 0x580U + node->node_id, payload, 8);
}

SYN_Status syn_canopen_init(SYN_CANOpenNode *node,
                            const SYN_CANOpenNodeConfig *cfg,
                            const SYN_CANOpenODEntry *od_table,
                            size_t od_count)
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
    uint8_t boot_payload[1] = { 0x00U };
    canopen_queue_tx(node, 0x700U + node->node_id, boot_payload, 1);

    return SYN_OK;
}

SYN_Status syn_canopen_od_read(SYN_CANOpenNode *node,
                               uint16_t index,
                               uint8_t subindex,
                               void *buf,
                               size_t buf_size,
                               size_t *out_len)
{
    if (node == NULL || buf == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM;
    }

    const SYN_CANOpenODEntry *entry = canopen_find_od(node, index, subindex);
    if (entry == NULL) {
        return SYN_ERROR;
    }

    if (entry->access == SYN_CANOPEN_ACCESS_WO) {
        return SYN_ERROR;
    }

    if (buf_size < entry->size) {
        return SYN_INVALID_PARAM;
    }

    (void)memcpy(buf, entry->data_ptr, entry->size);
    *out_len = entry->size;

    return SYN_OK;
}

SYN_Status syn_canopen_od_write(SYN_CANOpenNode *node,
                                uint16_t index,
                                uint8_t subindex,
                                const void *data,
                                size_t len)
{
    if (node == NULL || data == NULL) {
        return SYN_INVALID_PARAM;
    }

    const SYN_CANOpenODEntry *entry = canopen_find_od(node, index, subindex);
    if (entry == NULL) {
        return SYN_ERROR;
    }

    if (entry->access == SYN_CANOPEN_ACCESS_RO) {
        return SYN_ERROR;
    }

    if (len != entry->size) {
        return SYN_INVALID_PARAM;
    }

    (void)memcpy(entry->data_ptr, data, len);

    return SYN_OK;
}

SYN_Status syn_canopen_process_rx(SYN_CANOpenNode *node,
                                  uint32_t cob_id,
                                  const uint8_t *data,
                                  uint8_t len)
{
    if (node == NULL || data == NULL || len == 0) {
        return SYN_INVALID_PARAM;
    }

    /* 1. NMT Control Message: COB-ID = 0x000 */
    if (cob_id == 0x000U && len >= 2) {
        uint8_t target_node = data[1];
        if (target_node == 0U || target_node == node->node_id) {
            uint8_t cmd = data[0];
            switch (cmd) {
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
            case SYN_CANOPEN_NMT_CMD_RESET_COMM:
                node->nmt_state = SYN_CANOPEN_NMT_STATE_PREOP;
                {
                    uint8_t boot_payload[1] = { 0x00U };
                    canopen_queue_tx(node, 0x700U + node->node_id, boot_payload, 1);
                }
                break;
            default:
                break;
            }
        }
        return SYN_OK;
    }

    /* 2. SDO Request Server: COB-ID = 0x600 + NodeID */
    if (cob_id == (0x600U + node->node_id) && len == 8) {
        size_t sdo_pos = 0;
        uint8_t cmd = syn_unpack_u8(data, &sdo_pos);
        uint16_t index = syn_unpack_u16_le(data, &sdo_pos);
        uint8_t subindex = syn_unpack_u8(data, &sdo_pos);

        /* SDO Download (Write Request) */
        if ((cmd & 0xE0U) == 0x20U) {
            size_t write_len = 4;
            if ((cmd & 0x02U) != 0) { /* Expedited download */
                uint8_t n = (cmd >> 2) & 0x03U;
                write_len = (size_t)(4 - n);
            }

            const SYN_CANOpenODEntry *entry = canopen_find_od(node, index, subindex);
            if (entry == NULL) {
                const SYN_CANOpenODEntry *entry_idx = NULL;
                for (size_t i = 0; i < node->od_count; i++) {
                    if (node->od_table[i].index == index) {
                        entry_idx = &node->od_table[i];
                        break;
                    }
                }
                if (entry_idx == NULL) {
                    canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_NOT_EXIST);
                } else {
                    canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_SUBINDEX_NOT_EXIST);
                }
            } else if (entry->access == SYN_CANOPEN_ACCESS_RO) {
                canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_READ_ONLY);
            } else if (write_len != entry->size) {
                canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_TYPE_MISMATCH);
            } else {
                (void)syn_canopen_od_write(node, index, subindex, &data[4], write_len);
                uint8_t resp[8] = { 0x60U, data[1], data[2], data[3], 0, 0, 0, 0 };
                canopen_queue_tx(node, 0x580U + node->node_id, resp, 8);
            }
            return SYN_OK;
        }

        /* SDO Upload (Read Request) */
        if ((cmd & 0xE0U) == 0x40U) {
            const SYN_CANOpenODEntry *entry = canopen_find_od(node, index, subindex);
            if (entry == NULL) {
                const SYN_CANOpenODEntry *entry_idx = NULL;
                for (size_t i = 0; i < node->od_count; i++) {
                    if (node->od_table[i].index == index) {
                        entry_idx = &node->od_table[i];
                        break;
                    }
                }
                if (entry_idx == NULL) {
                    canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_NOT_EXIST);
                } else {
                    canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_SUBINDEX_NOT_EXIST);
                }
            } else if (entry->access == SYN_CANOPEN_ACCESS_WO) {
                canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_WRITE_ONLY);
            } else {
                uint8_t read_buf[4] = { 0 };
                size_t read_len = 0;
                if (syn_canopen_od_read(node, index, subindex, read_buf, sizeof(read_buf), &read_len) == SYN_OK && read_len <= 4) {
                    uint8_t n = (uint8_t)(4 - read_len);
                    uint8_t resp[8];
                    resp[0] = 0x43U | (uint8_t)(n << 2);
                    resp[1] = data[1];
                    resp[2] = data[2];
                    resp[3] = data[3];
                    (void)memcpy(&resp[4], read_buf, read_len);
                    if (read_len < 4) {
                        (void)memset(&resp[4 + read_len], 0, 4 - read_len);
                    }
                    canopen_queue_tx(node, 0x580U + node->node_id, resp, 8);
                } else {
                    canopen_send_sdo_abort(node, index, subindex, SYN_CANOPEN_SDO_ABORT_TYPE_MISMATCH);
                }
            }
            return SYN_OK;
        }
    }

    /* 3. RPDO Processing (Operational State Only) */
    if (node->nmt_state == SYN_CANOPEN_NMT_STATE_OPERATIONAL) {
        for (int i = 0; i < 4; i++) {
            if (node->cfg.rpdo[i].enabled && node->cfg.rpdo[i].cob_id == cob_id) {
                (void)syn_canopen_od_write(node, node->cfg.rpdo[i].od_index, node->cfg.rpdo[i].od_subindex, data, len);
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
    if (node->cfg.heartbeat_ms > 0) {
        node->heartbeat_timer_ms += (uint16_t)dt_ms;
        if (node->heartbeat_timer_ms >= node->cfg.heartbeat_ms) {
            node->heartbeat_timer_ms = 0;

            uint8_t hb_payload[1];
            hb_payload[0] = node->nmt_state;
            canopen_queue_tx(node, 0x700U + node->node_id, hb_payload, 1);
        }
    }

    /* Periodic TPDO Transmission (Operational State Only) */
    if (node->nmt_state == SYN_CANOPEN_NMT_STATE_OPERATIONAL) {
        for (int i = 0; i < 4; i++) {
            if (node->cfg.tpdo[i].enabled && !node->tx_pending) {
                uint8_t tpdo_buf[8] = { 0 };
                size_t tpdo_len = 0;
                if (syn_canopen_od_read(node, node->cfg.tpdo[i].od_index, node->cfg.tpdo[i].od_subindex, tpdo_buf, sizeof(tpdo_buf), &tpdo_len) == SYN_OK) {
                    canopen_queue_tx(node, node->cfg.tpdo[i].cob_id, tpdo_buf, (uint8_t)tpdo_len);
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

    uint8_t payload[8] = { 0 };
    payload[0] = (uint8_t)(err_code & 0xFFU);
    payload[1] = (uint8_t)((err_code >> 8) & 0xFFU);
    payload[2] = err_reg;

    canopen_queue_tx(node, 0x080U + node->node_id, payload, 8);

    return SYN_OK;
}

bool syn_canopen_get_tx(SYN_CANOpenNode *node,
                        uint32_t *out_cob_id,
                        uint8_t *out_data,
                        uint8_t *out_len)
{
    if (node == NULL || !node->tx_pending) {
        return false;
    }

    if (out_cob_id != NULL) {
        *out_cob_id = node->tx_cob_id;
    }
    if (out_data != NULL) {
        (void)memcpy(out_data, node->tx_data, node->tx_len);
    }
    if (out_len != NULL) {
        *out_len = node->tx_len;
    }

    node->tx_pending = false;

    return true;
}
```


