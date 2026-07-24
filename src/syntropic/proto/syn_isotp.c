#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_ISOTP) || SYN_USE_ISOTP

/**
 * @file syn_isotp.c
 * @brief ISO 15765-2 (ISO-TP) Multi-Frame CAN Transport implementation.
 */

#include "syn_isotp.h"
#include "../util/syn_assert.h"

#include <string.h>

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
/* ── CAN FD length padding helper (ISO 11898-1 valid lengths) ───────────── */
static uint8_t syn_can_fd_pad_len(uint8_t len)
{
    if (len <= 8)  return len;
    if (len <= 12) return 12;
    if (len <= 16) return 16;
    if (len <= 20) return 20;
    if (len <= 24) return 24;
    if (len <= 32) return 32;
    if (len <= 48) return 48;
    return 64;
}
#endif

/* ── STmin conversion helper (microsecond resolution) ────────────────────── */
static uint32_t syn_isotp_decode_stmin_us(uint8_t st_min)
{
    if (st_min <= 0x7F) {
        return (uint32_t)st_min * 1000U; /* 0..127 ms -> us */
    }
    if (st_min >= 0xF1 && st_min <= 0xF9) {
        return (uint32_t)(st_min - 0xF0) * 100U; /* 100..900 us */
    }
    return 0;
}

void syn_isotp_init(SYN_ISOTP_Link *link, uint32_t rx_id, uint32_t tx_id,
                    uint8_t *rx_buf, size_t rx_buf_size,
                    uint8_t *tx_buf, size_t tx_buf_size)
{
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    syn_isotp_init_fd(link, rx_id, tx_id, rx_buf, rx_buf_size, tx_buf, tx_buf_size, false);
#else
    SYN_ASSERT(link != NULL);

    memset(link, 0, sizeof(*link));
    link->rx_id        = rx_id;
    link->tx_id        = tx_id;
    link->rx_buf       = rx_buf;
    link->rx_buf_size  = rx_buf_size;
    link->tx_buf       = tx_buf;
    link->tx_buf_size  = tx_buf_size;
    link->tx_state     = SYN_ISOTP_TX_IDLE;
    link->rx_state     = SYN_ISOTP_RX_IDLE;
#endif
}

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
void syn_isotp_init_fd(SYN_ISOTP_Link *link, uint32_t rx_id, uint32_t tx_id,
                       uint8_t *rx_buf, size_t rx_buf_size,
                       uint8_t *tx_buf, size_t tx_buf_size, bool is_fd)
{
    SYN_ASSERT(link != NULL);

    memset(link, 0, sizeof(*link));
    link->rx_id        = rx_id;
    link->tx_id        = tx_id;
    link->is_fd        = is_fd;
    link->rx_buf       = rx_buf;
    link->rx_buf_size  = rx_buf_size;
    link->tx_buf       = tx_buf;
    link->tx_buf_size  = tx_buf_size;
    link->tx_state     = SYN_ISOTP_TX_IDLE;
    link->rx_state     = SYN_ISOTP_RX_IDLE;
}
#endif

SYN_Status syn_isotp_send(SYN_ISOTP_Link *link, const uint8_t *payload, size_t payload_len)
{
    if (link == NULL || payload == NULL || payload_len < 1) {
        return SYN_INVALID_PARAM;
    }
    if (link->tx_state != SYN_ISOTP_TX_IDLE) {
        return SYN_BUSY;
    }
    if (payload_len > link->tx_buf_size) {
        return SYN_INVALID_PARAM;
    }

    memcpy(link->tx_buf, payload, payload_len);
    link->tx_len     = payload_len;
    link->tx_offset  = 0;
    link->tx_seq     = 1;
    link->tx_bs_count = 0;

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    size_t max_sf_len = link->is_fd ? 62 : 7;
#else
    size_t max_sf_len = 7;
#endif

    if (payload_len <= max_sf_len) {
        link->tx_state = SYN_ISOTP_TX_SEND_SF;
    } else {
        link->tx_state = SYN_ISOTP_TX_SEND_FF;
    }

    return SYN_OK;
}

bool syn_isotp_get_tx_frame(SYN_ISOTP_Link *link, SYN_CAN_Frame *frame)
{
    if (link == NULL || frame == NULL) return false;

    memset(frame, 0, sizeof(*frame));
    frame->id = link->tx_id;
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    frame->is_fd = link->is_fd;
#endif

    /* Flow Control response takes priority */
    if (link->rx_fc_pending) {
        link->rx_fc_pending = false;
        frame->data[0] = SYN_ISOTP_PCI_FC | link->rx_fc_status;
        frame->data[1] = 0; /* BS = 0 (unlimited) */
        frame->data[2] = 0; /* STmin = 0 ms */
        frame->dlc     = 8;
        return true;
    }

    switch (link->tx_state) {
    case SYN_ISOTP_TX_SEND_SF:
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
        if (link->tx_len <= 7) {
            frame->data[0] = SYN_ISOTP_PCI_SF | (uint8_t)(link->tx_len & 0x0F);
            memcpy(&frame->data[1], link->tx_buf, link->tx_len);
            frame->dlc = link->is_fd ? syn_can_fd_pad_len((uint8_t)(1 + link->tx_len)) : 8;
        } else {
            frame->data[0] = 0x00; /* SF Escape */
            frame->data[1] = (uint8_t)link->tx_len;
            memcpy(&frame->data[2], link->tx_buf, link->tx_len);
            frame->dlc = syn_can_fd_pad_len((uint8_t)(2 + link->tx_len));
        }
#else
        frame->data[0] = SYN_ISOTP_PCI_SF | (uint8_t)(link->tx_len & 0x0F);
        memcpy(&frame->data[1], link->tx_buf, link->tx_len);
        frame->dlc = 8;
#endif
        link->tx_state = SYN_ISOTP_TX_IDLE;
        return true;

    case SYN_ISOTP_TX_SEND_FF:
        if (link->tx_len <= 4095) {
            /* Standard 12-bit First Frame */
            frame->data[0] = SYN_ISOTP_PCI_FF | (uint8_t)((link->tx_len >> 8) & 0x0F);
            frame->data[1] = (uint8_t)(link->tx_len & 0xFF);
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
            size_t chunk = link->is_fd ? 62 : 6;
            frame->dlc = link->is_fd ? 64 : 8;
#else
            size_t chunk = 6;
            frame->dlc = 8;
#endif
            if (chunk > link->tx_len) chunk = link->tx_len;
            memcpy(&frame->data[2], link->tx_buf, chunk);
            link->tx_offset = chunk;
        } else {
            /* ISO 15765-2:2016 32-bit Extended First Frame */
            frame->data[0] = 0x10; /* FF Escape */
            frame->data[1] = 0x00;
            frame->data[2] = (uint8_t)((uint32_t)link->tx_len >> 24);
            frame->data[3] = (uint8_t)((uint32_t)link->tx_len >> 16);
            frame->data[4] = (uint8_t)((uint32_t)link->tx_len >> 8);
            frame->data[5] = (uint8_t)((uint32_t)link->tx_len & 0xFF);
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
            size_t chunk = link->is_fd ? 58 : 2;
            frame->dlc = link->is_fd ? 64 : 8;
#else
            size_t chunk = 2;
            frame->dlc = 8;
#endif
            if (chunk > link->tx_len) chunk = link->tx_len;
            memcpy(&frame->data[6], link->tx_buf, chunk);
            link->tx_offset = chunk;
        }
        link->tx_state = SYN_ISOTP_TX_WAIT_FC;
        return true;

    case SYN_ISOTP_TX_SEND_CF:
        if (link->tx_st_timer_us > 0) {
            return false; /* Enforce STmin separation time */
        }

        frame->data[0] = SYN_ISOTP_PCI_CF | (link->tx_seq & 0x0F);
        size_t rem = link->tx_len - link->tx_offset;
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
        size_t max_chunk = link->is_fd ? 63 : 7;
        size_t chunk = (rem > max_chunk) ? max_chunk : rem;
        memcpy(&frame->data[1], &link->tx_buf[link->tx_offset], chunk);
        frame->dlc = link->is_fd ? syn_can_fd_pad_len((uint8_t)(1 + chunk)) : 8;
#else
        size_t chunk = (rem > 7) ? 7 : rem;
        memcpy(&frame->data[1], &link->tx_buf[link->tx_offset], chunk);
        frame->dlc = 8;
#endif

        link->tx_offset += chunk;
        link->tx_seq = (uint8_t)((link->tx_seq + 1) & 0x0F);
        link->tx_bs_count++;
        link->tx_st_timer_us = syn_isotp_decode_stmin_us(link->tx_st_min);

        if (link->tx_offset >= link->tx_len) {
            link->tx_state = SYN_ISOTP_TX_IDLE;
        } else if (link->tx_bs > 0 && link->tx_bs_count >= link->tx_bs) {
            link->tx_bs_count = 0;
            link->tx_state    = SYN_ISOTP_TX_WAIT_FC;
        }
        return true;

    default:
        return false;
    }
}

void syn_isotp_process_rx_frame(SYN_ISOTP_Link *link, const SYN_CAN_Frame *frame)
{
    if (link == NULL || frame == NULL) return;
    if (frame->id != link->rx_id) return;

    uint8_t pci_type = frame->data[0] & 0xF0U;

    switch (pci_type) {
    case SYN_ISOTP_PCI_SF: {
        size_t sf_len;
        size_t data_offset;

        if ((frame->data[0] & 0x0FU) != 0) {
            /* Standard Single Frame */
            sf_len = frame->data[0] & 0x0FU;
            data_offset = 1;
        } else {
            /* CAN FD Extended Single Frame */
            sf_len = frame->data[1];
            data_offset = 2;
        }

        if (sf_len >= 1 && sf_len <= link->rx_buf_size && link->rx_buf != NULL) {
            memcpy(link->rx_buf, &frame->data[data_offset], sf_len);
            link->rx_len   = sf_len;
            link->rx_state = SYN_ISOTP_RX_COMPLETE;
        }
        break;
    }

    case SYN_ISOTP_PCI_FF: {
        size_t ff_len;
        size_t data_offset;

        uint16_t len_12 = ((uint16_t)(frame->data[0] & 0x0FU) << 8) | frame->data[1];
        if (len_12 != 0) {
            ff_len = len_12;
            data_offset = 2;
        } else {
            /* 32-bit Extended First Frame */
            ff_len = (size_t)(((uint32_t)frame->data[2] << 24) |
                              ((uint32_t)frame->data[3] << 16) |
                              ((uint32_t)frame->data[4] << 8)  |
                              (uint32_t)frame->data[5]);
            data_offset = 6;
        }

        if (ff_len > 7 && link->rx_buf != NULL) {
            if (ff_len > link->rx_buf_size) {
                link->rx_fc_pending = true;
                link->rx_fc_status  = SYN_ISOTP_FC_OVERFLOW;
                link->rx_state      = SYN_ISOTP_RX_IDLE;
            } else {
                size_t frame_len = (frame->dlc > 0) ? frame->dlc : 8;
                size_t payload_in_ff = (frame_len > data_offset) ? (frame_len - data_offset) : 0;
                if (payload_in_ff > ff_len) payload_in_ff = ff_len;

                memcpy(link->rx_buf, &frame->data[data_offset], payload_in_ff);
                link->rx_len        = payload_in_ff;
                link->rx_expected   = ff_len;
                link->rx_seq        = 1;
                link->rx_fc_pending = true;
                link->rx_fc_status  = SYN_ISOTP_FC_CTS;
                link->rx_state      = SYN_ISOTP_RX_WAIT_CF;
            }
        }
        break;
    }

    case SYN_ISOTP_PCI_CF: {
        if (link->rx_state == SYN_ISOTP_RX_WAIT_CF) {
            uint8_t seq = frame->data[0] & 0x0FU;
            if (seq == link->rx_seq) {
                size_t frame_len = (frame->dlc > 0) ? frame->dlc : 8;
                size_t rem = link->rx_expected - link->rx_len;
                size_t cf_payload = (frame_len > 1) ? (frame_len - 1) : 0;
                if (cf_payload > rem) cf_payload = rem;

                memcpy(&link->rx_buf[link->rx_len], &frame->data[1], cf_payload);

                link->rx_len += cf_payload;
                link->rx_seq = (uint8_t)((link->rx_seq + 1) & 0x0F);

                if (link->rx_len >= link->rx_expected) {
                    link->rx_state = SYN_ISOTP_RX_COMPLETE;
                }
            } else {
                /* Sequence mismatch error */
                link->rx_state = SYN_ISOTP_RX_IDLE;
            }
        }
        break;
    }

    case SYN_ISOTP_PCI_FC: {
        if (link->tx_state == SYN_ISOTP_TX_WAIT_FC) {
            uint8_t fc_status = frame->data[0] & 0x0FU;
            if (fc_status == SYN_ISOTP_FC_CTS) {
                link->tx_bs          = frame->data[1];
                link->tx_st_min      = frame->data[2];
                link->tx_st_timer_us = 0;
                link->tx_state       = SYN_ISOTP_TX_SEND_CF;
            } else if (fc_status == SYN_ISOTP_FC_OVERFLOW) {
                link->tx_state       = SYN_ISOTP_TX_IDLE;
            }
        }
        break;
    }

    default:
        break;
    }
}

ssize_t syn_isotp_receive(SYN_ISOTP_Link *link, uint8_t *out_buf, size_t max_len)
{
    if (link == NULL || out_buf == NULL) {
        return -1;
    }

    if (link->rx_state == SYN_ISOTP_RX_COMPLETE) {
        size_t copy_len = (link->rx_len > max_len) ? max_len : link->rx_len;
        memcpy(out_buf, link->rx_buf, copy_len);
        link->rx_state = SYN_ISOTP_RX_IDLE;
        if (copy_len > 32767U) copy_len = 32767U;
        return (ssize_t)copy_len;
    }

    return -1;
}

void syn_isotp_step(SYN_ISOTP_Link *link, uint32_t dt_ms)
{
    syn_isotp_step_us(link, dt_ms * 1000U);
}

void syn_isotp_step_us(SYN_ISOTP_Link *link, uint32_t dt_us)
{
    if (link == NULL) return;

    if (link->tx_st_timer_us > 0) {
        if (dt_us >= link->tx_st_timer_us) {
            link->tx_st_timer_us = 0;
        } else {
            link->tx_st_timer_us -= dt_us;
        }
    }
}

#endif /* SYN_USE_ISOTP */
