#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_ETHERCAT) || SYN_USE_ETHERCAT

/**
 * @file syn_ethercat.c
 * @brief Bare-metal EtherCAT Protocol Engine implementation.
 */

#include "../util/syn_assert.h"
#include "syn_ethercat.h"

#include <string.h>

/* ── Inline Helpers ─────────────────────────────────────────────────────── */

/**
 * @brief Store 16-bit unsigned integer in little-endian byte order.
 * @param p Pointer to output buffer.
 * @param val 16-bit integer value.
 */
static inline void store16_le(uint8_t *p, uint16_t val)
{
    p[0] = (uint8_t)(val & 0xFF);
    p[1] = (uint8_t)((val >> 8) & 0xFF);
}

/**
 * @brief Store 32-bit unsigned integer in little-endian byte order.
 * @param p Pointer to output buffer.
 * @param val 32-bit integer value.
 */
static inline void store32_le(uint8_t *p, uint32_t val)
{
    p[0] = (uint8_t)(val & 0xFF);
    p[1] = (uint8_t)((val >> 8) & 0xFF);
    p[2] = (uint8_t)((val >> 16) & 0xFF);
    p[3] = (uint8_t)((val >> 24) & 0xFF);
}

/**
 * @brief Load 16-bit unsigned integer from little-endian byte order.
 * @param p Pointer to input buffer.
 * @return 16-bit integer value.
 */
static inline uint16_t load16_le(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

/**
 * @brief Load 32-bit unsigned integer from little-endian byte order.
 * @param p Pointer to input buffer.
 * @return 32-bit integer value.
 */
static inline uint32_t load32_le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* ── Public API ─────────────────────────────────────────────────────────── */

void syn_ecat_frame_begin(uint8_t *buf, size_t buf_size)
{
    if (buf == NULL || buf_size < 2) {
        return;
    }
    uint16_t ecat_hdr = (uint16_t)((SYN_ETHERCAT_TYPE_DATAGRAM & 0x0F) << 12);
    store16_le(buf, ecat_hdr);
}

size_t syn_ecat_frame_add_datagram(uint8_t *buf, size_t buf_size, SYN_EcatCmd cmd, uint8_t idx,
                                   uint32_t addr, const uint8_t *data, uint16_t data_len, bool circ)
{
    if (buf == NULL || buf_size < 14U || data_len > SYN_ECAT_MAX_DGRAM_BYTES) {
        return 0;
    }

    uint16_t cur_hdr = load16_le(buf);
    size_t cur_dg_bytes = (size_t)(cur_hdr & SYN_ECAT_MAX_DGRAM_BYTES);
    size_t offset = 2U + cur_dg_bytes;
    size_t dg_total = 12U + (size_t)data_len;

    if ((cur_dg_bytes + dg_total) > SYN_ECAT_MAX_DGRAM_BYTES || (offset + dg_total) > buf_size) {
        return 0;
    }

    /* If there is a preceding datagram, set its 'm' (more datagrams) bit */
    if (cur_dg_bytes > 0U) {
        size_t pos = 2U;
        while (pos < offset) {
            uint16_t d_len = load16_le(&buf[pos + 6U]) & SYN_ECAT_MAX_DGRAM_BYTES;
            size_t d_size = 12U + (size_t)d_len;
            if (pos + d_size == offset) {
                uint16_t prev_flags = load16_le(&buf[pos + 6U]);
                prev_flags |= (1U << 15);
                store16_le(&buf[pos + 6U], prev_flags);
                break;
            }
            pos += d_size;
        }
    }

    buf[offset + 0U] = (uint8_t)cmd;
    buf[offset + 1U] = idx;
    store32_le(&buf[offset + 2U], addr);

    uint16_t len_flags = (uint16_t)(data_len & SYN_ECAT_MAX_DGRAM_BYTES);
    if (circ) {
        len_flags |= (1U << 14);
    }
    store16_le(&buf[offset + 6U], len_flags);
    store16_le(&buf[offset + 8U], 0U); /* irq */

    if (data_len > 0U) {
        if (data != NULL) {
            memcpy(&buf[offset + 10U], data, data_len);
        } else {
            memset(&buf[offset + 10U], 0, data_len);
        }
    }

    store16_le(&buf[offset + 10U + data_len], 0U); /* WKC = 0 on transmit */

    size_t new_dg_bytes = cur_dg_bytes + dg_total;
    uint16_t new_hdr = (uint16_t)(new_dg_bytes & SYN_ECAT_MAX_DGRAM_BYTES);
    new_hdr |= (uint16_t)((SYN_ETHERCAT_TYPE_DATAGRAM & 0x0FU) << 12);
    store16_le(buf, new_hdr);

    return 2U + new_dg_bytes;
}

size_t syn_ecat_frame_finalize(uint8_t *buf)
{
    if (buf == NULL) {
        return 0;
    }
    return 2 + (size_t)(load16_le(buf) & 0x07FF);
}

bool syn_ecat_frame_parse_next(const uint8_t *frame, size_t frame_len, size_t *offset,
                               SYN_EcatDatagramResult *out)
{
    if (frame == NULL || offset == NULL || out == NULL || frame_len < 14) {
        return false;
    }

    uint16_t ecat_hdr = load16_le(frame);
    uint16_t frame_type = (ecat_hdr >> 12) & 0x0F;
    if (frame_type != SYN_ETHERCAT_TYPE_DATAGRAM) {
        return false;
    }

    size_t total_dg_len = (size_t)(ecat_hdr & 0x07FF);
    if (frame_len < 2 + total_dg_len) {
        return false;
    }

    if (*offset < 2 || *offset >= 2 + total_dg_len) {
        return false;
    }

    if (*offset + 12 > 2 + total_dg_len) {
        return false;
    }

    out->cmd = frame[*offset + 0];
    out->idx = frame[*offset + 1];
    out->addr = load32_le(&frame[*offset + 2]);
    uint16_t len_flags = load16_le(&frame[*offset + 6]);
    out->data_len = len_flags & 0x07FF;

    if (*offset + 12 + (size_t)out->data_len > 2 + total_dg_len) {
        return false;
    }

    out->data = (out->data_len > 0) ? &frame[*offset + 10] : NULL;
    out->wkc = load16_le(&frame[*offset + 10 + (size_t)out->data_len]);

    *offset += 12 + (size_t)out->data_len;
    return true;
}

void syn_ecat_init(SYN_EcatNode *node, uint16_t station_addr, SYN_CANOpenNode *od)
{
    SYN_ASSERT(node != NULL);

    memset(node, 0, sizeof(*node));
    node->state = SYN_ECAT_STATE_INIT;
    node->target_state = SYN_ECAT_STATE_INIT;
    node->station_addr = station_addr;
    node->al_status = 0;
    node->wkc_expected = 1;
    node->od = od;
}

size_t syn_ecat_build_datagram_frame(uint8_t *buf, size_t buf_len, const SYN_EcatDatagram *dg,
                                     const uint8_t *data, uint16_t data_len)
{
    SYN_ASSERT(buf != NULL && dg != NULL);

    /* Total = 2 (EtherCAT Header) + 10 (Datagram Header) + data_len + 2 (WKC) */
    size_t dg_bytes = 12U + (size_t)data_len;
    size_t total_len = 2U + dg_bytes;
    if (dg_bytes > SYN_ECAT_MAX_DGRAM_BYTES || buf_len < total_len) {
        return 0;
    }

    /* 1. Pack 2-byte EtherCAT Header */
    uint16_t ecat_hdr = (uint16_t)(dg_bytes & SYN_ECAT_MAX_DGRAM_BYTES);
    ecat_hdr |= (uint16_t)((SYN_ETHERCAT_TYPE_DATAGRAM & 0x0FU) << 12);
    store16_le(buf, ecat_hdr);

    /* 2. Pack 10-byte Datagram Header */
    buf[2] = dg->cmd;
    buf[3] = dg->idx;
    store32_le(&buf[4], dg->addr);

    uint16_t len_flags = (uint16_t)(data_len & SYN_ECAT_MAX_DGRAM_BYTES);
    if (dg->m) {
        len_flags |= (1U << 15);
    }
    if (dg->circ) {
        len_flags |= (1U << 14);
    }
    store16_le(&buf[8], len_flags);

    store16_le(&buf[10], dg->irq);

    /* 3. Copy or zero-fill Data */
    if (data_len > 0U) {
        if (data != NULL) {
            memcpy(&buf[12], data, data_len);
        } else {
            memset(&buf[12], 0, data_len);
        }
    }

    /* 4. Pack Working Counter (WKC = 0 on transmit) */
    store16_le(&buf[12U + data_len], 0U);

    return total_len;
}

/**
 * @brief Read low ESC control/status register byte.
 * @param node EtherCAT slave node instance.
 * @param reg Physical ESC register offset.
 * @return Register byte value.
 */
static uint8_t ecat_slave_read_low_reg(const SYN_EcatNode *node, uint16_t reg)
{
    switch (reg) {
    case 0x0000U:
    case 0x0001U:
        return 0x01U;
    case 0x0010U:
        return (uint8_t)(node->station_addr & 0xFFU);
    case 0x0011U:
        return (uint8_t)((node->station_addr >> 8U) & 0xFFU);
    case 0x0120U:
        return (uint8_t)node->target_state;
    case 0x0130U:
        return (uint8_t)node->state;
    case 0x0134U:
        return (uint8_t)(node->al_status & 0xFFU);
    case 0x0135U:
        return (uint8_t)((node->al_status >> 8U) & 0xFFU);
    case 0x0502U:
        return (uint8_t)(node->sii_ctrl & 0xFFU);
    case 0x0503U:
        return (uint8_t)((node->sii_ctrl >> 8U) & 0x7FU);
    default:
        return 0x00U;
    }
}

/**
 * @brief Read serialized FMMU register byte (0x0600..0x063F).
 * @param node EtherCAT slave node instance.
 * @param reg Physical ESC register offset.
 * @return Serialized FMMU register byte.
 */
static uint8_t ecat_slave_read_fmmu_byte(const SYN_EcatNode *node, uint16_t reg)
{
    const SYN_EcatFMMUConfig *f = &node->fmmu[(reg - SYN_ESC_REG_FMMU0) / 16U];
    uint8_t b = (uint8_t)((reg - SYN_ESC_REG_FMMU0) % 16U);
    uint8_t raw[16] = {0};
    store32_le(&raw[0], f->logical_start);
    store16_le(&raw[4], f->length);
    raw[6] = f->logical_start_bit;
    raw[7] = f->logical_end_bit;
    store16_le(&raw[8], f->phys_start_addr);
    raw[10] = f->phys_start_bit;
    raw[11] = f->type;
    raw[12] = f->enable ? 1U : 0U;
    return raw[b];
}

/**
 * @brief Read serialized SyncManager register byte (0x0800..0x081F).
 * @param node EtherCAT slave node instance.
 * @param reg Physical ESC register offset.
 * @return Serialized SyncManager register byte.
 */
static uint8_t ecat_slave_read_sm_byte(const SYN_EcatNode *node, uint16_t reg)
{
    if (reg == 0x080DU) {
        return node->mbox_in_full ? 0x08U : 0x00U;
    }
    const SYN_EcatSMConfig *sm = &node->sm[(reg - SYN_ESC_REG_SM0) / 8U];
    uint8_t b = (uint8_t)((reg - SYN_ESC_REG_SM0) % 8U);
    uint8_t raw[8] = {0};
    store16_le(&raw[0], sm->start_addr);
    store16_le(&raw[2], sm->length);
    raw[4] = sm->control;
    raw[6] = sm->enable;
    return raw[b];
}

/**
 * @brief Read slave mailbox or PDO DPRAM byte (0x1000..0x117F).
 * @param node EtherCAT slave node instance.
 * @param reg Physical DPRAM offset.
 * @return Byte value at requested DPRAM address, or 0 if unmapped.
 */
static uint8_t ecat_slave_read_dpram_byte(const SYN_EcatNode *node, uint16_t reg)
{
    if (reg >= 0x1000U && reg < (0x1000U + SYN_ECAT_MBOX_SIZE)) {
        return node->mbox_out[reg - 0x1000U];
    }
    if (reg >= 0x1080U && reg < (0x1080U + SYN_ECAT_MBOX_SIZE)) {
        return node->mbox_in[reg - 0x1080U];
    }
    if (reg >= 0x1100U && reg < (0x1100U + SYN_ECAT_PDO_BUF_SIZE)) {
        return node->pdo_out[reg - 0x1100U];
    }
    if (reg >= 0x1140U && reg < (0x1140U + SYN_ECAT_PDO_BUF_SIZE)) {
        return node->pdo_in[reg - 0x1140U];
    }
    return 0x00U;
}

/**
 * @brief Read slave SII data, FMMU/SyncManager config, mailbox, or PDO DPRAM byte.
 * @param node EtherCAT slave node instance.
 * @param reg Physical ESC register or DPRAM offset.
 * @return Byte value at requested ESC address.
 */
static uint8_t ecat_slave_read_esc_byte(const SYN_EcatNode *node, uint16_t reg)
{
    if (reg < 0x0508U) {
        return ecat_slave_read_low_reg(node, reg);
    }
    if (reg <= 0x050BU) {
        uint32_t w = node->sii_words[node->sii_addr & (SYN_ECAT_SII_WORDS - 1U)];
        uint8_t shift = (uint8_t)((reg - 0x0508U) * 8U);
        return (uint8_t)((w >> shift) & 0xFFU);
    }
    if (reg >= SYN_ESC_REG_FMMU0 && reg < (SYN_ESC_REG_FMMU0 + 64U)) {
        return ecat_slave_read_fmmu_byte(node, reg);
    }
    if (reg >= SYN_ESC_REG_SM0 && reg < (SYN_ESC_REG_SM0 + 32U)) {
        return ecat_slave_read_sm_byte(node, reg);
    }
    return ecat_slave_read_dpram_byte(node, reg);
}

/**
 * @brief Process CoE SDO request written into slave SM0 mailbox (0x1000).
 * @param node EtherCAT slave node instance.
 */
static void ecat_slave_handle_coe_sdo(SYN_EcatNode *node)
{
    uint16_t coe_hdr = load16_le(&node->mbox_out[6]);
    uint8_t coe_type = (uint8_t)((coe_hdr >> 12U) & 0x0FU);
    if ((node->mbox_out[5] & 0x0FU) != 0x03U || coe_type != SYN_ECAT_COE_TYPE_SDO_REQ) {
        return;
    }

    uint8_t sdo_cmd = node->mbox_out[8];
    uint16_t idx = load16_le(&node->mbox_out[9]);
    uint8_t sub = node->mbox_out[11];

    memset(node->mbox_in, 0, sizeof(node->mbox_in));
    store16_le(&node->mbox_in[0], 10U);
    store16_le(&node->mbox_in[2], node->station_addr);
    node->mbox_in[5] = 0x03U;
    store16_le(&node->mbox_in[6], (uint16_t)((SYN_ECAT_COE_TYPE_SDO_RESP & 0x0FU) << 12U));
    store16_le(&node->mbox_in[9], idx);
    node->mbox_in[11] = sub;

#if SYN_ECAT_COE_OD_ENABLED
    if ((sdo_cmd & 0xE0U) == 0x20U && node->od != NULL) {
        size_t dlen = ((sdo_cmd & 0x01U) != 0U) ? (size_t)(4U - ((sdo_cmd >> 2U) & 0x03U)) : 4U;
        if (syn_canopen_od_write(node->od, idx, sub, &node->mbox_out[12], dlen) == SYN_OK) {
            node->mbox_in[8] = 0x60U;
            node->mbox_in_full = true;
            return;
        }
    } else if ((sdo_cmd & 0xE0U) == 0x40U && node->od != NULL) {
        uint8_t rbuf[4] = {0};
        size_t rlen = 0U;
        if (syn_canopen_od_read(node->od, idx, sub, rbuf, sizeof(rbuf), &rlen) == SYN_OK &&
            rlen >= 1U) {
            node->mbox_in[8] = (uint8_t)(0x43U | (((4U - rlen) & 0x03U) << 2U));
            memcpy(&node->mbox_in[12], rbuf, rlen);
            node->mbox_in_full = true;
            return;
        }
    }
#else
    (void)sdo_cmd;
#endif

    node->mbox_in[8] = 0x80U;
    store32_le(&node->mbox_in[12], 0x06020000UL);
    node->mbox_in_full = true;
}

/**
 * @brief Write slave FMMU (0x0600..0x063F) or SyncManager (0x0800..0x081F) register block.
 * @param node EtherCAT slave node instance.
 * @param phys_addr Physical ESC register start address.
 * @param src Source data bytes.
 * @param len Number of bytes to write.
 */
static void ecat_slave_write_fmmu_sm(SYN_EcatNode *node, uint16_t phys_addr, const uint8_t *src,
                                     uint16_t len)
{
    if (phys_addr >= SYN_ESC_REG_FMMU0 && phys_addr < (SYN_ESC_REG_FMMU0 + 64U) && len == 16U) {
        uint8_t idx = (uint8_t)((phys_addr - SYN_ESC_REG_FMMU0) / 16U);
        SYN_EcatFMMUConfig *f = &node->fmmu[idx];
        f->logical_start = load32_le(&src[0]);
        f->length = load16_le(&src[4]);
        f->logical_start_bit = src[6];
        f->logical_end_bit = src[7];
        f->phys_start_addr = load16_le(&src[8]);
        f->phys_start_bit = src[10];
        f->type = src[11];
        f->enable = (src[12] != 0U);
        return;
    }
    if (phys_addr >= SYN_ESC_REG_SM0 && phys_addr < (SYN_ESC_REG_SM0 + 32U) && len == 8U) {
        uint8_t idx = (uint8_t)((phys_addr - SYN_ESC_REG_SM0) / 8U);
        SYN_EcatSMConfig *sm = &node->sm[idx];
        sm->start_addr = load16_le(&src[0]);
        sm->length = load16_le(&src[2]);
        sm->control = src[4];
        sm->enable = src[6];
    }
}

/**
 * @brief Write slave mailbox DPRAM, output PDO buffer, or FMMU/SM configuration registers.
 * @param node EtherCAT slave node instance.
 * @param phys_addr Physical ESC address.
 * @param src Source data bytes.
 * @param len Number of bytes to write.
 */
static void ecat_slave_write_dpram(SYN_EcatNode *node, uint16_t phys_addr, const uint8_t *src,
                                   uint16_t len)
{
    if (phys_addr >= 0x1000U && phys_addr < (0x1000U + SYN_ECAT_MBOX_SIZE)) {
        uint16_t off = (uint16_t)(phys_addr - 0x1000U);
        uint16_t avail = (uint16_t)(SYN_ECAT_MBOX_SIZE - off);
        uint16_t copy_len = (len > avail) ? avail : len;
        memcpy(&node->mbox_out[off], src, copy_len);
        if (off == 0U && copy_len >= 12U) {
            ecat_slave_handle_coe_sdo(node);
        }
        return;
    }
    if (phys_addr >= 0x1100U && phys_addr < (0x1100U + SYN_ECAT_PDO_BUF_SIZE)) {
        uint16_t off = (uint16_t)(phys_addr - 0x1100U);
        uint16_t avail = (uint16_t)(SYN_ECAT_PDO_BUF_SIZE - off);
        uint16_t copy_len = (len > avail) ? avail : len;
        memcpy(&node->pdo_out[off], src, copy_len);
        node->rx_pdos++;
        return;
    }
    ecat_slave_write_fmmu_sm(node, phys_addr, src, len);
}

/**
 * @brief Write bytes to slave ESC physical register or DPRAM space.
 * @param node EtherCAT slave node instance.
 * @param phys_addr Physical ESC start address.
 * @param src Source data bytes.
 * @param len Number of bytes to write.
 */
static void ecat_slave_phys_write(SYN_EcatNode *node, uint16_t phys_addr, const uint8_t *src,
                                  uint16_t len)
{
    if (phys_addr == SYN_ESC_REG_STATION_ADDR && len >= 2U) {
        node->station_addr = load16_le(src);
        return;
    }
    if (phys_addr == SYN_ESC_REG_AL_CONTROL && len >= 1U) {
        (void)syn_ecat_set_state(node, (SYN_EcatState)(src[0] & 0x0FU));
        syn_ecat_update(node);
        return;
    }
    if (phys_addr == SYN_ESC_REG_SII_EEPROM_CTRL && len >= 2U) {
        node->sii_ctrl = (uint16_t)(load16_le(src) & 0x7FFFU);
        if (len >= 6U) {
            node->sii_addr = load32_le(&src[2]);
        }
        return;
    }
    if (phys_addr == 0x0504U && len >= 4U) {
        node->sii_addr = load32_le(src);
        return;
    }
    ecat_slave_write_dpram(node, phys_addr, src, len);
}

/**
 * @brief Read bytes from slave ESC physical register or DPRAM space.
 * @param node EtherCAT slave node instance.
 * @param phys_addr Physical ESC start address.
 * @param dst Destination buffer.
 * @param len Number of bytes to read.
 */
static void ecat_slave_phys_read(SYN_EcatNode *node, uint16_t phys_addr, uint8_t *dst, uint16_t len)
{
    for (uint16_t i = 0U; i < len; i++) {
        dst[i] = ecat_slave_read_esc_byte(node, (uint16_t)(phys_addr + i));
    }
    if (phys_addr == 0x1080U && len > 0U) {
        node->mbox_in_full = false;
    }
    if (phys_addr >= 0x1140U && phys_addr < (0x1140U + SYN_ECAT_PDO_BUF_SIZE) && len > 0U) {
        node->tx_pdos++;
    }
}

/**
 * @brief Execute physical Read (1), Write (2), or ReadWrite (3) and return WKC delta.
 * @param node EtherCAT slave node instance.
 * @param mode Access mode (1 = read, 2 = write, 3 = read-write).
 * @param reg Physical ESC start address.
 * @param data In-place datagram payload buffer.
 * @param len Payload length in bytes.
 * @return Working counter increment for the operation.
 */
static uint16_t ecat_slave_exec_phys_rw(SYN_EcatNode *node, uint8_t mode, uint16_t reg,
                                        uint8_t *data, uint16_t len)
{
    if (mode == 1U) {
        ecat_slave_phys_read(node, reg, data, len);
        return 1U;
    }
    if (mode == 2U) {
        ecat_slave_phys_write(node, reg, data, len);
        return 1U;
    }
    uint8_t prev[SYN_ECAT_PDO_BUF_SIZE];
    uint16_t off = 0U;
    while (off < len) {
        uint16_t rem = (uint16_t)(len - off);
        uint16_t cp = (rem > SYN_ECAT_PDO_BUF_SIZE) ? SYN_ECAT_PDO_BUF_SIZE : rem;
        uint16_t cur_reg = (uint16_t)(reg + off);
        ecat_slave_phys_read(node, cur_reg, prev, cp);
        ecat_slave_phys_write(node, cur_reg, &data[off], cp);
        memcpy(&data[off], prev, cp);
        off = (uint16_t)(off + cp);
    }
    return 3U;
}

/**
 * @brief Map overlapping FMMU region for LRD/LWR/LRW datagrams.
 * @param node EtherCAT slave node instance.
 * @param f FMMU entry configuration.
 * @param cmd EtherCAT logical datagram command code.
 * @param log_addr Logical start address of datagram.
 * @param data In-place datagram payload buffer.
 * @param len Datagram payload length in bytes.
 * @param did_rd Set true if a logical read occurred.
 * @param did_wr Set true if a logical write occurred.
 */
static void ecat_slave_apply_fmmu(SYN_EcatNode *node, const SYN_EcatFMMUConfig *f, uint8_t cmd,
                                  uint32_t log_addr, uint8_t *data, uint16_t len, bool *did_rd,
                                  bool *did_wr)
{
    uint64_t req_end = (uint64_t)log_addr + (uint64_t)len;
    uint64_t fmmu_end = (uint64_t)f->logical_start + (uint64_t)f->length;
    uint64_t ov_start =
        (log_addr > f->logical_start) ? (uint64_t)log_addr : (uint64_t)f->logical_start;
    uint64_t ov_end = (req_end < fmmu_end) ? req_end : fmmu_end;
    if (ov_start >= ov_end) {
        return;
    }

    uint16_t slice_len = (uint16_t)(ov_end - ov_start);
    uint16_t dg_off = (uint16_t)(ov_start - (uint64_t)log_addr);
    uint16_t phys =
        (uint16_t)(f->phys_start_addr + (uint16_t)(ov_start - (uint64_t)f->logical_start));
    uint8_t wr_buf[SYN_ECAT_PDO_BUF_SIZE];

    bool do_wr = (cmd != SYN_ECAT_CMD_LRD) && ((f->type & 0x02U) != 0U);
    bool do_rd = (cmd != SYN_ECAT_CMD_LWR) && ((f->type & 0x01U) != 0U);

    uint16_t off = 0U;
    while (off < slice_len) {
        uint16_t rem = (uint16_t)(slice_len - off);
        uint16_t cp = (rem > SYN_ECAT_PDO_BUF_SIZE) ? SYN_ECAT_PDO_BUF_SIZE : rem;
        uint16_t cur_dg = (uint16_t)(dg_off + off);
        uint16_t cur_phys = (uint16_t)(phys + off);
        if (do_wr) {
            memcpy(wr_buf, &data[cur_dg], cp);
            *did_wr = true;
        }
        if (do_rd) {
            ecat_slave_phys_read(node, cur_phys, &data[cur_dg], cp);
            *did_rd = true;
        }
        if (do_wr) {
            ecat_slave_phys_write(node, cur_phys, wr_buf, cp);
        }
        off = (uint16_t)(off + cp);
    }
}

/**
 * @brief Process logical datagram (LRD/LWR/LRW) across all enabled FMMUs.
 * @param node EtherCAT slave node instance.
 * @param cmd EtherCAT logical datagram command code.
 * @param log_addr Logical start address of datagram.
 * @param data In-place datagram payload buffer.
 * @param len Datagram payload length in bytes.
 * @return Working counter increment for logical access.
 */
static uint16_t ecat_slave_process_logical(SYN_EcatNode *node, uint8_t cmd, uint32_t log_addr,
                                           uint8_t *data, uint16_t len)
{
    bool did_rd = false;
    bool did_wr = false;
    for (uint8_t i = 0U; i < 4U; i++) {
        if (node->fmmu[i].enable && node->fmmu[i].length > 0U) {
            ecat_slave_apply_fmmu(node, &node->fmmu[i], cmd, log_addr, data, len, &did_rd, &did_wr);
        }
    }
    if (cmd == SYN_ECAT_CMD_LRW) {
        return (uint16_t)((did_rd ? 1U : 0U) + (did_wr ? 2U : 0U));
    }
    return (did_rd || did_wr) ? 1U : 0U;
}

/**
 * @brief Dispatch one slave datagram in-place and return updated datagram WKC.
 * @param node EtherCAT slave node instance.
 * @param dg Pointer to start of datagram header in EtherCAT frame.
 * @param d_len Datagram payload length in bytes.
 * @return Updated working counter stored in datagram trailer.
 */
static uint16_t ecat_slave_process_datagram(SYN_EcatNode *node, uint8_t *dg, uint16_t d_len)
{
    uint8_t cmd = dg[0];
    uint32_t addr = load32_le(&dg[2]);
    uint16_t reg = (uint16_t)(addr & 0xFFFFU);
    uint8_t *payload = &dg[10];
    uint16_t dg_wkc = load16_le(&dg[10U + d_len]);
    uint16_t delta = 0U;

    if (cmd >= SYN_ECAT_CMD_APRD && cmd <= SYN_ECAT_CMD_APRW) {
        int16_t pos = (int16_t)(uint16_t)(addr >> 16U);
        store16_le(&dg[4], (uint16_t)(pos + 1));
        if (pos == 0) {
            delta = ecat_slave_exec_phys_rw(node, (uint8_t)(cmd - SYN_ECAT_CMD_APRD + 1U), reg,
                                            payload, d_len);
        }
    } else if (cmd >= SYN_ECAT_CMD_FPRD && cmd <= SYN_ECAT_CMD_FPRW) {
        if (node->station_addr != 0U && (uint16_t)(addr >> 16U) == node->station_addr) {
            delta = ecat_slave_exec_phys_rw(node, (uint8_t)(cmd - SYN_ECAT_CMD_FPRD + 1U), reg,
                                            payload, d_len);
        }
    } else if (cmd >= SYN_ECAT_CMD_BRD && cmd <= SYN_ECAT_CMD_BRW) {
        delta = ecat_slave_exec_phys_rw(node, (uint8_t)(cmd - SYN_ECAT_CMD_BRD + 1U), reg, payload,
                                        d_len);
    } else if (cmd >= SYN_ECAT_CMD_LRD && cmd <= SYN_ECAT_CMD_LRW) {
        delta = ecat_slave_process_logical(node, cmd, addr, payload, d_len);
    }

    dg_wkc = (uint16_t)(dg_wkc + delta);
    store16_le(&dg[10U + d_len], dg_wkc);
    return dg_wkc;
}

/**
 * @brief Validate EtherCAT datagram frame header and return payload end offset.
 * @param frame Raw EtherCAT frame buffer.
 * @param len Frame length in bytes.
 * @param out_end Output end offset of nested datagrams.
 * @return true if frame header is valid, false otherwise.
 */
static bool ecat_validate_frame_hdr(const uint8_t *frame, size_t len, size_t *out_end)
{
    if (len < 14U) {
        return false;
    }

    uint16_t ecat_hdr = load16_le(frame);
    uint16_t frame_type = (uint16_t)((ecat_hdr >> 12U) & 0x0FU);
    uint16_t dg_len = (uint16_t)(ecat_hdr & SYN_ECAT_MAX_DGRAM_BYTES);
    if (frame_type != SYN_ETHERCAT_TYPE_DATAGRAM || dg_len < 12U || len < (size_t)(2U + dg_len)) {
        return false;
    }

    *out_end = 2U + (size_t)dg_len;
    return true;
}

SYN_Status syn_ecat_parse_frame(SYN_EcatNode *node, const uint8_t *frame, size_t len, uint16_t *wkc)
{
    SYN_ASSERT(node != NULL && frame != NULL);

    size_t end = 0U;
    if (!ecat_validate_frame_hdr(frame, len, &end)) {
        return SYN_ERROR;
    }

    size_t pos = 2U;
    uint16_t total_wkc = 0U;

    while (pos < end) {
        if ((pos + 12U) > end) {
            return SYN_ERROR;
        }
        uint16_t d_len = (uint16_t)(load16_le(&frame[pos + 6U]) & SYN_ECAT_MAX_DGRAM_BYTES);
        if ((pos + 12U + (size_t)d_len) > end) {
            return SYN_ERROR;
        }
        total_wkc = (uint16_t)(total_wkc + load16_le(&frame[pos + 10U + d_len]));
        pos += 12U + (size_t)d_len;
    }

    if (wkc != NULL) {
        *wkc = total_wkc;
    }
    node->wkc_last = total_wkc;

    return (total_wkc < node->wkc_expected) ? SYN_ERROR : SYN_OK;
}

SYN_Status syn_ecat_slave_process_frame(SYN_EcatNode *node, uint8_t *frame, size_t len,
                                        uint16_t *wkc)
{
    SYN_ASSERT(node != NULL && frame != NULL);

    size_t end = 0U;
    if (!ecat_validate_frame_hdr(frame, len, &end)) {
        return SYN_ERROR;
    }

    size_t pos = 2U;
    uint16_t total_wkc = 0U;

    while (pos < end) {
        if ((pos + 12U) > end) {
            return SYN_ERROR;
        }
        uint16_t d_len = (uint16_t)(load16_le(&frame[pos + 6U]) & SYN_ECAT_MAX_DGRAM_BYTES);
        if ((pos + 12U + (size_t)d_len) > end) {
            return SYN_ERROR;
        }
        total_wkc = (uint16_t)(total_wkc + ecat_slave_process_datagram(node, &frame[pos], d_len));
        pos += 12U + (size_t)d_len;
    }

    if (wkc != NULL) {
        *wkc = total_wkc;
    }
    node->wkc_last = total_wkc;

    return (total_wkc < node->wkc_expected) ? SYN_ERROR : SYN_OK;
}

SYN_Status syn_ecat_set_state(SYN_EcatNode *node, SYN_EcatState new_state)
{
    SYN_ASSERT(node != NULL);

    switch (new_state) {
    case SYN_ECAT_STATE_INIT:
    case SYN_ECAT_STATE_PREOP:
    case SYN_ECAT_STATE_BOOT:
    case SYN_ECAT_STATE_SAFEOP:
    case SYN_ECAT_STATE_OP:
        node->target_state = new_state;
        return SYN_OK;
    default:
        return SYN_ERROR;
    }
}

void syn_ecat_update(SYN_EcatNode *node)
{
    SYN_ASSERT(node != NULL);

    if (node->state != node->target_state) {
        /* Simple valid transition logic */
        node->state = node->target_state;
        node->al_status = 0;
    }
}

size_t syn_ecat_coe_encode_sdo_download(uint8_t *buf, size_t buf_len, uint16_t index,
                                        uint8_t subindex, const void *data, size_t data_len)
{
    SYN_ASSERT(buf != NULL);

    if (data == NULL || data_len == 0U || data_len > 4U || buf_len < 10U) {
        return 0;
    }

    /* CoE Mailbox Header: Type 2 (SDO Request) */
    uint16_t coe_hdr = (uint16_t)((SYN_ECAT_COE_TYPE_SDO_REQ & 0x0FU) << 12);
    store16_le(buf, coe_hdr);

    /* SDO Download Command Header: Expedited SDO Download Request (e=1, s=1) */
    uint8_t sdo_hdr = 0x22U | (uint8_t)(((4U - data_len) & 0x03U) << 2) | 0x01U;
    buf[2] = sdo_hdr;
    store16_le(&buf[3], index);
    buf[5] = subindex;

    /* SDO Data */
    memset(&buf[6], 0, 4U);
    memcpy(&buf[6], data, data_len);

    return 10U;
}

size_t syn_ecat_coe_encode_sdo_upload(uint8_t *buf, size_t buf_len, uint16_t index,
                                      uint8_t subindex)
{
    SYN_ASSERT(buf != NULL);

    if (buf_len < 10U) {
        return 0;
    }

    /* CoE Mailbox Header: Type 2 (SDO Request) */
    uint16_t coe_hdr = (uint16_t)((SYN_ECAT_COE_TYPE_SDO_REQ & 0x0FU) << 12);
    store16_le(buf, coe_hdr);

    /* SDO Upload Request Command Header: 0x40 */
    buf[2] = 0x40U;
    store16_le(&buf[3], index);
    buf[5] = subindex;
    memset(&buf[6], 0, 4U);

    return 10U;
}

/* ── EtherCAT Master API Implementation ─────────────────────────────────── */

SYN_Status syn_ecat_master_init(SYN_EcatMaster *m, uint8_t *tx_buf, uint16_t tx_buf_size,
                                uint8_t *rx_buf, uint16_t rx_buf_size, uint8_t *output_image,
                                uint16_t output_size, uint8_t *input_image, uint16_t input_size)
{
    if (m == NULL || tx_buf == NULL || rx_buf == NULL || tx_buf_size < 64 || rx_buf_size < 64) {
        return SYN_INVALID_PARAM;
    }

    memset(m, 0, sizeof(*m));
    m->tx_buf = tx_buf;
    m->tx_buf_size = tx_buf_size;
    m->rx_buf = rx_buf;
    m->rx_buf_size = rx_buf_size;
    m->output_image = output_image;
    m->output_size = output_size;
    m->input_image = input_image;
    m->input_size = input_size;

    return SYN_OK;
}

size_t syn_ecat_encode_scan_bus(SYN_EcatMaster *m)
{
    if (m == NULL || m->tx_buf == NULL) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    uint8_t dummy[2] = {0, 0};
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_BRD, 0x01,
                                       SYN_ESC_REG_TYPE, dummy, 2, false);
}

uint8_t syn_ecat_decode_scan_bus(SYN_EcatMaster *m, size_t rx_len)
{
    if (m == NULL || m->rx_buf == NULL || rx_len < 14) {
        return 0;
    }

    size_t offset = 2;
    SYN_EcatDatagramResult res;
    if (syn_ecat_frame_parse_next(m->rx_buf, rx_len, &offset, &res)) {
        uint8_t count =
            (res.wkc > SYN_ECAT_MAX_SLAVES) ? (uint8_t)SYN_ECAT_MAX_SLAVES : (uint8_t)res.wkc;
        m->slave_count = count;
        return count;
    }

    return 0;
}

size_t syn_ecat_encode_assign_addr(SYN_EcatMaster *m, uint16_t position, uint16_t station_addr)
{
    if (m == NULL || m->tx_buf == NULL) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    uint8_t payload[2];
    store16_le(payload, station_addr);

    /* Position auto-increment addressing format: 0 - position in high 16 bits */
    uint16_t auto_inc = (uint16_t)(-(int16_t)position);
    uint32_t addr = ((uint32_t)auto_inc << 16) | SYN_ESC_REG_STATION_ADDR;

    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_APWR,
                                       (uint8_t)position, addr, payload, 2, false);
}

size_t syn_ecat_encode_read_sii(SYN_EcatMaster *m, uint16_t station_addr, uint16_t word_offset)
{
    if (m == NULL || m->tx_buf == NULL) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    uint32_t addr = ((uint32_t)station_addr << 16) | SYN_ESC_REG_SII_EEPROM_CTRL;
    uint8_t payload[6];
    store16_le(&payload[0], 0x0100); /* Read 4-byte command */
    store32_le(&payload[2], (uint32_t)word_offset);

    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, 0x02, addr,
                                       payload, 6, false);
}

SYN_Status syn_ecat_decode_read_sii(SYN_EcatMaster *m, size_t rx_len, uint32_t *out_val)
{
    if (m == NULL || m->rx_buf == NULL || out_val == NULL || rx_len < 14U) {
        return SYN_INVALID_PARAM;
    }

    size_t offset = 2U;
    SYN_EcatDatagramResult res;
    if (syn_ecat_frame_parse_next(m->rx_buf, rx_len, &offset, &res)) {
        if (res.wkc >= 1U && res.data_len >= 4U) {
            *out_val = load32_le(res.data);
            return SYN_OK;
        }
    }

    return SYN_ERROR;
}

size_t syn_ecat_encode_write_sm(SYN_EcatMaster *m, uint16_t station_addr, uint8_t sm_index,
                                const SYN_EcatSMConfig *cfg)
{
    if (m == NULL || m->tx_buf == NULL || cfg == NULL || sm_index >= 4) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    uint32_t addr = ((uint32_t)station_addr << 16) | (SYN_ESC_REG_SM0 + (uint32_t)sm_index * 8U);
    uint8_t payload[8] = {0};
    store16_le(&payload[0], cfg->start_addr);
    store16_le(&payload[2], cfg->length);
    payload[4] = cfg->control;
    payload[5] = 0;
    payload[6] = cfg->enable;
    payload[7] = 0;

    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, sm_index, addr,
                                       payload, 8, false);
}

size_t syn_ecat_encode_write_fmmu(SYN_EcatMaster *m, uint16_t station_addr, uint8_t fmmu_index,
                                  const SYN_EcatFMMUConfig *cfg)
{
    if (m == NULL || m->tx_buf == NULL || cfg == NULL || fmmu_index >= 4) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    uint32_t addr =
        ((uint32_t)station_addr << 16) | (SYN_ESC_REG_FMMU0 + (uint32_t)fmmu_index * 16U);
    uint8_t payload[16] = {0};
    store32_le(&payload[0], cfg->logical_start);
    store16_le(&payload[4], cfg->length);
    payload[6] = cfg->logical_start_bit;
    payload[7] = cfg->logical_end_bit;
    store16_le(&payload[8], cfg->phys_start_addr);
    payload[10] = cfg->phys_start_bit;
    payload[11] = cfg->type;
    payload[12] = cfg->enable ? 1U : 0U;

    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, fmmu_index,
                                       addr, payload, 16, false);
}

size_t syn_ecat_encode_read_al_status(SYN_EcatMaster *m, uint16_t station_addr)
{
    if (m == NULL || m->tx_buf == NULL) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    uint32_t addr = ((uint32_t)station_addr << 16) | SYN_ESC_REG_AL_STATUS;
    uint8_t dummy[4] = {0};
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPRD, 0x05, addr,
                                       dummy, 4, false);
}

SYN_Status syn_ecat_decode_read_al_status(SYN_EcatMaster *m, size_t rx_len, SYN_EcatState *state,
                                          uint16_t *status_code)
{
    if (m == NULL || m->rx_buf == NULL || state == NULL || status_code == NULL || rx_len < 14U) {
        return SYN_INVALID_PARAM;
    }

    size_t offset = 2U;
    SYN_EcatDatagramResult res;
    if (syn_ecat_frame_parse_next(m->rx_buf, rx_len, &offset, &res)) {
        if (res.wkc >= 1U && res.data_len >= 4U) {
            *state = (SYN_EcatState)(res.data[0] & 0x0FU);
            *status_code = load16_le(&res.data[2]);
            return SYN_OK;
        }
    }

    return SYN_ERROR;
}

/* ── Phase 3: Mailbox & CoE SDO API Implementation ─────────────────────── */

size_t syn_ecat_encode_coe_sdo_write(SYN_EcatMaster *m, uint16_t station_addr, uint16_t index,
                                     uint8_t subindex, const void *data, size_t data_len)
{
    if (m == NULL || m->tx_buf == NULL || data == NULL || data_len == 0U || data_len > 4U) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);

    /* Pad full SM0 mailbox buffer so ESC triggers MBX_FULL on last byte write */
    uint8_t payload[SYN_ECAT_MBOX_SIZE] = {0};

    /* Mailbox Header */
    store16_le(&payload[0], 10U);          /* Mailbox payload length (CoE 2B + SDO 8B) */
    store16_le(&payload[2], station_addr); /* Target station address */
    payload[4] = 0x00U;                    /* Channel 0, Priority 0 */
    payload[5] = 0x03U;                    /* Mailbox Type 3 = CoE */

    /* CoE Header */
    uint16_t coe_hdr = (uint16_t)((SYN_ECAT_COE_TYPE_SDO_REQ & 0x0FU) << 12);
    store16_le(&payload[6], coe_hdr);

    /* SDO Download Command Header (expedited, e=1, s=1) */
    uint8_t sdo_hdr = 0x22U | (uint8_t)(((4U - data_len) & 0x03U) << 2) | 0x01U;
    payload[8] = sdo_hdr;
    store16_le(&payload[9], index);
    payload[11] = subindex;
    memcpy(&payload[12], data, data_len);

    uint32_t addr = ((uint32_t)station_addr << 16) | 0x1000U; /* Default Mailbox Out DPRAM */
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, 0x06U, addr,
                                       payload, SYN_ECAT_MBOX_SIZE, false);
}

size_t syn_ecat_encode_coe_sdo_read(SYN_EcatMaster *m, uint16_t station_addr, uint16_t index,
                                    uint8_t subindex)
{
    if (m == NULL || m->tx_buf == NULL) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);

    uint8_t payload[SYN_ECAT_MBOX_SIZE] = {0};

    /* Mailbox Header */
    store16_le(&payload[0], 10U);
    store16_le(&payload[2], station_addr);
    payload[4] = 0x00U;
    payload[5] = 0x03U; /* CoE */

    /* CoE Header */
    uint16_t coe_hdr = (uint16_t)((SYN_ECAT_COE_TYPE_SDO_REQ & 0x0FU) << 12);
    store16_le(&payload[6], coe_hdr);

    /* SDO Upload Command Header (0x40) */
    payload[8] = 0x40U;
    store16_le(&payload[9], index);
    payload[11] = subindex;

    uint32_t addr = ((uint32_t)station_addr << 16) | 0x1000U;
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, 0x07U, addr,
                                       payload, SYN_ECAT_MBOX_SIZE, false);
}

/**
 * @brief Validate and extract CoE SDO expedited upload response payload from mailbox data.
 * @param mbox_data Received SM1 mailbox buffer bytes.
 * @param check_idx True to verify SDO index/subindex match when response index is non-zero.
 * @param exp_idx Expected SDO Object Dictionary index.
 * @param exp_sub Expected SDO subindex.
 * @param out_data Destination buffer for extracted SDO payload.
 * @param max_len Capacity of out_data in bytes.
 * @param out_len Output extracted payload length in bytes.
 * @return SYN_OK on valid expedited SDO upload response, SYN_ERROR otherwise.
 */
static SYN_Status ecat_parse_coe_sdo_payload(const uint8_t *mbox_data, bool check_idx,
                                             uint16_t exp_idx, uint8_t exp_sub, void *out_data,
                                             size_t max_len, size_t *out_len)
{
    uint8_t mbox_type = mbox_data[5] & 0x0FU;
    uint16_t coe_hdr = load16_le(&mbox_data[6]);
    uint8_t coe_type = (uint8_t)((coe_hdr >> 12U) & 0x0FU);
    if (mbox_type != 0x03U || coe_type != SYN_ECAT_COE_TYPE_SDO_RESP) {
        return SYN_ERROR;
    }

    uint8_t sdo_hdr = mbox_data[8];
    if ((sdo_hdr & 0xE0U) != 0x40U || (sdo_hdr & 0x02U) == 0U) {
        return SYN_ERROR;
    }

    uint16_t resp_idx = load16_le(&mbox_data[9]);
    if (check_idx && resp_idx != 0U && (resp_idx != exp_idx || mbox_data[11] != exp_sub)) {
        return SYN_ERROR;
    }

    size_t payload_len = ((sdo_hdr & 0x01U) != 0U) ? (size_t)(4U - ((sdo_hdr >> 2U) & 0x03U)) : 4U;
    if (payload_len > max_len) {
        payload_len = max_len;
    }
    memcpy(out_data, &mbox_data[12], payload_len);
    *out_len = payload_len;
    return SYN_OK;
}

/**
 * @brief Decode CoE SDO upload response datagram with optional index/subindex verification.
 * @param m Pointer to master handle.
 * @param rx_len Received frame length.
 * @param check_idx True to verify SDO index/subindex match when response index is non-zero.
 * @param exp_idx Expected SDO Object Dictionary index.
 * @param exp_sub Expected SDO subindex.
 * @param out_data Output destination buffer.
 * @param max_len Maximum output buffer capacity.
 * @param out_len Pointer to receive read byte count.
 * @return SYN_OK on success, error status otherwise.
 */
static SYN_Status ecat_decode_coe_sdo_common(SYN_EcatMaster *m, size_t rx_len, bool check_idx,
                                             uint16_t exp_idx, uint8_t exp_sub, void *out_data,
                                             size_t max_len, size_t *out_len)
{
    if (m == NULL || m->rx_buf == NULL || out_data == NULL || out_len == NULL || rx_len < 14U) {
        return SYN_INVALID_PARAM;
    }

    size_t offset = 2U;
    SYN_EcatDatagramResult res;
    if (syn_ecat_frame_parse_next(m->rx_buf, rx_len, &offset, &res)) {
        if (res.wkc >= 1U && res.data_len >= 16U) {
            return ecat_parse_coe_sdo_payload(res.data, check_idx, exp_idx, exp_sub, out_data,
                                              max_len, out_len);
        }
    }

    return SYN_ERROR;
}

SYN_Status syn_ecat_decode_coe_sdo_response(SYN_EcatMaster *m, size_t rx_len, void *out_data,
                                            size_t max_len, size_t *out_len)
{
    return ecat_decode_coe_sdo_common(m, rx_len, false, 0U, 0U, out_data, max_len, out_len);
}

/* ── Phase 4: Cyclic Process Data Exchange Implementation ───────────────── */

size_t syn_ecat_encode_cyclic(SYN_EcatMaster *m)
{
    if (m == NULL || m->tx_buf == NULL) {
        return 0;
    }

    uint16_t xfer_size = (m->output_size >= m->input_size) ? m->output_size : m->input_size;
    if (xfer_size == 0U) {
        return 0;
    }
    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    size_t flen = syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_LRW, 0x10U,
                                              m->logical_addr, NULL, xfer_size, false);
    if ((flen > 0U) && (m->output_size > 0U) && (m->output_image != NULL)) {
        memcpy(&m->tx_buf[12], m->output_image, m->output_size);
    }
    return flen;
}

SYN_Status syn_ecat_decode_cyclic(SYN_EcatMaster *m, size_t rx_len)
{
    if (m == NULL || m->rx_buf == NULL || rx_len < 14U) {
        return SYN_INVALID_PARAM;
    }

    size_t offset = 2U;
    SYN_EcatDatagramResult res;
    if (syn_ecat_frame_parse_next(m->rx_buf, rx_len, &offset, &res)) {
        m->wkc_last = res.wkc;
        if (res.wkc >= 1U && res.wkc >= m->wkc_expected &&
            (m->input_size == 0U || res.data_len >= m->input_size)) {
            if (m->input_image != NULL && m->input_size > 0U) {
                memcpy(m->input_image, res.data, m->input_size);
            }
            m->cycle_count++;
            return SYN_OK;
        }
        m->wkc_errors++;
        return SYN_ERROR;
    }

    return SYN_ERROR;
}

/* ── Phase 5: ESM State Transitions Implementation ──────────────────────── */

size_t syn_ecat_encode_set_al_control(SYN_EcatMaster *m, uint16_t station_addr, SYN_EcatState state)
{
    if (m == NULL || m->tx_buf == NULL) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    uint32_t addr = ((uint32_t)station_addr << 16) | SYN_ESC_REG_AL_CONTROL;
    uint8_t payload[2] = {(uint8_t)(state & 0x0F), 0x00};
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, 0x08, addr,
                                       payload, 2, false);
}

/* ── Phase 6: Distributed Clocks (DC) Implementation ───────────────────── */

size_t syn_ecat_encode_dc_read_system_time(SYN_EcatMaster *m)
{
    if (m == NULL || m->tx_buf == NULL) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    uint8_t dummy[8] = {0};
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_ARMW, 0x09,
                                       SYN_ESC_REG_DC_SYSTEM_TIME, dummy, 8, false);
}

size_t syn_ecat_encode_dc_configure(SYN_EcatMaster *m, uint16_t station_addr,
                                    uint32_t sync0_cycle_ns, int32_t sync0_shift_ns)
{
    if (m == NULL || m->tx_buf == NULL) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    uint32_t base = (uint32_t)station_addr << 16U;
    uint8_t cyc_buf[4];
    uint8_t shift_buf[8];
    uint8_t act_buf[2];
    uint64_t shift_u64 = (uint64_t)(int64_t)sync0_shift_ns;

    store32_le(cyc_buf, sync0_cycle_ns);
    store32_le(&shift_buf[0], (uint32_t)(shift_u64 & 0xFFFFFFFFULL));
    store32_le(&shift_buf[4], (uint32_t)((shift_u64 >> 32U) & 0xFFFFFFFFULL));
    store16_le(act_buf, (sync0_cycle_ns > 0U) ? 0x0003U : 0x0000U);

    if (syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, 0x0AU,
                                    base | SYN_ESC_REG_DC_SYNC0_CYCLE, cyc_buf, 4U, false) == 0U ||
        syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, 0x0BU,
                                    base | SYN_ESC_REG_DC_START_TIME, shift_buf, 8U, false) == 0U) {
        return 0;
    }
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, 0x0CU,
                                       base | SYN_ESC_REG_DC_ACT, act_buf, 2U, false);
}

/* ── EtherCAT Master Transport & Protothread Tasks ─────────────────────── */

SYN_Status syn_ecat_master_set_rx_frame(SYN_EcatMaster *m, const uint8_t *frame, size_t len)
{
    if (m == NULL || frame == NULL || len < 14 || len > m->rx_buf_size) {
        return SYN_INVALID_PARAM;
    }

    memcpy(m->rx_buf, frame, len);
    m->rx_frame_len = (uint16_t)len;
    m->frame_rx_ready = true;
    return SYN_OK;
}

bool syn_ecat_master_pop_tx_frame(SYN_EcatMaster *m, const uint8_t **out_frame, size_t *out_len)
{
    if (m == NULL || out_frame == NULL || out_len == NULL || !m->frame_tx_pending) {
        return false;
    }

    *out_frame = m->tx_buf;
    *out_len = (size_t)m->tx_frame_len;
    m->frame_tx_pending = false;
    return true;
}

SYN_PT_Status syn_ecat_master_scan_task(SYN_PT *pt, SYN_EcatMaster *m)
{
    if (pt == NULL || m == NULL) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    /* 1. Send scan bus BRD */
    m->tx_frame_len = (uint16_t)syn_ecat_encode_scan_bus(m);
    if (m->tx_frame_len == 0U) {
        PT_EXIT(pt);
    }
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);
    syn_ecat_decode_scan_bus(m, m->rx_frame_len);

    /* 2. Assign station addresses for each slave */
    for (m->current_slave_idx = 0;
         m->current_slave_idx < m->slave_count && m->current_slave_idx < SYN_ECAT_MAX_SLAVES;
         m->current_slave_idx++) {
        m->slaves[m->current_slave_idx].station_addr = (uint16_t)(0x1001 + m->current_slave_idx);
        m->tx_frame_len = (uint16_t)syn_ecat_encode_assign_addr(
            m, m->current_slave_idx, m->slaves[m->current_slave_idx].station_addr);
        if (m->tx_frame_len == 0U) {
            PT_EXIT(pt);
        }
        m->frame_tx_pending = true;
        m->frame_rx_ready = false;
        PT_WAIT_UNTIL(pt, m->frame_rx_ready);
        m->slaves[m->current_slave_idx].auto_inc_addr = m->current_slave_idx;
        m->slaves[m->current_slave_idx].state = SYN_ECAT_STATE_INIT;
    }

    m->master_state = SYN_ECAT_STATE_INIT;
    PT_END(pt);
}

/**
 * @brief Get configured station address for slave index.
 * @param m Pointer to EtherCAT Master instance.
 * @param idx Slave index in master slave table.
 * @return Station address.
 */
static uint16_t syn_ecat_get_slave_station_addr(const SYN_EcatMaster *m, uint8_t idx)
{
    /* LCOV_EXCL_START: Static helper defensive bounds guard */
    if (m == NULL || idx >= SYN_ECAT_MAX_SLAVES) {
        return 0x1001;
    }
    /* LCOV_EXCL_STOP */
    uint16_t addr = m->slaves[idx].station_addr;
    return (addr != 0) ? addr : (uint16_t)(0x1001 + idx);
}

SYN_PT_Status syn_ecat_master_transition_task(SYN_PT *pt, SYN_EcatMaster *m,
                                              SYN_EcatState target_state)
{
    if (pt == NULL || m == NULL) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    for (m->current_slave_idx = 0;
         m->current_slave_idx < m->slave_count && m->current_slave_idx < SYN_ECAT_MAX_SLAVES;
         m->current_slave_idx++) {
        uint16_t st_addr = syn_ecat_get_slave_station_addr(m, m->current_slave_idx);

        /* If moving to PREOP, write SyncManager 0 & 1 default mailbox settings */
        if (target_state == SYN_ECAT_STATE_PREOP) {
            static SYN_EcatSMConfig sm0 = {.start_addr = 0x1000U,
                                           .length = SYN_ECAT_MBOX_SIZE,
                                           .control = 0x26U,
                                           .enable = 0x01U};
            static SYN_EcatSMConfig sm1 = {.start_addr = 0x1080U,
                                           .length = SYN_ECAT_MBOX_SIZE,
                                           .control = 0x22U,
                                           .enable = 0x01U};

            m->tx_frame_len = (uint16_t)syn_ecat_encode_write_sm(m, st_addr, 0, &sm0);
            if (m->tx_frame_len == 0U) {
                PT_EXIT(pt);
            }
            m->frame_tx_pending = true;
            m->frame_rx_ready = false;
            PT_WAIT_UNTIL(pt, m->frame_rx_ready);

            st_addr = syn_ecat_get_slave_station_addr(m, m->current_slave_idx);
            m->tx_frame_len = (uint16_t)syn_ecat_encode_write_sm(m, st_addr, 1, &sm1);
            if (m->tx_frame_len == 0U) {
                PT_EXIT(pt);
            }
            m->frame_tx_pending = true;
            m->frame_rx_ready = false;
            PT_WAIT_UNTIL(pt, m->frame_rx_ready);
        }

        /* Request AL Control state change */
        st_addr = syn_ecat_get_slave_station_addr(m, m->current_slave_idx);
        m->tx_frame_len = (uint16_t)syn_ecat_encode_set_al_control(m, st_addr, target_state);
        if (m->tx_frame_len == 0U) {
            PT_EXIT(pt);
        }
        m->frame_tx_pending = true;
        m->frame_rx_ready = false;
        PT_WAIT_UNTIL(pt, m->frame_rx_ready);

        /* Read AL Status */
        st_addr = syn_ecat_get_slave_station_addr(m, m->current_slave_idx);
        m->tx_frame_len = (uint16_t)syn_ecat_encode_read_al_status(m, st_addr);
        if (m->tx_frame_len == 0U) {
            PT_EXIT(pt);
        }
        m->frame_tx_pending = true;
        m->frame_rx_ready = false;
        PT_WAIT_UNTIL(pt, m->frame_rx_ready);
        syn_ecat_decode_read_al_status(m, m->rx_frame_len, &m->slaves[m->current_slave_idx].state,
                                       &m->slaves[m->current_slave_idx].al_status_code);
    }

    m->master_state = target_state;
    PT_END(pt);
}

SYN_PT_Status syn_ecat_master_cyclic_task(SYN_PT *pt, SYN_EcatMaster *m)
{
    if (pt == NULL || m == NULL) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    m->tx_frame_len = (uint16_t)syn_ecat_encode_cyclic(m);
    if (m->tx_frame_len == 0U) {
        PT_EXIT(pt);
    }
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);

    syn_ecat_decode_cyclic(m, m->rx_frame_len);

    PT_END(pt);
}

size_t syn_ecat_encode_read_reg(SYN_EcatMaster *m, uint16_t station_addr, uint16_t reg_addr,
                                uint16_t len)
{
    if (m == NULL || m->tx_buf == NULL || len == 0U) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    uint32_t addr = ((uint32_t)station_addr << 16) | (uint32_t)reg_addr;
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPRD, 0x01U, addr,
                                       NULL, len, false);
}

size_t syn_ecat_encode_write_reg(SYN_EcatMaster *m, uint16_t station_addr, uint16_t reg_addr,
                                 const void *data, uint16_t len)
{
    if (m == NULL || m->tx_buf == NULL || data == NULL || len == 0) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    uint32_t addr = ((uint32_t)station_addr << 16) | (uint32_t)reg_addr;
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, 0x01, addr,
                                       (const uint8_t *)data, len, false);
}

SYN_Status syn_ecat_decode_read_reg(SYN_EcatMaster *m, size_t rx_len, void *out_data, uint16_t len)
{
    if (m == NULL || m->rx_buf == NULL || out_data == NULL || rx_len < 14) {
        return SYN_INVALID_PARAM;
    }

    size_t offset = 2;
    SYN_EcatDatagramResult res;
    if (syn_ecat_frame_parse_next(m->rx_buf, rx_len, &offset, &res)) {
        if (res.wkc >= 1 && res.data_len >= len) {
            memcpy(out_data, res.data, len);
            return SYN_OK;
        }
    }
    return SYN_ERROR;
}

/**
 * @brief Check whether the last SII EEPROM status poll response indicates idle.
 * @param m Pointer to EtherCAT master instance.
 * @return true when SII busy bit is cleared.
 */
static bool ecat_sii_poll_done(SYN_EcatMaster *m)
{
    uint8_t status_buf[2] = {0};
    if (syn_ecat_decode_read_reg(m, m->rx_frame_len, status_buf, 2U) != SYN_OK) {
        return false;
    }
    return (load16_le(status_buf) & 0x8000U) == 0U;
}

/**
 * @brief Check whether the last SM1 status poll response indicates a queued mailbox frame.
 * @param m Pointer to EtherCAT master instance.
 * @return true when SM1 mailbox-full flag (bit 3) is set.
 */
static bool ecat_sm1_poll_ready(SYN_EcatMaster *m)
{
    uint8_t sm1_status = 0U;
    if (syn_ecat_decode_read_reg(m, m->rx_frame_len, &sm1_status, 1U) != SYN_OK) {
        return false;
    }
    return (sm1_status & 0x08U) != 0U;
}

SYN_PT_Status syn_ecat_master_read_sii_task(SYN_PT *pt, SYN_EcatMaster *m, uint16_t station_addr,
                                            uint16_t word_offset, uint32_t *out_data)
{
    if (pt == NULL || m == NULL || out_data == NULL) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    m->sdo_ok = false;

    /* 1. Write word offset to SII Address reg 0x0504 */
    uint8_t addr_bytes[4];
    store32_le(addr_bytes, (uint32_t)word_offset);
    m->tx_frame_len = (uint16_t)syn_ecat_encode_write_reg(m, station_addr, 0x0504, addr_bytes, 4);
    if (m->tx_frame_len == 0U) {
        PT_EXIT(pt);
    }
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);

    /* 2. Write Read Command (0x0100) to SII Control reg 0x0502 */
    uint8_t cmd_bytes[2];
    store16_le(cmd_bytes, 0x0100U);
    m->tx_frame_len = (uint16_t)syn_ecat_encode_write_reg(m, station_addr, 0x0502, cmd_bytes, 2);
    if (m->tx_frame_len == 0U) {
        PT_EXIT(pt);
    }
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);

    /* 3. Poll Busy bit (bit 15 of 0x0502) with timeout counter */
    m->poll_retries = 1000U;
    do {
        m->tx_frame_len = (uint16_t)syn_ecat_encode_read_reg(m, station_addr, 0x0502, 2);
        if (m->tx_frame_len == 0U) {
            PT_EXIT(pt);
        }
        m->frame_tx_pending = true;
        m->frame_rx_ready = false;
        PT_WAIT_UNTIL(pt, m->frame_rx_ready);

        if (ecat_sii_poll_done(m)) {
            break; /* EEPROM operation complete */
        }
        m->poll_retries--;
        if (m->poll_retries == 0U) {
            PT_EXIT(pt);
        }
    } while (m->poll_retries > 0U);

    /* 4. Read 32-bit EEPROM data from reg 0x0508 */
    m->tx_frame_len = (uint16_t)syn_ecat_encode_read_reg(m, station_addr, 0x0508, 4);
    if (m->tx_frame_len == 0U) {
        PT_EXIT(pt);
    }
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);

    uint8_t data_buf[4] = {0};
    if (syn_ecat_decode_read_reg(m, m->rx_frame_len, data_buf, 4) == SYN_OK) {
        *out_data = load32_le(data_buf);
        m->sdo_ok = true;
    }
    if (!m->sdo_ok) {
        PT_EXIT(pt);
    }

    PT_END(pt);
}

SYN_PT_Status syn_ecat_master_sdo_read_task(SYN_PT *pt, SYN_EcatMaster *m, uint16_t station_addr,
                                            uint16_t index, uint8_t subindex, void *out_data,
                                            size_t max_len, size_t *out_len)
{
    if (pt == NULL || m == NULL || out_data == NULL || out_len == NULL) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    m->sdo_ok = false;

    /* 1. Send SDO Read request to SM0 Mailbox */
    m->tx_frame_len = (uint16_t)syn_ecat_encode_coe_sdo_read(m, station_addr, index, subindex);
    if (m->tx_frame_len == 0U) {
        PT_EXIT(pt);
    }
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);

    /* 2. Poll SM1 Status (reg 0x080D - bit 3: mailbox full) with timeout counter */
    m->poll_retries = 1000U;
    do {
        m->tx_frame_len = (uint16_t)syn_ecat_encode_read_reg(m, station_addr, 0x080D, 1);
        if (m->tx_frame_len == 0U) {
            PT_EXIT(pt);
        }
        m->frame_tx_pending = true;
        m->frame_rx_ready = false;
        PT_WAIT_UNTIL(pt, m->frame_rx_ready);

        if (ecat_sm1_poll_ready(m)) {
            break; /* Mailbox response ready */
        }
        m->poll_retries--;
        if (m->poll_retries == 0U) {
            PT_EXIT(pt);
        }
    } while (m->poll_retries > 0U);

    /* 3. Read full SM1 Mailbox buffer (address 0x1080) to trigger ESC mailbox empty */
    m->tx_frame_len =
        (uint16_t)syn_ecat_encode_read_reg(m, station_addr, 0x1080U, SYN_ECAT_MBOX_SIZE);
    if (m->tx_frame_len == 0U) {
        PT_EXIT(pt);
    }
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);

    if (ecat_decode_coe_sdo_common(m, m->rx_frame_len, true, index, subindex, out_data, max_len,
                                   out_len) == SYN_OK) {
        m->sdo_ok = true;
    }
    if (!m->sdo_ok) {
        PT_EXIT(pt);
    }

    PT_END(pt);
}

/**
 * @brief Discover total mapped PDO bit length for one SyncManager assignment object.
 * @param pt           Protothread instance for this direction scan.
 * @param m            EtherCAT master context.
 * @param station_addr Target slave configured station address.
 * @param assign_idx   PDO assignment CoE index (e.g. 0x1C12 or 0x1C13).
 * @param out_bits     Accumulated bit count output.
 * @return Protothread execution status.
 */
static SYN_PT_Status ecat_discover_pdo_dir_task(SYN_PT *pt, SYN_EcatMaster *m,
                                                uint16_t station_addr, uint16_t assign_idx,
                                                uint32_t *out_bits)
{
    PT_BEGIN(pt);

    m->pdo_assign_cnt = 0U;
    memset(m->pdo_sdo_buf, 0, sizeof(m->pdo_sdo_buf));
    PT_SPAWN(pt, &m->sub_pt,
             syn_ecat_master_sdo_read_task(&m->sub_pt, m, station_addr, assign_idx, 0U,
                                           m->pdo_sdo_buf, 1U, &m->pdo_read_len));
    if (!m->sdo_ok) {
        PT_EXIT(pt);
    }
    m->pdo_assign_cnt = m->pdo_sdo_buf[0];

    for (m->pdo_assign_idx = 1U; m->pdo_assign_idx <= m->pdo_assign_cnt; m->pdo_assign_idx++) {
        memset(m->pdo_sdo_buf, 0, sizeof(m->pdo_sdo_buf));
        PT_SPAWN(pt, &m->sub_pt,
                 syn_ecat_master_sdo_read_task(&m->sub_pt, m, station_addr, assign_idx,
                                               (uint8_t)m->pdo_assign_idx, m->pdo_sdo_buf, 2U,
                                               &m->pdo_read_len));
        if (!m->sdo_ok) {
            PT_EXIT(pt);
        }
        m->pdo_map_idx = load16_le(m->pdo_sdo_buf);
        if (m->pdo_map_idx != 0U) {
            memset(m->pdo_sdo_buf, 0, sizeof(m->pdo_sdo_buf));
            PT_SPAWN(pt, &m->sub_pt,
                     syn_ecat_master_sdo_read_task(&m->sub_pt, m, station_addr, m->pdo_map_idx, 0U,
                                                   m->pdo_sdo_buf, 1U, &m->pdo_read_len));
            if (!m->sdo_ok) {
                PT_EXIT(pt);
            }
            m->pdo_count = m->pdo_sdo_buf[0];
            for (m->pdo_entry_idx = 1U; m->pdo_entry_idx <= m->pdo_count; m->pdo_entry_idx++) {
                memset(m->pdo_sdo_buf, 0, sizeof(m->pdo_sdo_buf));
                PT_SPAWN(pt, &m->sub_pt,
                         syn_ecat_master_sdo_read_task(&m->sub_pt, m, station_addr, m->pdo_map_idx,
                                                       (uint8_t)m->pdo_entry_idx, m->pdo_sdo_buf,
                                                       4U, &m->pdo_read_len));
                if (!m->sdo_ok) {
                    PT_EXIT(pt);
                }
                *out_bits += (load32_le(m->pdo_sdo_buf) & 0xFFU);
            }
        }
    }

    PT_END(pt);
}

SYN_PT_Status syn_ecat_master_discover_pdo_mapping_task(SYN_PT *pt, SYN_EcatMaster *m,
                                                        uint16_t station_addr,
                                                        uint16_t *out_rx_bytes,
                                                        uint16_t *out_tx_bytes)
{
    if (pt == NULL || m == NULL || out_rx_bytes == NULL || out_tx_bytes == NULL) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);

    *out_rx_bytes = 0U;
    *out_tx_bytes = 0U;
    m->total_rx_bits = 0U;
    m->total_tx_bits = 0U;

    PT_SPAWN(pt, &m->dir_pt,
             ecat_discover_pdo_dir_task(&m->dir_pt, m, station_addr, 0x1C12U, &m->total_rx_bits));
    if (!m->sdo_ok) {
        PT_EXIT(pt);
    }

    PT_SPAWN(pt, &m->dir_pt,
             ecat_discover_pdo_dir_task(&m->dir_pt, m, station_addr, 0x1C13U, &m->total_tx_bits));
    if (!m->sdo_ok) {
        PT_EXIT(pt);
    }

    *out_rx_bytes = (uint16_t)((m->total_rx_bits + 7U) / 8U);
    *out_tx_bytes = (uint16_t)((m->total_tx_bits + 7U) / 8U);

    PT_END(pt);
}

#endif /* SYN_USE_ETHERCAT */
