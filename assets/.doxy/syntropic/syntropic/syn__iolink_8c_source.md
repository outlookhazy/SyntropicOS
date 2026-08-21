

# File syn\_iolink.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_iolink.c**](syn__iolink_8c.md)

[Go to the documentation of this file](syn__iolink_8c.md)


```C++

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_IOLINK) || SYN_USE_IOLINK

#include "../port/syn_port_system.h"
#include "syn_iolink.h"

#include <string.h>

#define SYN_IOLINK_DEFAULT_CYCLE_MS 5U
#define SYN_IOLINK_CRC6_POLY 0x3DU
#define SYN_IOLINK_CRC6_SEED 0x1FU

/* ── Checksum & CRC Calculation ─────────────────────────────────────────── */

uint8_t syn_iolink_calc_checksum(const uint8_t *data, size_t len)
{
    if (data == NULL || len == 0U) {
        return 0U;
    }

    uint8_t crc = SYN_IOLINK_CRC6_SEED;
    for (size_t i = 0U; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0U; bit < 8U; bit++) {
            if ((crc & 0x80U) != 0U) {
                crc = (uint8_t)((crc << 1U) ^ SYN_IOLINK_CRC6_POLY);
            } else {
                crc = (uint8_t)(crc << 1U);
            }
        }
    }
    crc = (crc >> 2U) & 0x3FU; /* 6-bit CRC in bits 0..5 */

    /* Calculate parity bits: bit 7 = even parity, bit 6 = odd parity */
    uint8_t p0 = (crc ^ (crc >> 2U) ^ (crc >> 4U)) & 0x01U;
    uint8_t p1 = ~((crc >> 1U) ^ (crc >> 3U) ^ (crc >> 5U)) & 0x01U;

    return (uint8_t)(crc | (p1 << 6U) | (p0 << 7U));
}

/* ── Master Implementation ───────────────────────────────────────────────── */

SYN_Status syn_iolink_master_init(SYN_IOLink_Master *master, const SYN_IOLink_MasterConfig *cfg)
{
    if (master == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (cfg->transport == NULL || cfg->rx_buf == NULL || cfg->tx_buf == NULL ||
        cfg->rx_buf_size < 32U || cfg->tx_buf_size < 32U) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(master, 0, sizeof(*master));
    master->cfg = *cfg;
    if (master->cfg.cycle_time_ms == 0U) {
        master->cfg.cycle_time_ms = SYN_IOLINK_DEFAULT_CYCLE_MS;
    }

    master->state = SYN_IOLINK_PORT_INACTIVE;
    master->mseq_type = SYN_IOLINK_TYPE_0;
    return SYN_OK;
}

static SYN_Status master_read_direct_param(SYN_IOLink_Master *master, uint8_t addr,
                                           uint8_t *out_val)
{
    uint8_t tx[3];
    tx[0] = (uint8_t)(0x80U | (addr & 0x3FU)); /* Read direct parameter (Bit 7=1, Bit 6=0) */
    tx[1] = syn_iolink_calc_checksum(&tx[0], 1U);

    if (!syn_transport_send(master->cfg.transport, tx, 2U)) {
        return SYN_ERROR;
    }

    size_t rx_len = 0U;
    if (!syn_transport_recv(master->cfg.transport, master->cfg.rx_buf, master->cfg.rx_buf_size,
                            &rx_len) ||
        rx_len < 2U) {
        return SYN_ERROR;
    }

    uint8_t expected_cks = syn_iolink_calc_checksum(master->cfg.rx_buf, rx_len - 1U);
    if (master->cfg.rx_buf[rx_len - 1U] != expected_cks) {
        return SYN_ERROR;
    }

    if (out_val != NULL) {
        *out_val = master->cfg.rx_buf[0];
    }
    return SYN_OK;
}

SYN_Status syn_iolink_master_start(SYN_IOLink_Master *master)
{
    if (master == NULL || master->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    master->state = SYN_IOLINK_PORT_STARTUP;

    /* 1. Read Minimum Cycle Time */
    uint8_t min_cycle = 0U;
    if (master_read_direct_param(master, SYN_IOLINK_PAGE_MIN_CYCLE_TIME, &min_cycle) != SYN_OK) {
        master->state = SYN_IOLINK_PORT_FAULT;
        return SYN_ERROR;
    }
    master->dev_params.min_cycle_time = min_cycle;

    /* 2. Read Revision ID */
    uint8_t rev_id = 0U;
    if (master_read_direct_param(master, SYN_IOLINK_PAGE_REVISION_ID, &rev_id) != SYN_OK) {
        master->state = SYN_IOLINK_PORT_FAULT;
        return SYN_ERROR;
    }
    master->dev_params.revision_id = rev_id;

    /* 3. Read Vendor ID (2 bytes) */
    uint8_t v1 = 0U;
    uint8_t v2 = 0U;
    (void)master_read_direct_param(master, SYN_IOLINK_PAGE_VENDOR_ID_1, &v1);
    (void)master_read_direct_param(master, SYN_IOLINK_PAGE_VENDOR_ID_2, &v2);
    master->dev_params.vendor_id = (uint16_t)(((uint16_t)v1 << 8U) | v2);

    /* 4. Read Device ID (3 bytes) */
    uint8_t d1 = 0U;
    uint8_t d2 = 0U;
    uint8_t d3 = 0U;
    (void)master_read_direct_param(master, SYN_IOLINK_PAGE_DEVICE_ID_1, &d1);
    (void)master_read_direct_param(master, SYN_IOLINK_PAGE_DEVICE_ID_2, &d2);
    (void)master_read_direct_param(master, SYN_IOLINK_PAGE_DEVICE_ID_3, &d3);
    master->dev_params.device_id = ((uint32_t)d1 << 16U) | ((uint32_t)d2 << 8U) | (uint32_t)d3;

    /* 5. Read Process Data Lengths */
    uint8_t pd_in_len = 0U;
    uint8_t pd_out_len = 0U;
    (void)master_read_direct_param(master, SYN_IOLINK_PAGE_PD_IN_LEN, &pd_in_len);
    (void)master_read_direct_param(master, SYN_IOLINK_PAGE_PD_OUT_LEN, &pd_out_len);
    master->dev_params.pd_in_len =
        (pd_in_len > 0U && pd_in_len <= SYN_IOLINK_MAX_PD_LEN) ? pd_in_len : 1U;
    master->dev_params.pd_out_len = (pd_out_len <= SYN_IOLINK_MAX_PD_LEN) ? pd_out_len : 1U;

    master->state = SYN_IOLINK_PORT_OPERATE;
    return SYN_OK;
}

SYN_Status syn_iolink_master_exchange_pd(SYN_IOLink_Master *master, const uint8_t *pd_out,
                                         size_t out_len, uint8_t *pd_in, size_t *in_len)
{
    if (master == NULL || master->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t tx[SYN_IOLINK_MAX_PD_LEN + 4U];
    tx[0] = 0x00U; /* Process Data Mode (Bit 7=0, Bit 6=0) */
    size_t tx_pos = 1U;

    if (pd_out != NULL && out_len > 0U) {
        size_t copy_len = (out_len <= SYN_IOLINK_MAX_PD_LEN) ? out_len : SYN_IOLINK_MAX_PD_LEN;
        (void)memcpy(&tx[tx_pos], pd_out, copy_len);
        tx_pos += copy_len;
    }

    uint8_t ckt = syn_iolink_calc_checksum(tx, tx_pos);
    tx[tx_pos++] = ckt;

    if (!syn_transport_send(master->cfg.transport, tx, tx_pos)) {
        master->state = SYN_IOLINK_PORT_FAULT;
        return SYN_ERROR;
    }

    size_t rx_len = 0U;
    if (!syn_transport_recv(master->cfg.transport, master->cfg.rx_buf, master->cfg.rx_buf_size,
                            &rx_len) ||
        rx_len < 2U) {
        master->state = SYN_IOLINK_PORT_FAULT;
        return SYN_ERROR;
    }

    uint8_t expected_cks = syn_iolink_calc_checksum(master->cfg.rx_buf, rx_len - 1U);
    if (master->cfg.rx_buf[rx_len - 1U] != expected_cks) {
        master->state = SYN_IOLINK_PORT_FAULT;
        return SYN_ERROR;
    }

    size_t pld_len = rx_len - 1U;
    if (pd_in != NULL && in_len != NULL) {
        (void)memcpy(pd_in, master->cfg.rx_buf, pld_len);
        *in_len = pld_len;
    }

    (void)memcpy(master->current_pd_in, master->cfg.rx_buf, pld_len);
    master->current_pd_in_len = pld_len;

    return SYN_OK;
}

SYN_Status syn_iolink_master_read_isdu(SYN_IOLink_Master *master, uint16_t index, uint8_t subindex,
                                       uint8_t *data, size_t max_len, size_t *out_len)
{
    if (master == NULL || master->cfg.transport == NULL || data == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t tx[8];
    tx[0] = 0xC0U; /* ISDU Read (Bit 7=1, Bit 6=1) */
    tx[1] = (uint8_t)((index >> 8U) & 0xFFU);
    tx[2] = (uint8_t)(index & 0xFFU);
    tx[3] = subindex;
    tx[4] = syn_iolink_calc_checksum(tx, 4U);

    if (!syn_transport_send(master->cfg.transport, tx, 5U)) {
        return SYN_ERROR;
    }

    size_t rx_len = 0U;
    if (!syn_transport_recv(master->cfg.transport, master->cfg.rx_buf, master->cfg.rx_buf_size,
                            &rx_len) ||
        rx_len < 2U) {
        return SYN_ERROR;
    }

    uint8_t expected_cks = syn_iolink_calc_checksum(master->cfg.rx_buf, rx_len - 1U);
    if (master->cfg.rx_buf[rx_len - 1U] != expected_cks) {
        return SYN_ERROR;
    }

    size_t pld_len = rx_len - 1U;
    size_t copy_len = (pld_len < max_len) ? pld_len : max_len;
    (void)memcpy(data, master->cfg.rx_buf, copy_len);
    *out_len = copy_len;
    return SYN_OK;
}

SYN_Status syn_iolink_master_write_isdu(SYN_IOLink_Master *master, uint16_t index, uint8_t subindex,
                                        const uint8_t *data, size_t len)
{
    if (master == NULL || master->cfg.transport == NULL || (data == NULL && len > 0U)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t tx[SYN_IOLINK_MAX_ISDU_LEN + 8U];
    tx[0] = 0x40U; /* ISDU Write (Bit 7=0, Bit 6=1) */
    tx[1] = (uint8_t)((index >> 8U) & 0xFFU);
    tx[2] = (uint8_t)(index & 0xFFU);
    tx[3] = subindex;
    size_t tx_pos = 4U;

    if (len > 0U) {
        size_t copy_len = (len <= SYN_IOLINK_MAX_ISDU_LEN) ? len : SYN_IOLINK_MAX_ISDU_LEN;
        (void)memcpy(&tx[tx_pos], data, copy_len);
        tx_pos += copy_len;
    }

    tx[tx_pos] = syn_iolink_calc_checksum(tx, tx_pos);
    tx_pos++;

    if (!syn_transport_send(master->cfg.transport, tx, tx_pos)) {
        return SYN_ERROR;
    }

    size_t rx_len = 0U;
    if (!syn_transport_recv(master->cfg.transport, master->cfg.rx_buf, master->cfg.rx_buf_size,
                            &rx_len) ||
        rx_len < 2U) {
        return SYN_ERROR;
    }

    uint8_t expected_cks = syn_iolink_calc_checksum(master->cfg.rx_buf, rx_len - 1U);
    if (master->cfg.rx_buf[rx_len - 1U] != expected_cks) {
        return SYN_ERROR;
    }

    return SYN_OK;
}

SYN_Status syn_iolink_master_step(SYN_IOLink_Master *master, uint32_t now_ms)
{
    if (master == NULL || master->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    if (master->state == SYN_IOLINK_PORT_OPERATE &&
        (now_ms - master->last_cycle_ms >= master->cfg.cycle_time_ms ||
         master->last_cycle_ms == 0U)) {
        master->last_cycle_ms = now_ms;
        uint8_t dummy_in[SYN_IOLINK_MAX_PD_LEN];
        size_t in_len = 0U;
        (void)syn_iolink_master_exchange_pd(master, master->current_pd_out,
                                            master->current_pd_out_len, dummy_in, &in_len);
    }
    return SYN_OK;
}

SYN_PT_Status syn_iolink_master_pt(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_ENDED;
    }

    SYN_IOLink_Master *master = (SYN_IOLink_Master *)task->user_data;

    PT_BEGIN(pt);

    for (;;) {
        uint32_t now = syn_port_get_tick_ms();
        if (syn_iolink_master_step(master, now) != SYN_OK) {
            break;
        }
        PT_YIELD(pt);
    }

    PT_END(pt);
}

/* ── Device Implementation ───────────────────────────────────────────────── */

SYN_Status syn_iolink_device_init(SYN_IOLink_Device *dev, const SYN_IOLink_DeviceConfig *cfg)
{
    if (dev == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(dev, 0, sizeof(*dev));
    dev->cfg = *cfg;
    dev->state = SYN_IOLINK_PORT_OPERATE;
    return SYN_OK;
}

SYN_Status syn_iolink_device_set_pd_in(SYN_IOLink_Device *dev, const uint8_t *data, size_t len)
{
    if (dev == NULL || (data == NULL && len > 0U)) {
        return SYN_INVALID_PARAM;
    }
    size_t copy_len = (len <= SYN_IOLINK_MAX_PD_LEN) ? len : SYN_IOLINK_MAX_PD_LEN;
    if (copy_len > 0U) {
        (void)memcpy(dev->pd_in, data, copy_len);
    }
    return SYN_OK;
}

SYN_Status syn_iolink_device_get_pd_out(const SYN_IOLink_Device *dev, uint8_t *out_buf,
                                        size_t max_len, size_t *out_len)
{
    if (dev == NULL || out_buf == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM;
    }
    size_t copy_len =
        (dev->cfg.params.pd_out_len <= max_len) ? dev->cfg.params.pd_out_len : max_len;
    (void)memcpy(out_buf, dev->pd_out, copy_len);
    *out_len = copy_len;
    return SYN_OK;
}

SYN_Status syn_iolink_device_process_frame(SYN_IOLink_Device *dev, const uint8_t *rx, size_t rx_len,
                                           uint8_t *tx, size_t max_tx, size_t *out_len)
{
    if (dev == NULL || rx == NULL || tx == NULL || out_len == NULL || rx_len < 2U) {
        return SYN_INVALID_PARAM;
    }

    *out_len = 0U;
    uint8_t ckt = rx[rx_len - 1U];
    uint8_t expected_ckt = syn_iolink_calc_checksum(rx, rx_len - 1U);
    if (ckt != expected_ckt) {
        return SYN_ERROR;
    }

    uint8_t mc = rx[0];
    bool is_read = ((mc & 0x80U) != 0U);
    bool is_isdu = ((mc & 0x40U) != 0U);
    uint8_t addr = (mc & 0x3FU);

    /* 1. Direct Parameter Page 1 Access */
    if (!is_isdu && is_read) {
        uint8_t param_val = 0U;
        switch (addr) {
        case SYN_IOLINK_PAGE_MIN_CYCLE_TIME:
            param_val = dev->cfg.params.min_cycle_time;
            break;
        case SYN_IOLINK_PAGE_REVISION_ID:
            param_val = dev->cfg.params.revision_id;
            break;
        case SYN_IOLINK_PAGE_VENDOR_ID_1:
            param_val = (uint8_t)((dev->cfg.params.vendor_id >> 8U) & 0xFFU);
            break;
        case SYN_IOLINK_PAGE_VENDOR_ID_2:
            param_val = (uint8_t)(dev->cfg.params.vendor_id & 0xFFU);
            break;
        case SYN_IOLINK_PAGE_DEVICE_ID_1:
            param_val = (uint8_t)((dev->cfg.params.device_id >> 16U) & 0xFFU);
            break;
        case SYN_IOLINK_PAGE_DEVICE_ID_2:
            param_val = (uint8_t)((dev->cfg.params.device_id >> 8U) & 0xFFU);
            break;
        case SYN_IOLINK_PAGE_DEVICE_ID_3:
            param_val = (uint8_t)(dev->cfg.params.device_id & 0xFFU);
            break;
        case SYN_IOLINK_PAGE_PD_IN_LEN:
            param_val = dev->cfg.params.pd_in_len;
            break;
        case SYN_IOLINK_PAGE_PD_OUT_LEN:
            param_val = dev->cfg.params.pd_out_len;
            break;
        default:
            param_val = 0U;
            break;
        }

        tx[0] = param_val;
        tx[1] = syn_iolink_calc_checksum(tx, 1U);
        *out_len = 2U;
        return SYN_OK;
    }

    /* 2. Process Data Exchange */
    if (!is_isdu && !is_read) {
        size_t pd_out_len = rx_len - 2U;
        if (pd_out_len > 0U && pd_out_len <= SYN_IOLINK_MAX_PD_LEN) {
            (void)memcpy(dev->pd_out, &rx[1], pd_out_len);
        }

        size_t pd_in_len =
            (dev->cfg.params.pd_in_len > 0U && dev->cfg.params.pd_in_len <= SYN_IOLINK_MAX_PD_LEN)
                ? dev->cfg.params.pd_in_len
                : 1U;
        if (pd_in_len + 1U > max_tx) {
            return SYN_ERROR;
        }

        (void)memcpy(tx, dev->pd_in, pd_in_len);
        tx[pd_in_len] = syn_iolink_calc_checksum(tx, pd_in_len);
        *out_len = pd_in_len + 1U;
        return SYN_OK;
    }

    /* 3. ISDU Transactions */
    if (is_isdu && rx_len >= 5U) {
        uint16_t index = (uint16_t)(((uint16_t)rx[1] << 8U) | rx[2]);
        uint8_t subindex = rx[3];

        if (is_read && dev->cfg.on_read != NULL) {
            size_t pld_len = 0U;
            SYN_Status st =
                dev->cfg.on_read(index, subindex, tx, max_tx - 1U, &pld_len, dev->cfg.user_data);
            if (st == SYN_OK) {
                tx[pld_len] = syn_iolink_calc_checksum(tx, pld_len);
                *out_len = pld_len + 1U;
                return SYN_OK;
            }
        } else if (!is_read && dev->cfg.on_write != NULL) {
            size_t pld_len = rx_len - 5U;
            SYN_Status st = dev->cfg.on_write(index, subindex, &rx[4], pld_len, dev->cfg.user_data);
            if (st == SYN_OK) {
                tx[0] = 0x00U; /* ACK */
                tx[1] = syn_iolink_calc_checksum(tx, 1U);
                *out_len = 2U;
                return SYN_OK;
            }
        }
    }

    return SYN_ERROR;
}

#endif /* SYN_USE_IOLINK */
```


