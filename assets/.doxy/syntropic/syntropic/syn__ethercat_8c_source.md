

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

/* ── Public API ─────────────────────────────────────────────────────────── */

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

#endif /* SYN_USE_ETHERCAT */
```


