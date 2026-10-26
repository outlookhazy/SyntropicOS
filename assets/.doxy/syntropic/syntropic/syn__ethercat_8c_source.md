

# File syn\_ethercat.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ethercat.c**](syn__ethercat_8c.md)

[Go to the documentation of this file](syn__ethercat_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_ETHERCAT) || SYN_USE_ETHERCAT

#include "../util/syn_assert.h"
#include "syn_ethercat.h"

#include <string.h>

/* ── Inline Helpers ─────────────────────────────────────────────────────── */

static inline void store16_le(uint8_t *p, uint16_t val)
{
    p[0] = (uint8_t)(val & 0xFF);
    p[1] = (uint8_t)((val >> 8) & 0xFF);
}

static inline void store32_le(uint8_t *p, uint32_t val)
{
    p[0] = (uint8_t)(val & 0xFF);
    p[1] = (uint8_t)((val >> 8) & 0xFF);
    p[2] = (uint8_t)((val >> 16) & 0xFF);
    p[3] = (uint8_t)((val >> 24) & 0xFF);
}

static inline uint16_t load16_le(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

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
    if (buf == NULL || buf_size < 14) {
        return 0;
    }

    uint16_t cur_hdr = load16_le(buf);
    size_t cur_dg_bytes = (size_t)(cur_hdr & 0x07FF);
    size_t offset = 2 + cur_dg_bytes;
    size_t dg_total = 12 + (size_t)data_len;

    if (offset + dg_total > buf_size) {
        return 0;
    }

    /* If there is a preceding datagram, set its 'm' (more datagrams) bit */
    if (cur_dg_bytes > 0) {
        size_t pos = 2;
        while (pos < offset) {
            uint16_t d_len = load16_le(&buf[pos + 6]) & 0x07FF;
            size_t d_size = 12 + (size_t)d_len;
            if (pos + d_size == offset) {
                uint16_t prev_flags = load16_le(&buf[pos + 6]);
                prev_flags |= (1U << 15);
                store16_le(&buf[pos + 6], prev_flags);
                break;
            }
            pos += d_size;
        }
    }

    buf[offset + 0] = (uint8_t)cmd;
    buf[offset + 1] = idx;
    store32_le(&buf[offset + 2], addr);

    uint16_t len_flags = (uint16_t)(data_len & 0x07FF);
    if (circ) {
        len_flags |= (1U << 14);
    }
    store16_le(&buf[offset + 6], len_flags);
    store16_le(&buf[offset + 8], 0); /* irq */

    if (data_len > 0 && data != NULL) {
        memcpy(&buf[offset + 10], data, data_len);
    }

    store16_le(&buf[offset + 10 + data_len], 0); /* WKC = 0 on transmit */

    size_t new_dg_bytes = cur_dg_bytes + dg_total;
    uint16_t new_hdr = (uint16_t)(new_dg_bytes & 0x07FF);
    new_hdr |= (uint16_t)((SYN_ETHERCAT_TYPE_DATAGRAM & 0x0F) << 12);
    store16_le(buf, new_hdr);

    return 2 + new_dg_bytes;
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
    size_t total_len = 2 + 10 + data_len + 2;
    if (buf_len < total_len)
        return 0;

    /* 1. Pack 2-byte EtherCAT Header */
    uint16_t ecat_hdr = (uint16_t)((10 + data_len + 2) & 0x07FF);
    ecat_hdr |= (uint16_t)((SYN_ETHERCAT_TYPE_DATAGRAM & 0x0F) << 12);
    store16_le(buf, ecat_hdr);

    /* 2. Pack 10-byte Datagram Header */
    buf[2] = dg->cmd;
    buf[3] = dg->idx;
    store32_le(&buf[4], dg->addr);

    uint16_t len_flags = (uint16_t)(data_len & 0x07FF);
    if (dg->m)
        len_flags |= (1U << 15);
    if (dg->circ)
        len_flags |= (1U << 14);
    store16_le(&buf[8], len_flags);

    store16_le(&buf[10], dg->irq);

    /* 3. Copy Data */
    if (data_len > 0 && data != NULL) {
        memcpy(&buf[12], data, data_len);
    }

    /* 4. Pack Working Counter (WKC = 0 on transmit) */
    store16_le(&buf[12 + data_len], 0);

    return total_len;
}

SYN_Status syn_ecat_parse_frame(SYN_EcatNode *node, const uint8_t *frame, size_t len, uint16_t *wkc)
{
    SYN_ASSERT(node != NULL && frame != NULL);

    if (len < 14) /* 2 byte header + 10 byte dg header + 2 byte WKC */
        return SYN_ERROR;

    uint16_t ecat_hdr = load16_le(frame);
    uint16_t frame_type = (ecat_hdr >> 12) & 0x0F;
    if (frame_type != SYN_ETHERCAT_TYPE_DATAGRAM)
        return SYN_ERROR;

    uint16_t dg_len = ecat_hdr & 0x07FF;
    if (dg_len < 12 || len < (size_t)(2 + dg_len))
        return SYN_ERROR;

    /* Extract Datagram WKC from the end of the first datagram */
    size_t wkc_offset = 2 + dg_len - 2;
    uint16_t received_wkc = load16_le(&frame[wkc_offset]);
    if (wkc != NULL) {
        *wkc = received_wkc;
    }

    node->wkc_last = received_wkc;

    /* Check if WKC matches expected */
    if (received_wkc < node->wkc_expected) {
        return SYN_ERROR;
    }

    return SYN_OK;
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

    if (data_len > 4 || buf_len < 10) /* 2 byte Mailbox + 8 byte SDO */
        return 0;

    /* CoE Mailbox Header: Type 3 (SDO Request) */
    uint16_t coe_hdr = (uint16_t)((SYN_ECAT_COE_TYPE_SDO_REQ & 0x0F) << 12);
    store16_le(buf, coe_hdr);

    /* SDO Download Command Header: Expedited SDO Download Request (0x22 / 0x23 / etc) */
    uint8_t sdo_hdr = 0x22 | (uint8_t)(((4 - data_len) & 0x03) << 2) | 0x02; /* e=1, s=1 */
    buf[2] = sdo_hdr;
    store16_le(&buf[3], index);
    buf[5] = subindex;

    /* SDO Data */
    memset(&buf[6], 0, 4);
    if (data != NULL && data_len > 0) {
        memcpy(&buf[6], data, data_len);
    }

    return 10;
}

size_t syn_ecat_coe_encode_sdo_upload(uint8_t *buf, size_t buf_len, uint16_t index,
                                      uint8_t subindex)
{
    SYN_ASSERT(buf != NULL);

    if (buf_len < 10)
        return 0;

    /* CoE Mailbox Header: Type 3 (SDO Request) */
    uint16_t coe_hdr = (uint16_t)((SYN_ECAT_COE_TYPE_SDO_REQ & 0x0F) << 12);
    store16_le(buf, coe_hdr);

    /* SDO Upload Request Command Header: 0x40 */
    buf[2] = 0x40;
    store16_le(&buf[3], index);
    buf[5] = subindex;
    memset(&buf[6], 0, 4);

    return 10;
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
    if (m == NULL || m->rx_buf == NULL || out_val == NULL || rx_len < 14) {
        return SYN_INVALID_PARAM;
    }

    size_t offset = 2;
    SYN_EcatDatagramResult res;
    if (syn_ecat_frame_parse_next(m->rx_buf, rx_len, &offset, &res)) {
        if (res.data_len >= 4) {
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
    if (m == NULL || m->rx_buf == NULL || state == NULL || status_code == NULL || rx_len < 14) {
        return SYN_INVALID_PARAM;
    }

    size_t offset = 2;
    SYN_EcatDatagramResult res;
    if (syn_ecat_frame_parse_next(m->rx_buf, rx_len, &offset, &res)) {
        if (res.data_len >= 4) {
            *state = (SYN_EcatState)(res.data[0] & 0x0F);
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
    if (m == NULL || m->tx_buf == NULL || data_len > 4) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);

    /* 6B Mailbox Header + 2B CoE Header + 8B SDO = 16B payload */
    uint8_t payload[16] = {0};

    /* Mailbox Header */
    store16_le(&payload[0], 10);           /* Mailbox payload length (CoE 2B + SDO 8B) */
    store16_le(&payload[2], station_addr); /* Target station address */
    payload[4] = 0x00;                     /* Channel 0, Priority 0 */
    payload[5] = 0x03;                     /* Mailbox Type 3 = CoE */

    /* CoE Header */
    uint16_t coe_hdr = (uint16_t)((SYN_ECAT_COE_TYPE_SDO_REQ & 0x0F) << 12);
    store16_le(&payload[6], coe_hdr);

    /* SDO Download Command Header (expedited) */
    uint8_t sdo_hdr = 0x22 | (uint8_t)(((4 - data_len) & 0x03) << 2) | 0x02;
    payload[8] = sdo_hdr;
    store16_le(&payload[9], index);
    payload[11] = subindex;

    if (data != NULL && data_len > 0) {
        memcpy(&payload[12], data, data_len);
    }

    uint32_t addr = ((uint32_t)station_addr << 16) | 0x1000U; /* Default Mailbox Out DPRAM */
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, 0x06, addr,
                                       payload, 16, false);
}

size_t syn_ecat_encode_coe_sdo_read(SYN_EcatMaster *m, uint16_t station_addr, uint16_t index,
                                    uint8_t subindex)
{
    if (m == NULL || m->tx_buf == NULL) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);

    uint8_t payload[16] = {0};

    /* Mailbox Header */
    store16_le(&payload[0], 10);
    store16_le(&payload[2], station_addr);
    payload[4] = 0x00;
    payload[5] = 0x03; /* CoE */

    /* CoE Header */
    uint16_t coe_hdr = (uint16_t)((SYN_ECAT_COE_TYPE_SDO_REQ & 0x0F) << 12);
    store16_le(&payload[6], coe_hdr);

    /* SDO Upload Command Header (0x40) */
    payload[8] = 0x40;
    store16_le(&payload[9], index);
    payload[11] = subindex;

    uint32_t addr = ((uint32_t)station_addr << 16) | 0x1000U;
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, 0x07, addr,
                                       payload, 16, false);
}

SYN_Status syn_ecat_decode_coe_sdo_response(SYN_EcatMaster *m, size_t rx_len, void *out_data,
                                            size_t max_len, size_t *out_len)
{
    if (m == NULL || m->rx_buf == NULL || out_data == NULL || out_len == NULL || rx_len < 14) {
        return SYN_INVALID_PARAM;
    }

    size_t offset = 2;
    SYN_EcatDatagramResult res;
    if (syn_ecat_frame_parse_next(m->rx_buf, rx_len, &offset, &res)) {
        if (res.data_len >= 16) {
            /* Check Mailbox Type (byte 5 & 0x0F == 3) */
            uint8_t mbox_type = res.data[5] & 0x0F;
            if (mbox_type != 3) {
                return SYN_ERROR;
            }

            /* SDO Header at offset 8 */
            uint8_t sdo_hdr = res.data[8];
            /* Expedited upload response header matches 0x40 (0x4F, 0x4B, 0x47, 0x43) */
            if ((sdo_hdr & 0xE0) == 0x40) {
                uint8_t n = (sdo_hdr >> 2) & 0x03;
                size_t payload_len = 4 - n;
                if (payload_len > max_len) {
                    payload_len = max_len;
                }
                memcpy(out_data, &res.data[12], payload_len);
                *out_len = payload_len;
                return SYN_OK;
            }
        }
    }

    return SYN_ERROR;
}

/* ── Phase 4: Cyclic Process Data Exchange Implementation ───────────────── */

size_t syn_ecat_encode_cyclic(SYN_EcatMaster *m)
{
    if (m == NULL || m->tx_buf == NULL || m->output_size == 0) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_LRW, 0x10,
                                       m->logical_addr, m->output_image, m->output_size, false);
}

SYN_Status syn_ecat_decode_cyclic(SYN_EcatMaster *m, size_t rx_len)
{
    if (m == NULL || m->rx_buf == NULL || rx_len < 14) {
        return SYN_INVALID_PARAM;
    }

    size_t offset = 2;
    SYN_EcatDatagramResult res;
    if (syn_ecat_frame_parse_next(m->rx_buf, rx_len, &offset, &res)) {
        m->wkc_last = res.wkc;
        if (res.wkc >= m->wkc_expected) {
            if (m->input_image != NULL && m->input_size > 0 && res.data_len >= m->input_size) {
                memcpy(m->input_image, res.data, m->input_size);
            }
            m->cycle_count++;
            return SYN_OK;
        } else {
            m->wkc_errors++;
            return SYN_ERROR;
        }
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
    uint32_t addr = ((uint32_t)station_addr << 16) | SYN_ESC_REG_DC_SYNC0_CYCLE;
    uint8_t payload[8];
    store32_le(&payload[0], sync0_cycle_ns);
    store32_le(&payload[4], (uint32_t)sync0_shift_ns);

    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPWR, 0x0A, addr,
                                       payload, 8, false);
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
        return PT_ENDED;
    }

    PT_BEGIN(pt);

    /* 1. Send scan bus BRD */
    m->tx_frame_len = (uint16_t)syn_ecat_encode_scan_bus(m);
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
        m->frame_tx_pending = true;
        m->frame_rx_ready = false;
        PT_WAIT_UNTIL(pt, m->frame_rx_ready);
        m->slaves[m->current_slave_idx].auto_inc_addr = m->current_slave_idx;
        m->slaves[m->current_slave_idx].state = SYN_ECAT_STATE_INIT;
    }

    m->master_state = SYN_ECAT_STATE_INIT;
    PT_END(pt);
}

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
        return PT_ENDED;
    }

    PT_BEGIN(pt);

    for (m->current_slave_idx = 0;
         m->current_slave_idx < m->slave_count && m->current_slave_idx < SYN_ECAT_MAX_SLAVES;
         m->current_slave_idx++) {
        uint16_t st_addr = syn_ecat_get_slave_station_addr(m, m->current_slave_idx);

        /* If moving to PREOP, write SyncManager 0 & 1 default mailbox settings */
        if (target_state == SYN_ECAT_STATE_PREOP) {
            static SYN_EcatSMConfig sm0 = {
                .start_addr = 0x1000, .length = 128, .control = 0x26, .enable = 0x01};
            static SYN_EcatSMConfig sm1 = {
                .start_addr = 0x1080, .length = 128, .control = 0x22, .enable = 0x01};

            m->tx_frame_len = (uint16_t)syn_ecat_encode_write_sm(m, st_addr, 0, &sm0);
            m->frame_tx_pending = true;
            m->frame_rx_ready = false;
            PT_WAIT_UNTIL(pt, m->frame_rx_ready);

            st_addr = syn_ecat_get_slave_station_addr(m, m->current_slave_idx);
            m->tx_frame_len = (uint16_t)syn_ecat_encode_write_sm(m, st_addr, 1, &sm1);
            m->frame_tx_pending = true;
            m->frame_rx_ready = false;
            PT_WAIT_UNTIL(pt, m->frame_rx_ready);
        }

        /* Request AL Control state change */
        st_addr = syn_ecat_get_slave_station_addr(m, m->current_slave_idx);
        m->tx_frame_len = (uint16_t)syn_ecat_encode_set_al_control(m, st_addr, target_state);
        m->frame_tx_pending = true;
        m->frame_rx_ready = false;
        PT_WAIT_UNTIL(pt, m->frame_rx_ready);

        /* Read AL Status */
        st_addr = syn_ecat_get_slave_station_addr(m, m->current_slave_idx);
        m->tx_frame_len = (uint16_t)syn_ecat_encode_read_al_status(m, st_addr);
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
        return PT_ENDED;
    }

    PT_BEGIN(pt);

    m->tx_frame_len = (uint16_t)syn_ecat_encode_cyclic(m);
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);

    syn_ecat_decode_cyclic(m, m->rx_frame_len);

    PT_END(pt);
}

size_t syn_ecat_encode_read_reg(SYN_EcatMaster *m, uint16_t station_addr, uint16_t reg_addr,
                                uint16_t len)
{
    if (m == NULL || m->tx_buf == NULL || len == 0) {
        return 0;
    }

    syn_ecat_frame_begin(m->tx_buf, m->tx_buf_size);
    uint32_t addr = ((uint32_t)station_addr << 16) | (uint32_t)reg_addr;
    uint8_t dummy[16] = {0};
    uint16_t datagram_len = (len > sizeof(dummy)) ? (uint16_t)sizeof(dummy) : len;
    return syn_ecat_frame_add_datagram(m->tx_buf, m->tx_buf_size, SYN_ECAT_CMD_FPRD, 0x01, addr,
                                       dummy, datagram_len, false);
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

SYN_PT_Status syn_ecat_master_read_sii_task(SYN_PT *pt, SYN_EcatMaster *m, uint16_t station_addr,
                                            uint16_t word_offset, uint32_t *out_data)
{
    if (pt == NULL || m == NULL || out_data == NULL) {
        return PT_ENDED;
    }

    PT_BEGIN(pt);

    /* 1. Write word offset to SII Address reg 0x0504 */
    uint8_t addr_bytes[4];
    store32_le(addr_bytes, (uint32_t)word_offset);
    m->tx_frame_len = (uint16_t)syn_ecat_encode_write_reg(m, station_addr, 0x0504, addr_bytes, 4);
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);

    /* 2. Write Read Command (0x0100) to SII Control reg 0x0502 */
    uint8_t cmd_bytes[2];
    store16_le(cmd_bytes, 0x0100U);
    m->tx_frame_len = (uint16_t)syn_ecat_encode_write_reg(m, station_addr, 0x0502, cmd_bytes, 2);
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);

    /* 3. Poll Busy bit (bit 15 of 0x0502) with timeout counter */
    m->poll_retries = 1000U;
    do {
        m->tx_frame_len = (uint16_t)syn_ecat_encode_read_reg(m, station_addr, 0x0502, 2);
        m->frame_tx_pending = true;
        m->frame_rx_ready = false;
        PT_WAIT_UNTIL(pt, m->frame_rx_ready);

        uint8_t status_buf[2] = {0};
        if (syn_ecat_decode_read_reg(m, m->rx_frame_len, status_buf, 2) == SYN_OK) {
            uint16_t ctrl = load16_le(status_buf);
            if ((ctrl & 0x8000U) == 0) {
                break; /* EEPROM operation complete */
            }
        }
        if (m->poll_retries > 0) {
            m->poll_retries--;
        }
        if (m->poll_retries == 0) {
            PT_EXIT(pt);
        }
    } while (m->poll_retries > 0);

    /* 4. Read 32-bit EEPROM data from reg 0x0508 */
    m->tx_frame_len = (uint16_t)syn_ecat_encode_read_reg(m, station_addr, 0x0508, 4);
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);

    uint8_t data_buf[4] = {0};
    if (syn_ecat_decode_read_reg(m, m->rx_frame_len, data_buf, 4) == SYN_OK) {
        *out_data = load32_le(data_buf);
    }

    PT_END(pt);
}

SYN_PT_Status syn_ecat_master_sdo_read_task(SYN_PT *pt, SYN_EcatMaster *m, uint16_t station_addr,
                                            uint16_t index, uint8_t subindex, void *out_data,
                                            size_t max_len, size_t *out_len)
{
    if (pt == NULL || m == NULL || out_data == NULL || out_len == NULL) {
        return PT_ENDED;
    }

    PT_BEGIN(pt);

    /* 1. Send SDO Read request to SM0 Mailbox */
    m->tx_frame_len = (uint16_t)syn_ecat_encode_coe_sdo_read(m, station_addr, index, subindex);
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);

    /* 2. Poll SM1 Status (reg 0x080D - bit 3: mailbox full) with timeout counter */
    m->poll_retries = 1000U;
    do {
        m->tx_frame_len = (uint16_t)syn_ecat_encode_read_reg(m, station_addr, 0x080D, 1);
        m->frame_tx_pending = true;
        m->frame_rx_ready = false;
        PT_WAIT_UNTIL(pt, m->frame_rx_ready);

        uint8_t sm1_status = 0;
        if (syn_ecat_decode_read_reg(m, m->rx_frame_len, &sm1_status, 1) == SYN_OK) {
            if ((sm1_status & 0x08U) != 0) {
                break; /* Mailbox response ready */
            }
        }
        if (m->poll_retries > 0) {
            m->poll_retries--;
        }
        if (m->poll_retries == 0) {
            PT_EXIT(pt);
        }
    } while (m->poll_retries > 0);

    /* 3. Read SDO response from SM1 Mailbox buffer (address 0x1080) */
    m->tx_frame_len = (uint16_t)syn_ecat_encode_read_reg(m, station_addr, 0x1080, 16);
    m->frame_tx_pending = true;
    m->frame_rx_ready = false;
    PT_WAIT_UNTIL(pt, m->frame_rx_ready);

    uint32_t val = 0;
    if (syn_ecat_decode_coe_sdo_response(m, m->rx_frame_len, &val, max_len, out_len) == SYN_OK) {
        if (max_len >= 4) {
            store32_le((uint8_t *)out_data, val);
        } else if (max_len >= 2) {
            store16_le((uint8_t *)out_data, (uint16_t)val);
        } else if (max_len >= 1) {
            ((uint8_t *)out_data)[0] = (uint8_t)val;
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
        return PT_ENDED;
    }

    PT_BEGIN(pt);

    m->total_rx_bits = 0;
    m->total_tx_bits = 0;

    /* Read 0x1C12:00 (RxPDO Assign Count) */
    m->pdo_count = 0;
    m->pdo_sdo_val = 0;
    PT_SPAWN(pt, &m->sub_pt,
             syn_ecat_master_sdo_read_task(&m->sub_pt, m, station_addr, 0x1C12, 0, &m->pdo_sdo_val,
                                           1, &m->pdo_read_len));
    m->pdo_count = (uint8_t)m->pdo_sdo_val;

    if (m->pdo_count > 0) {
        /* Read first RxPDO assignment (0x1C12:01) */
        m->pdo_sdo_val = 0;
        PT_SPAWN(pt, &m->sub_pt,
                 syn_ecat_master_sdo_read_task(&m->sub_pt, m, station_addr, 0x1C12, 1,
                                               &m->pdo_sdo_val, 2, &m->pdo_read_len));
        m->pdo_map_idx = (uint16_t)m->pdo_sdo_val;
        if (m->pdo_map_idx != 0) {
            /* Read RxPDO mapping entry count (e.g. 0x1600:00) */
            m->pdo_sdo_val = 0;
            PT_SPAWN(pt, &m->sub_pt,
                     syn_ecat_master_sdo_read_task(&m->sub_pt, m, station_addr, m->pdo_map_idx, 0,
                                                   &m->pdo_sdo_val, 1, &m->pdo_read_len));
            m->pdo_count = (uint8_t)m->pdo_sdo_val;
            for (m->pdo_entry_idx = 1; m->pdo_entry_idx <= m->pdo_count; m->pdo_entry_idx++) {
                m->pdo_sdo_val = 0;
                PT_SPAWN(pt, &m->sub_pt,
                         syn_ecat_master_sdo_read_task(&m->sub_pt, m, station_addr, m->pdo_map_idx,
                                                       m->pdo_entry_idx, &m->pdo_sdo_val, 4,
                                                       &m->pdo_read_len));
                m->total_rx_bits += (m->pdo_sdo_val & 0xFFU);
            }
        }
    }

    /* Read 0x1C13:00 (TxPDO Assign Count) */
    m->pdo_count = 0;
    m->pdo_sdo_val = 0;
    PT_SPAWN(pt, &m->sub_pt,
             syn_ecat_master_sdo_read_task(&m->sub_pt, m, station_addr, 0x1C13, 0, &m->pdo_sdo_val,
                                           1, &m->pdo_read_len));
    m->pdo_count = (uint8_t)m->pdo_sdo_val;

    if (m->pdo_count > 0) {
        /* Read first TxPDO assignment (0x1C13:01) */
        m->pdo_sdo_val = 0;
        PT_SPAWN(pt, &m->sub_pt,
                 syn_ecat_master_sdo_read_task(&m->sub_pt, m, station_addr, 0x1C13, 1,
                                               &m->pdo_sdo_val, 2, &m->pdo_read_len));
        m->pdo_map_idx = (uint16_t)m->pdo_sdo_val;
        if (m->pdo_map_idx != 0) {
            /* Read TxPDO mapping entry count (e.g. 0x1A00:00) */
            m->pdo_sdo_val = 0;
            PT_SPAWN(pt, &m->sub_pt,
                     syn_ecat_master_sdo_read_task(&m->sub_pt, m, station_addr, m->pdo_map_idx, 0,
                                                   &m->pdo_sdo_val, 1, &m->pdo_read_len));
            m->pdo_count = (uint8_t)m->pdo_sdo_val;
            for (m->pdo_entry_idx = 1; m->pdo_entry_idx <= m->pdo_count; m->pdo_entry_idx++) {
                m->pdo_sdo_val = 0;
                PT_SPAWN(pt, &m->sub_pt,
                         syn_ecat_master_sdo_read_task(&m->sub_pt, m, station_addr, m->pdo_map_idx,
                                                       m->pdo_entry_idx, &m->pdo_sdo_val, 4,
                                                       &m->pdo_read_len));
                m->total_tx_bits += (m->pdo_sdo_val & 0xFFU);
            }
        }
    }

    *out_rx_bytes = (uint16_t)((m->total_rx_bits + 7U) / 8U);
    *out_tx_bytes = (uint16_t)((m->total_tx_bits + 7U) / 8U);

    PT_END(pt);
}

#endif /* SYN_USE_ETHERCAT */
```


